/**
 * @file      mode_perf.c
 * @brief     Performance mode implementation.
 * @author    Shane Barnes
 * @date      28 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "cv_obj.h"
#include "dlist.h"
#include "fion_poll.h"
#include "form_perf.h"
#include "logger.h"
#include "mode_perf.h"
#include "mutex_obj.h"
#include "output_if_std.h"
#include "sock_mod.h"
#include "thread_obj.h"
#include "thread_pool.h"
#include "token_bucket.h"
#include "util_cpu.h"
#include "util_date.h"
#include "util_debug.h"
#include "util_mem.h"
#include "util_string.h"

#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct modeobj_priv
{
    struct args_obj    args;
    struct threadpool  threadpool;
    struct mutexobj    mtx;
    struct cvobj       cv;
    struct dlist      *sockq;
    uint32_t           sock_count;
    uint32_t           total_socks;
};

bool modeperf_create(struct modeobj * const mode,
                     const struct args_obj * const args)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) &&
                         (mode->priv == NULL) &&
                         (args != NULL)))
    {
        if ((mode->priv = UTILMEM_CALLOC(struct modeobj_priv,
                                         sizeof(struct modeobj_priv),
                                         1)) == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate memory\n",
                          __FUNCTION__);
        }
        else if (memcpy(&mode->priv->args,
                        args,
                        sizeof(mode->priv->args)) == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to copy arguments\n",
                          __FUNCTION__);
        }
        else if ((mode->priv->sockq = UTILMEM_CALLOC(struct dlist,
                                                     sizeof(struct dlist),
                                                     args->threads)) == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create socket queue\n",
                          __FUNCTION__);
            UTILMEM_FREE(mode->priv);
            mode->priv = NULL;
        }
        else if (!mutexobj_create(&mode->priv->mtx))
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create mutex\n",
                          __FUNCTION__);
            UTILMEM_FREE(mode->priv->sockq);
            UTILMEM_FREE(mode->priv);
            mode->priv = NULL;
        }
        else if (!cvobj_create(&mode->priv->cv))
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create condition variable\n",
                          __FUNCTION__);
            mutexobj_destroy(&mode->priv->mtx);
            UTILMEM_FREE(mode->priv->sockq);
            UTILMEM_FREE(mode->priv);
            mode->priv = NULL;
        }
        else if (!threadpool_create(&mode->priv->threadpool, args->threads + 1))
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create thread pool\n",
                          __FUNCTION__);
            cvobj_destroy(&mode->priv->cv);
            mutexobj_destroy(&mode->priv->mtx);
            UTILMEM_FREE(mode->priv->sockq);
            UTILMEM_FREE(mode->priv);
            mode->priv = NULL;
        }
        else
        {
            mode->ops.mode_create  = modeperf_create;
            mode->ops.mode_destroy = modeperf_destroy;
            mode->ops.mode_start   = modeperf_start;
            mode->ops.mode_stop    = modeperf_stop;
            mode->ops.mode_cancel  = modeperf_cancel;

            ret = true;
        }
    }

    return ret;
}

bool modeperf_destroy(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        mode->ops.mode_stop(mode);
        threadpool_destroy(&mode->priv->threadpool);
        cvobj_destroy(&mode->priv->cv);
        mutexobj_destroy(&mode->priv->mtx);
        UTILMEM_FREE(mode->priv->sockq);
        UTILMEM_FREE(mode->priv);
        mode->priv = NULL;
        memset(&mode->ops, 0, sizeof(mode->ops));

        ret = true;
    }

    return ret;
}

/**
 * @brief Set a socket configuration.
 *
 * @param[in,out] obj       A pointer to a socket object.
 * @param[in]     timeoutms Socket timeout in milliseconds.
 *
 * @return Void.
 */
