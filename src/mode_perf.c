/**
 * @file      mode_perf.c
 * @brief     Performance mode implementation.
 * @author    Shane Barnes
 * @date      28 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "dlist.h"
#include "form_perf.h"
#include "logger.h"
#include "mode_perf.h"
#include "output_if_std.h"
#include "sock_mod.h"
#include "thread_obj.h"
#include "token_bucket.h"
#include "util_cpu.h"
#include "util_date.h"
#include "util_mem.h"
#include "util_string.h"

#include <string.h>
#include <unistd.h>

static struct threadobj thread;
static struct args_obj *opts = NULL;

/**
 * @brief Set a socket configuration.
 *
 * @param[in,out] obj       A pointer to a socket object.
 * @param[in]     timeoutms Socket timeout in milliseconds.
 *
 * @return Void.
 */
static void modeperf_conf(struct sockobj * const obj, const int32_t timeoutms)
{
    memcpy(obj->conf.ipaddr, opts->ipaddr, sizeof(obj->conf.ipaddr));
    obj->conf.ipport        = opts->ipport;
    obj->conf.backlog       = opts->backlog;
    obj->conf.timeoutms     = timeoutms;
    obj->conf.datalimitbyte = opts->datalimitbyte;
    obj->conf.ratelimitbps  = opts->ratelimitbps;
    obj->conf.timelimitusec = opts->timelimitusec;
    obj->conf.family        = opts->family;
    obj->conf.type          = opts->type;
    obj->conf.model         = opts->arch;
}

/**
 * @brief Call a socket's receive or send function.
 *
 * @param[in]     call   A pointer to a socket object's receive or send
 *                       function.
 * @param[in,out] stats  Function-specific socket statistics.
 * @param[in,out] obj    A pointer to a socket object.
 * @param[in]     buf    A pointer to a buffer.
 * @param[in]     buflen The maximum size of a buffer in bytes.
 * @param[in]     tsus   The current Unix time in microseconds.
 *
 * @return The number of bytes returned by a socket's receive or send function.
 */
static int32_t modeperf_call(int32_t (*call)(struct sockobj * const obj,
                                          void * const buf,
                                          const uint32_t len),
                             struct sockobj_flowstats *stats,
                             struct sockobj * const sock,
                             void * const buf,
                             const uint32_t buflen,
                             const uint64_t tsus)
{
    int32_t ret = 0;
    uint64_t len = 0;

    if (opts->datalimitbyte > 0)
    {
        if (stats->totalbytes < opts->datalimitbyte)
        {
            len = opts->datalimitbyte - stats->totalbytes;

            if (len > buflen)
            {
                len = buflen;
            }

            len = tokenbucket_remove(&sock->tb, len * 8) / 8;
        }
    }
    else
    {
        len = tokenbucket_remove(&sock->tb, buflen * 8) / 8;
    }

    if (len > 0)
    {
        ret = call(sock, buf, len);
    }
    // @todo poll socket if buflen is zero?

    if (ret < 0)
    {
        sock->ops.sock_close(sock);
    }
    else if ((opts->timelimitusec > 0) &&
             ((tsus - sock->info.startusec) >= opts->timelimitusec))
    {
        sock->ops.sock_close(sock);
    }
    else if ((opts->datalimitbyte > 0) &&
             (stats->totalbytes >= opts->datalimitbyte))
    {
        sock->ops.sock_close(sock);
    }

    tokenbucket_return(&sock->tb, ret < 0 ? 0 : len - (uint32_t)ret);


    return ret;
}

/**
 * @see See header file for interface comments.
 */
static void *modeperf_thread(void * arg)
{
    bool exit = true;
    struct threadobj *thread = (struct threadobj *)arg;
    struct dlist list;
    struct sockobj group, server, *sock = NULL;
    struct formobj form;
    char *recvbuf = NULL, *sendbuf = NULL;
    int32_t formbytes = 0, recvbytes = 0, sendbytes = 0;
    uint32_t count = 0;

    //??
    struct dlist_node *next = NULL, *node = NULL;
    struct sockobj_flowstats *stats = NULL;
    struct utilcpu_info info;
    uint64_t activetimeus = 0;
#if defined(__APPLE__)
    uint64_t inactivetimeus = 1000;
#else
    uint64_t inactivetimeus = 0;
#endif
    uint64_t delayus = 0, mindelayus = 0;
    uint64_t tsus = 0;
    uint32_t burstlimit = opts->backlog <= 0 ? SOMAXCONN : opts->backlog;
    uint32_t burst = 0;
    memset(&group, 0, sizeof(group));

    if (thread == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if ((recvbuf = UTILMEM_MALLOC(char, opts->buflen)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: receive buffer allocation failed\n",
                      __FUNCTION__);
    }
    else if ((sendbuf = UTILMEM_MALLOC(char, opts->buflen)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: send buffer allocation failed\n",
                      __FUNCTION__);

        UTILMEM_FREE(recvbuf);
        recvbuf = NULL;
    }
    else
    {
        exit = false;
        memset(&list, 0, sizeof(list));

        form.ops.form_head = formperf_head;
        form.ops.form_body = formperf_body;
        form.ops.form_foot = formperf_foot;
        form.srcbuf = recvbuf;
        form.srclen = opts->buflen;
        form.dstbuf = sendbuf;
        form.dstlen = opts->buflen;

        if (opts->arch == SOCKOBJ_MODEL_SERVER)
        {
            modeperf_conf(&server, 500);
            exit = !sockmod_init(&server);
        }

        while ((exit == false) && (threadobj_isrunning(thread) == true))
        {
            burst = count;

            while ((exit == false) && ((count - burst) < burstlimit))
            {
                if ((count < opts->maxcon) ||
                    (opts->arch == SOCKOBJ_MODEL_SERVER))
                {
                    // @todo Do not allocate memory every iteration.
                    sock = UTILMEM_MALLOC(struct sockobj, 1);

                    if (sock == NULL)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: failed to allocate memory\n",
                                      __FUNCTION__);
                    }
                    else if (dlist_inserttail(&list, sock) == false)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: failed to store allocated memory\n",
                                      __FUNCTION__);

                        UTILMEM_FREE(sock);
                        sock = NULL;
                    }
                    else if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                    {
                        modeperf_conf(sock, 0);

                        if (sockmod_init(sock) == false)
                        {
                            UTILMEM_FREE(list.tail->val);
                            dlist_removetail(&list);
                            sock = NULL;
                            exit = true; ///only for multiple connections??
                            break;
                        }
                        else
                        {
                            sock->id = ++count;
                            form.sock = sock;

                            if (count == 1)
                            {
                                group.info.startusec = sock->info.startusec;
                                formbytes = form.ops.form_head(&form);
                                output_if_std_send(form.dstbuf, formbytes);
                            }
                        }
                    }
                    else
                    {
                        server.event.timeoutms = (list.size > 1 ? 0 : 500);

                        if (server.ops.sock_accept(&server, sock) == true)
                        {
                            if ((opts->maxcon == 0) ||
                                (list.size <= opts->maxcon))
                            {
                                sock->id = ++count;
                                logger_printf(LOGGER_LEVEL_INFO,
                                              "%s: server accepted connection on %s\n",
                                              __FUNCTION__,
                                              server.addrself.sockaddrstr);
                                sock->event.timeoutms = 0;
                                form.sock = sock;
                                if (list.size == 1)
                                {
                                    group.info.startusec = sock->info.startusec;
                                    formbytes = form.ops.form_head(&form);
                                    output_if_std_send(form.dstbuf, formbytes);
                                }
                            }
                            else
                            {
                                // Refuse connection.
                                sock->ops.sock_close(sock);
                                UTILMEM_FREE(list.tail->val);
                                dlist_removetail(&list);
                                sock = NULL;
                            }
                        }
                        else
                        {
                            UTILMEM_FREE(list.tail->val);
                            dlist_removetail(&list);
                            sock = NULL;

                            if (server.event.revents & FIONOBJ_REVENT_ERROR)
                            {
                                // Wait for any open sockets to complete.
                                if (list.size == 0)
                                {
                                    exit = true;
                                }
                            }
                            else if (list.size == 0)
                            {
                                form.sock = &server;
                                formbytes = formobj_idle(&form);
                                output_if_std_send(form.dstbuf, formbytes);
                                formbytes = utilstring_concat(form.dstbuf,
                                                              form.dstlen,
                                                              "%c",
                                                              '\r');
                                output_if_std_send(form.dstbuf, formbytes);
                            }

                            break;
                        }
                    }
                }
                else if ((opts->maxcon == 0) && (opts->arch == SOCKOBJ_MODEL_CLIENT))
                {
                    exit = true;
                }
                else
                {
                    break;
                }
            }

            mindelayus = 0;
            node = list.head;

            while (node != NULL)
            {
                sock = node->val;
                form.sock = sock;

                // @todo Perform once per iteration?
                tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
                form.tsus = tsus;

                if (activetimeus == 0)
                {
                    activetimeus = tsus;
                }

                if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                {
                    stats = &sock->info.send;

                    if ((sock->state & SOCKOBJ_STATE_CONNECT) == 0)
                    {
                        sock->ops.sock_connect(sock);
                        sendbytes = 0;
                    }
                    else
                    {
                        sendbytes = modeperf_call(sock->ops.sock_send,
                                                  &sock->info.send,
                                                  sock,
                                                  sendbuf,
                                                  opts->buflen,
                                                  tsus);
                    }

                    if (sendbytes > 0)
                    {
                        group.info.send.totalbytes += sendbytes;
                    }

                    if ((sock->state & SOCKOBJ_STATE_CLOSE) == 0)
                    {
                        formbytes = form.ops.form_body(&form);
                        // @todo protect against -1 being cast to uint32_t
                        output_if_std_send(form.dstbuf, formbytes);

                        // Prevent thread spin when no bytes are available.
                        if (sendbytes == 0)
                        {
                            if (sock->tb.rate > 0)
                            {
                                delayus = tokenbucket_delay(&sock->tb,
                                                            opts->buflen * 8);

                                if ((delayus < mindelayus) || (mindelayus == 0))
                                {
                                    mindelayus = delayus;
                                }
                            }
                            else if (tsus - activetimeus > inactivetimeus)
                            {
                                // @todo This needs to occur on a group of fds.
                                sock->event.timeoutms = 1;
                                sock->event.pevents = FIONOBJ_PEVENT_OUT;
                            }
                        }
                        else
                        {
                            sock->event.timeoutms = 0;
                            sock->event.pevents = FIONOBJ_PEVENT_IN;
                            activetimeus = tsus;
                        }
                    }
                }
                else
                {
                    stats = &sock->info.recv;

                    recvbytes = modeperf_call(sock->ops.sock_recv,
                                              &sock->info.recv,
                                              sock,
                                              recvbuf,
                                              opts->buflen,
                                              tsus);

                    if (recvbytes > 0)
                    {
                        group.info.recv.totalbytes += recvbytes;
                    }

                    if ((sock->state & SOCKOBJ_STATE_CLOSE) == 0)
                    {
                        formbytes = form.ops.form_body(&form);
                        output_if_std_send(form.dstbuf, formbytes);

                        // Prevent thread spin when no bytes are available.
                        if (recvbytes == 0)
                        {
                            if (sock->tb.rate > 0)
                            {
                                delayus = tokenbucket_delay(&sock->tb,
                                                            opts->buflen * 8);

                                if ((delayus < mindelayus) || (mindelayus == 0))
                                {
                                    mindelayus = delayus;
                                }
                            }
                            else if (tsus - activetimeus > inactivetimeus)
                            {
                                // @todo This needs to occur on a group of fds.
                                sock->event.timeoutms = 1;
                            }
                        }
                        else
                        {
                            sock->event.timeoutms = 0;
                            activetimeus = tsus;
                        }
                    }
                }

                if (sock->state & SOCKOBJ_STATE_CLOSE)
                {
                    if (list.size == 1)
                    {
                        group.info.stopusec = sock->info.stopusec;
                    }

                    formbytes = form.ops.form_foot(&form);
                    output_if_std_send(form.dstbuf, formbytes);

                    utilcpu_getinfo(&info);
                    logger_printf(LOGGER_LEVEL_INFO,
                                  "%s: cpu load: %d usr/sys usec: %" PRIu64
                                  " / %" PRIu64 "\n",
                                  __FUNCTION__,
                                  info.load,
                                  info.utimeus,
                                  info.stimeus);
                    logger_printf(LOGGER_LEVEL_INFO,
                                  "%s: calls pass/fail: %" PRIu64
                                  " / %" PRIu64
                                  "  time us pass/fail: %" PRIu64
                                  " / %" PRIu64 "\n",
                                  __FUNCTION__,
                                  stats->passedcalls,
                                  stats->failedcalls,
                                  stats->passedtsus,
                                  stats->failedtsus);
                    logger_printf(LOGGER_LEVEL_INFO,
                                  "%s: buflen avg/min/max: %" PRIi64
                                  " / %" PRIi64
                                  " / %" PRIi64 "\n",
                                  __FUNCTION__,
                                  stats->buflen.avg,
                                  stats->buflen.min,
                                  stats->buflen.max);

                    next = node->next;
                    UTILMEM_FREE(node->val);
                    dlist_remove(&list, node);
                    node = next;

                    if (list.size == 0)
                    {
                        group.id = count;
                        form.sock = &group;
                        formbytes = form.ops.form_foot(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        memset(&group, 0, sizeof(group));

                        if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                        {
                            exit = true;
                        }
                    }
                }
                else
                {
                    node = node->next;
                }
            }

            if (mindelayus > 0)
            {
                // @todo do not sleep any longer than the reporting interval;
                // @todo Replace with semaphore.
                usleep((uint32_t)mindelayus);
            }
        }

        UTILMEM_FREE(recvbuf);
        UTILMEM_FREE(sendbuf);
    }

    return NULL;
}