static void modeperf_conf(struct modeobj_priv * const mode,
                          struct sockobj * const sock,
                          const int32_t timeoutms)
{
    memcpy(sock->conf.ipaddr, mode->args.ipaddr, sizeof(sock->conf.ipaddr));
    sock->conf.ipport        = mode->args.ipport;
    sock->conf.backlog       = mode->args.backlog;
    sock->conf.timeoutms     = timeoutms;
    sock->conf.datalimitbyte = mode->args.datalimitbyte;
    sock->conf.ratelimitbps  = mode->args.ratelimitbps;
    sock->conf.timelimitusec = mode->args.timelimitusec;
    sock->conf.family        = mode->args.family;
    sock->conf.type          = mode->args.type;
    sock->conf.model         = mode->args.arch;
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
static int32_t modeperf_call(struct modeobj_priv * const mode,
                             int32_t (*call)(struct sockobj * const obj,
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

    if (mode->args.datalimitbyte > 0)
    {
        if (stats->totalbytes < mode->args.datalimitbyte)
        {
            len = mode->args.datalimitbyte - stats->totalbytes;

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
    else
    {
        if ((sock->event.ops.fion_poll(&sock->event) == false) ||
            (sock->event.ops.fion_getevents(&sock->event, 0) & FIONOBJ_REVENT_ERROR))
        {
            sock->ops.sock_close(sock);
            sock->ops.sock_destroy(sock);
        }
    }

    if (ret < 0)
    {
        sock->ops.sock_close(sock);
        sock->ops.sock_destroy(sock);
    }
    else if ((mode->args.timelimitusec > 0) &&
             ((tsus - sock->info.startusec) >= mode->args.timelimitusec))
    {
        sock->ops.sock_close(sock);
        sock->ops.sock_destroy(sock);
    }
    else if ((mode->args.datalimitbyte > 0) &&
             (stats->totalbytes >= mode->args.datalimitbyte))
    {
        sock->ops.sock_close(sock);
        sock->ops.sock_destroy(sock);
    }

    tokenbucket_return(&sock->tb, ret < 0 ? 0 : len - (uint32_t)ret);

    return ret;
}

/**
 * @brief Get a new socket from a socket queue.
 *
 * @param[in] qid       A socket queue id.
 * @param[in] timeoutms The maximum amount of time in milliseconds to wait for a
 *                      new socket.
 *
 * @return A pointer to a new socket on success. Otherwise, a null pointer is
 *         returned.
 */
static struct sockobj *modeperf_getsock(struct modeobj_priv * const mode,
                                        const uint32_t qid,
                                        const uint32_t timeoutms,
                                        bool * const shutdown)
{
    struct sockobj *ret = NULL;

    if (UTILDEBUG_VERIFY((mode != NULL) &&
                         (qid < mode->args.threads) &&
                         (shutdown != NULL)))
    {
        mutexobj_lock(&mode->mtx);

        if ((mode->sockq[qid].tail == NULL) && (timeoutms > 0))
        {
            cvobj_timedwait(&mode->cv, &mode->mtx, timeoutms * 1000);
        }

        if ((mode->sockq[qid].tail != NULL) &&
            ((ret = mode->sockq[qid].tail->val) != NULL))
        {
            dlist_removetail(&mode->sockq[qid]);
        }

        if ((mode->args.arch == SOCKOBJ_MODEL_CLIENT) &&
            (mode->total_socks == mode->args.maxcon) &&
            (mode->sock_count == 0))
        {
            *shutdown = true;
        }
        else
        {
            *shutdown = false;
        }

        mutexobj_unlock(&mode->mtx);
    }

    return ret;
}

static bool modeperf_retsock(struct modeobj_priv * const mode,
                             struct sockobj * const sock)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (sock != NULL)))
    {
        UTILMEM_FREE(sock);

        mutexobj_lock(&mode->mtx);
        mode->sock_count--;
        mutexobj_unlock(&mode->mtx);

        ret = true;
    }

    return ret;
}

/**
 * @brief A scheduler that accepts new sockets and inserts them into queue(s)
 *        based on a round-robin algorithm.
 *
 * @param[in,out] arg A pointer to a thread pool.
 *
 * @return NULL.
 */
static void *modeperf_acceptthread(void *arg)
{
    struct modeobj_priv *mode = (struct modeobj_priv*)arg;
    struct sockobj server, *sock = NULL;
    struct formobj form;
    bool exit = true;
    int32_t formbytes = 0;
    uint32_t qid = 0, tid = 0, socks = 0;

    memset(&server, 0, sizeof(server));
    memset(&form, 0, sizeof(form));

    if (UTILDEBUG_VERIFY(mode != NULL) && formperf_create(&form, 4096))
    {
        modeperf_conf(mode, &server, 500);
        form.sock = &server;
        exit = !sockmod_init(&server);
        tid = threadpool_getid(&mode->threadpool);

        logger_printf(LOGGER_LEVEL_INFO,
                      "Accepting sockets on thread id %u\n",
                      tid);

        while (!exit && threadpool_isrunning(&mode->threadpool))
        {
            if (sock == NULL)
            {
                sock = UTILMEM_CALLOC(struct sockobj,
                                      sizeof(struct sockobj),
                                      1);
            }

            if (server.ops.sock_accept(&server, sock))
            {
                if (sock != NULL)
                {
                    mutexobj_lock(&mode->mtx);

                    if (dlist_inserttail(&mode->sockq[qid], sock))
                    {
                        logger_printf(LOGGER_LEVEL_INFO,
                                      "%s: accepted socket on queue %u\n",
                                      __FUNCTION__,
                                      qid);
                        qid = (qid + 1 ) % mode->args.threads;
                        mode->sock_count++;
                        socks = mode->sock_count;
                        sock = NULL;
                    }
                    else
                    {
                        sock->ops.sock_close(sock);
                        sock->ops.sock_destroy(sock);
                    }

                    mutexobj_unlock(&mode->mtx);
                    //cvobj_signalone(&mode->cv);

                    if (socks == 1)
                    {
                        formbytes = form.ops.form_head(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                    }
                }
            }
            else if (server.event.revents & FIONOBJ_REVENT_ERROR)
            {
                server.ops.sock_close(&server);
                server.ops.sock_destroy(&server);
                exit = true;
            }

            mutexobj_lock(&mode->mtx);
            socks = mode->sock_count;
            mutexobj_unlock(&mode->mtx);
            if (socks == 0)
            {
                formbytes = formobj_idle(&form);
                output_if_std_send(form.dstbuf, formbytes);
                formbytes = utilstring_concat(form.dstbuf,
                                              form.dstlen,
                                              "%c",
                                              '\r');
                output_if_std_send(form.dstbuf, formbytes);
            }
        }

        form.ops.form_destroy(&form);
    }

    return NULL;
}

/**
 * @brief A scheduler that connects new sockets and inserts them into queue(s)
 *        based on a round-robin algorithm.
 *
 * @param[in,out] arg A pointer to a thread pool.
 *
 * @return NULL.
 */
static void *modeperf_connectthread(void *arg)
{
    struct modeobj_priv *mode = (struct modeobj_priv*)arg;
    struct sockobj *sock = NULL;
    struct formobj form;
    uint32_t i = 0, qid = 0, tid = 0, socks = 0;
    int32_t formbytes = 0;

    memset(&form, 0, sizeof(form));

    if (UTILDEBUG_VERIFY((mode != NULL) && formperf_create(&form, 4096)))
    {
        tid = threadpool_getid(&mode->threadpool);

        logger_printf(LOGGER_LEVEL_INFO,
                      "Connecting sockets on thread id %u\n",
                      tid);

        for (i = 0; (threadpool_isrunning(&mode->threadpool)) && (i < mode->args.maxcon); i++)
        {
            if (sock == NULL)
            {
                sock = UTILMEM_CALLOC(struct sockobj,
                                      sizeof(struct sockobj),
                                      1);
            }

            if (sock == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to allocate memory\n",
                              __FUNCTION__);
            }
            else
            {
                modeperf_conf(mode, sock, 0);

                if (!sockmod_init(sock))
                {
                    break;
                    //exit = true; ///only for multiple connections??
                }
                else
                {
                    sock->ops.sock_connect(sock);

                    mutexobj_lock(&mode->mtx);

                    if (!dlist_inserttail(&mode->sockq[qid], sock))
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: failed to store allocated memory\n",
                                      __FUNCTION__);

                        sock->ops.sock_close(sock);
                        sock->ops.sock_destroy(sock);
                    }
                    else
                    {
                        logger_printf(LOGGER_LEVEL_INFO,
                                      "%s: connected socket on queue %u\n",
                                      __FUNCTION__,
                                      qid);
                        qid = (qid + 1 ) % mode->args.threads;
                        mode->sock_count++;
                        mode->total_socks++;
                        socks++;
                        form.sock = sock;
                        sock = NULL;
                    }

                    mutexobj_unlock(&mode->mtx);
                    //cvobj_signalone(&mode->cv);

                    if (socks == 1)
                    {
                        formbytes = form.ops.form_head(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                    }
                }
            }
        }

        if (sock != NULL)
        {
            UTILMEM_FREE(sock);
            sock = NULL;
        }

        form.ops.form_destroy(&form);
    }

    return NULL;
}

/**
 * @brief Perform a performance mode task.
 *
 * @param[in,out] arg A pointer to a thread pool.
 *
 * @return NULL.
 */
static void *modeperf_workerthread(void *arg)
{
    bool exit = true;
    // @todo Get the actual thread object from the thread pool so that all
    //       threads are not trying to access the thread-safe
    //       threadpool_isrunning() in the while loop.
    struct fionobj fion;
    struct modeobj_priv *mode = (struct modeobj_priv*)arg;
    struct dlist list;
    struct sockobj group, *sock = NULL;
    struct formobj form;
    uint8_t *recvbuf = NULL, *sendbuf = NULL;
    int32_t formbytes = 0, recvbytes = 0, sendbytes = 0;
    uint32_t count = 0, tid = 0;

    struct dlist_node *next = NULL, *node = NULL;
    struct sockobj_flowstats *stats = NULL;
    struct utilcpu_info info;
    uint64_t activetimeus = 0;
    uint64_t inactivetimeus = 1000;
    uint64_t delayus = 0, mindelayus = 0;
    uint64_t tsus = 0;
    uint32_t burstlimit = mode->args.backlog <= 0 ? SOMAXCONN : mode->args.backlog;
    uint32_t burst = 0;
    memset(&form, 0, sizeof(form));
    memset(&fion, 0, sizeof(fion));
    memset(&group, 0, sizeof(group));

    if (!UTILDEBUG_VERIFY(mode != NULL))
    {
        // Do nothing.
    }
    else if (!formperf_create(&form, 4096))
    {
        // Do nothing.
    }
    else if (!fionpoll_create(&fion))
    {
        form.ops.form_destroy(&form);
    }
    else if ((recvbuf = UTILMEM_CALLOC(uint8_t,
                                       sizeof(uint8_t),
                                       mode->args.buflen)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: receive buffer allocation failed\n",
                      __FUNCTION__);
        form.ops.form_destroy(&form);
    }
    else if ((sendbuf = UTILMEM_CALLOC(uint8_t,
                                       sizeof(uint8_t),
                                       mode->args.buflen)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: send buffer allocation failed\n",
                      __FUNCTION__);

        UTILMEM_FREE(recvbuf);
        recvbuf = NULL;
        form.ops.form_destroy(&form);
    }
    else
    {
        exit = false;
        form.intervalusec = mode->args.intervalusec;
        tid = threadpool_getid(&mode->threadpool);
        fion.timeoutms = 0;
        fion.pevents = FIONOBJ_PEVENT_IN;
        memset(&list, 0, sizeof(list));

        while ((exit == false) && (threadpool_isrunning(&mode->threadpool) == true))
        {
            burst = count;

            while ((exit == false) && ((count - burst) < burstlimit))
            {
                sock = NULL;
                if (mode->args.arch == SOCKOBJ_MODEL_CLIENT)
                {
                    if ((sock = modeperf_getsock(mode, tid, list.size > 0 ? 0 : 500, &exit)) != NULL)
                    {
                        dlist_inserttail(&list, sock);
                        fion.ops.fion_insertfd(&fion, sock->fd);
                        sock->sid = ++count;
                        sock->tid = tid;
                        form.sock = &group;

                        if (count == 1)
                        {
                            group.info.startusec = sock->info.startusec;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    if ((sock = modeperf_getsock(mode, tid, list.size > 0 ? 0 : 500, &exit)) != NULL)
                    {
                        dlist_inserttail(&list, sock);
                        if ((mode->args.maxcon == 0) ||
                            (list.size <= mode->args.maxcon))
                        {
                            fion.ops.fion_insertfd(&fion, sock->fd);
                            sock->sid = ++count;
                            sock->tid = tid;
                            sock->event.timeoutms = 0;

                            if (list.size == 1)
                            {
                                group.info.startusec = sock->info.startusec;
                            }
                        }
                        else
                        {
                            // Refuse connection.
                            sock->ops.sock_close(sock);
                            sock->ops.sock_destroy(sock);
                            modeperf_retsock(mode, list.tail->val);
                            dlist_removetail(&list);
                            sock = NULL;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            node = list.head;

            while (node != NULL)
            {
                sock = node->val;
                form.sock = &group;

                // @todo Perform once per iteration?
                tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
                form.tsus = tsus;

                if (activetimeus == 0)
                {
                    activetimeus = tsus;
                }

                if (mode->args.arch == SOCKOBJ_MODEL_CLIENT)
                {
                    stats = &sock->info.send;

                    if ((sock->state & SOCKOBJ_STATE_CONNECT) == 0)
                    {
                        sock->ops.sock_connect(sock);
                        sendbytes = 0;
                    }
                    else
                    {
                        sendbytes = modeperf_call(mode,
                                                  sock->ops.sock_send,
                                                  &sock->info.send,
                                                  sock,
                                                  sendbuf,
                                                  mindelayus > 0 ? 0 : mode->args.buflen,
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
                                                            mode->args.buflen * 8);

                                if ((delayus < mindelayus) || (mindelayus == 0))
                                {
                                    mindelayus = delayus;
                                }
                            }
                            else if (tsus - activetimeus > inactivetimeus)
                            {
                                fion.timeoutms++;
                                fion.pevents = FIONOBJ_PEVENT_OUT;
                            }
                        }
                        else
                        {
                            fion.timeoutms = 0;
                            fion.pevents = FIONOBJ_PEVENT_IN;
                            activetimeus = tsus;
                        }
                    }
                }
                else
                {
                    stats = &sock->info.recv;

                    recvbytes = modeperf_call(mode,
                                              sock->ops.sock_recv,
                                              &sock->info.recv,
                                              sock,
                                              recvbuf,
                                              mindelayus > 0 ? 0 : mode->args.buflen,
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
                                                            mode->args.buflen * 8);

                                if ((delayus < mindelayus) || (mindelayus == 0))
                                {
                                    mindelayus = delayus;
                                }
                            }
                            else if (tsus - activetimeus > inactivetimeus)
                            {
                                fion.timeoutms++;
                            }
                        }
                        else
                        {
                            fion.timeoutms = 0;
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

                    //form.sock = sock;
                    //formbytes = form.ops.form_foot(&form);
                    //output_if_std_send(form.dstbuf, formbytes);

                    utilcpu_getinfo(&info);
                    logger_printf(LOGGER_LEVEL_DEBUG,
                                  "%s: tid: %u cpu load: %d usr/sys time sec: %u.%06u / %u.%06u\n",
                                  __FUNCTION__,
                                  tid,
                                  info.usage,
                                  info.usrtime.tv_sec,
                                  info.usrtime.tv_usec,
                                  info.systime.tv_sec,
                                  info.systime.tv_usec);
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
                    modeperf_retsock(mode, node->val);
                    dlist_remove(&list, node);
                    node = next;
                    fion.ops.fion_deletefd(&fion, sock->fd);

                    if (list.size == 0)
                    {
                        group.sid = count;
                        group.tid = tid;
                        form.sock = &group;
                        formbytes = form.ops.form_foot(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        memset(&group, 0, sizeof(group));

                        if (mode->args.arch == SOCKOBJ_MODEL_CLIENT)
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
                // @todo Replace with semaphore.
                if (mindelayus > 100000)
                {
                    //usleep(100000);
                    mindelayus -= 100000;
                }
                else
                {
                    usleep((uint32_t)mindelayus);
                    mindelayus = 0;
                }
            }
            else if ((list.size > 0) && ((uint32_t)fion.timeoutms == list.size))
            {
                fion.timeoutms = 1;
                fion.ops.fion_setflags(&fion);
                fion.ops.fion_poll(&fion);
                fion.timeoutms = 0;
            }
        }

        UTILMEM_FREE(recvbuf);
        UTILMEM_FREE(sendbuf);
        fionpoll_destroy(&fion);
        form.ops.form_destroy(&form);
    }

    return NULL;
}

bool modeperf_start(struct modeobj * const mode)
{
    bool ret = false;
    uint32_t i;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        threadpool_stop(&mode->priv->threadpool);
        ret = threadpool_start(&mode->priv->threadpool);

        for (i = 0; i < mode->priv->args.threads; i++)
        {
            ret &= threadpool_execute(&mode->priv->threadpool,
                                      modeperf_workerthread,
                                      mode->priv,
                                      i);
        }

        switch (mode->priv->args.arch)
        {
            case SOCKOBJ_MODEL_CLIENT:
                ret &= threadpool_execute(&mode->priv->threadpool,
                                          modeperf_connectthread,
                                          mode->priv,
                                          mode->priv->args.threads);
                break;
            case SOCKOBJ_MODEL_SERVER:
                ret &= threadpool_execute(&mode->priv->threadpool,
                                          modeperf_acceptthread,
                                          mode->priv,
                                          mode->priv->args.threads);
                break;
            default:
                break;
        }

        threadpool_wait(&mode->priv->threadpool, mode->priv->args.threads + 1);
    }

    return ret;
}

bool modeperf_stop(struct modeobj * const mode)
{
    bool ret = false;
    uint32_t i;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        ret  = mode->ops.mode_cancel(mode);
        ret &= threadpool_stop(&mode->priv->threadpool);

        for (i = 0; i < mode->priv->args.threads; i++)
        {
            while (mode->priv->sockq[i].size > 0)
            {
                dlist_removehead(&mode->priv->sockq[i]);
            }
        }
    }

    return ret;
}

bool modeperf_cancel(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        ret = threadpool_wake(&mode->priv->threadpool);
    }

    return ret;
}