/**
 * @see See header file for interface comments.
 */
static bool modeperf_init(struct args_obj * const args)
{
    bool ret = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        opts = args;
        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
static bool modeperf_start(void)
{
    bool ret = false;

    thread.function = modeperf_thread;
    thread.argument = &thread;

    if (opts == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (threadobj_create(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to create chat thread\n",
                      __FUNCTION__);
    }
    else if (threadobj_start(&thread) == false)
    {
        threadobj_destroy(&thread);
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to start chat thread\n",
                      __FUNCTION__);
    }
    else
    {
        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
static bool modeperf_stop(void)
{
    bool ret = false;

    if (threadobj_stop(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to stop chat thread\n",
                      __FUNCTION__);
    }
    else if (threadobj_destroy(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to destroy chat thread\n",
                      __FUNCTION__);
    }
    else
    {
        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool modeperf_run(struct args_obj * const args)
{
    bool ret = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (modeperf_init(args) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to initialize mode\n",
                          __FUNCTION__);

        }
        else if (modeperf_start() == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to start mode\n",
                          __FUNCTION__);

        }
        else if (threadobj_join(&thread) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to suspend caller\n",
                          __FUNCTION__);
        }
        else
        {
            modeperf_stop();
            ret = true;
        }
    }

    return ret;
}
