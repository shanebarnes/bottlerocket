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
    uint16_t           parts;
    struct args_obj    args;
    struct threadpool  threadpool;
    struct mutexobj   *mtxarr;
    struct cvobj      *cvarr;
    struct dlist      *sockq;
    uint32_t          *activesocks;
    uint32_t          *closedsocks;
    uint32_t          *configsocks;
    struct sockobj    *workerstats;
    struct formobj    *workerforms;
};

/**
 * @brief Destroy a fully or partially constructed mode object.
 *
 * @param[in,out] mode A pointer to a mode object.
 *
 * @return Void.
 */
static void modeperf_destroyparts(struct modeobj * const mode)
{
    uint32_t i;

    switch (mode->priv->parts)
    {
        case 12:
            memset(&mode->ops, 0, sizeof(mode->ops));
            for (i = 0; i < mode->priv->args.threads; i++)
            {
                cvobj_destroy(&mode->priv->cvarr[i]);
                mutexobj_destroy(&mode->priv->mtxarr[i]);
            }
        case 11:
            threadpool_destroy(&mode->priv->threadpool);
        case 10:
            UTILMEM_FREE(mode->priv->workerforms);
        case 9:
            UTILMEM_FREE(mode->priv->workerstats);
        case 8:
            UTILMEM_FREE(mode->priv->configsocks);
        case 7:
            UTILMEM_FREE(mode->priv->closedsocks);
        case 6:
            UTILMEM_FREE(mode->priv->activesocks);
        case 5:
            UTILMEM_FREE(mode->priv->cvarr);
        case 4:
            UTILMEM_FREE(mode->priv->mtxarr);
        case 3:
            UTILMEM_FREE(mode->priv->sockq);
        case 2:
            memset(&mode->priv->args, 0, sizeof(mode->priv->args));
        case 1:
            UTILMEM_FREE(mode->priv);
            mode->priv = NULL;
        case 0:
            break;
        default:
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: invalid mode object part (%u)\n",
                          __FUNCTION__,
                          mode->priv->parts);
            break;
    }
}

bool modeperf_create(struct modeobj * const mode,
                     const struct args_obj * const args)
{
    bool ret = false;
    uint32_t i;

    if (UTILDEBUG_VERIFY((mode != NULL) &&
                         (mode->priv == NULL) &&
                         (args != NULL)))
    {
        if ((mode->priv = UTILMEM_CALLOC(struct modeobj_priv,
                                         sizeof(struct modeobj_priv),
                                         1)) == NULL)
        {
            // Do nothing
        }
        else if (memcpy(&mode->priv->args,
                        args,
                        sizeof(mode->priv->args)) == NULL)
        {
            mode->priv->parts = 1;
        }
        else if ((mode->priv->sockq = UTILMEM_CALLOC(struct dlist,
                                                     sizeof(struct dlist),
                                                     args->threads)) == NULL)
        {
            mode->priv->parts = 2;
        }
        else if ((mode->priv->mtxarr = UTILMEM_CALLOC(struct mutexobj,
                                                     sizeof(struct mutexobj),
                                                     args->threads)) == NULL)
        {
            mode->priv->parts = 3;
        }
        else if ((mode->priv->cvarr = UTILMEM_CALLOC(struct cvobj,
                                                     sizeof(struct cvobj),
                                                     args->threads)) == NULL)
        {
            mode->priv->parts = 4;
        }
        else if ((mode->priv->activesocks = UTILMEM_CALLOC(uint32_t,
                                                           sizeof(uint32_t),
                                                           args->threads)) == NULL)
        {
            mode->priv->parts = 5;
        }
        else if ((mode->priv->closedsocks = UTILMEM_CALLOC(uint32_t,
                                                           sizeof(uint32_t),
                                                           args->threads)) == NULL)
        {
            mode->priv->parts = 6;
        }
        else if ((mode->priv->configsocks = UTILMEM_CALLOC(uint32_t,
                                                           sizeof(uint32_t),
                                                           args->threads)) == NULL)
        {
            mode->priv->parts = 7;
        }
        else if ((mode->priv->workerstats = UTILMEM_CALLOC(struct sockobj,
                                                           sizeof(struct sockobj),
                                                           args->threads)) == NULL)
        {
            mode->priv->parts = 8;
        }
        else if ((mode->priv->workerforms = UTILMEM_CALLOC(struct formobj,
                                                           sizeof(struct formobj),
                                                           args->threads)) == NULL)
        {
            mode->priv->parts = 9;
        }
        else if (!threadpool_create(&mode->priv->threadpool, args->threads + 1))
        {
            mode->priv->parts = 10;
        }
        else
        {
            mode->priv->parts = 11;

            for (i = 0; i < args->threads; i++)
            {
                mutexobj_create(&mode->priv->mtxarr[i]);
                cvobj_create(&mode->priv->cvarr[i]);
            }

            mode->ops.mode_create  = modeperf_create;
            mode->ops.mode_destroy = modeperf_destroy;
            mode->ops.mode_start   = modeperf_start;
            mode->ops.mode_stop    = modeperf_stop;
            mode->ops.mode_cancel  = modeperf_cancel;
            mode->priv->parts      = 12;

            ret = true;
        }

        if (!ret)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create mode object part %d\n",
                          __FUNCTION__,
                          mode->priv == NULL ? 1 : mode->priv->parts);

            if (mode->priv != NULL)
            {
                modeperf_destroyparts(mode);
            }
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
        modeperf_destroyparts(mode);
        ret = true;
    }

    return ret;
}

/**
 * @brief Copy a mode object configuration to a socket object configuration.
 *
 * @param[in,out] sock      A pointer to a socket object.
 * @param[in]     mode      A pointer to a mode object.
 * @param[in]     timeoutms Socket timeout in milliseconds.
 *
 * @return Void.
 */
static void modeperf_copy(struct modeobj_priv * const mode,
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
 * @brief Call a mode socket's receive or send function.
 *
 * @param[in,out] mode   A pointer to a mode object.
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
        if ((uint64_t)stats->buflen.sum < mode->args.datalimitbyte)
        {
            len = mode->args.datalimitbyte - stats->buflen.sum;

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
        if ((!sock->event.ops.fion_poll(&sock->event)) ||
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
             ((uint64_t)stats->buflen.sum >= mode->args.datalimitbyte))
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
 * @param[in,out] mode  A pointer to a mode object.
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
        *shutdown = false;

        mutexobj_lock(&mode->mtxarr[qid]);

        if ((mode->sockq[qid].tail == NULL) && (timeoutms > 0))
        {
            cvobj_timedwait(&mode->cvarr[qid],
                            &mode->mtxarr[qid],
                            timeoutms * 1000);
        }

        if ((mode->sockq[qid].tail != NULL) &&
            ((ret = mode->sockq[qid].tail->val) != NULL))
        {
            dlist_removetail(&mode->sockq[qid]);
            mode->activesocks[qid]++;
        }

        if ((mode->args.arch == SOCKOBJ_MODEL_CLIENT) &&
            (mode->activesocks[qid] == 0) &&
            (mode->closedsocks[qid] == mode->configsocks[qid]))
        {
            *shutdown = true;
        }

        mutexobj_unlock(&mode->mtxarr[qid]);
    }

    return ret;
}

static bool modeperf_retsock(struct modeobj_priv * const mode,
                             struct sockobj * const sock,
                             uint32_t qid)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) &&
                         (sock != NULL) &&
                         (qid < mode->args.threads)))
    {
        UTILMEM_FREE(sock);

        mutexobj_lock(&mode->mtxarr[qid]);
        mode->activesocks[qid]--;
        mutexobj_unlock(&mode->mtxarr[qid]);

        ret = true;
    }

    return ret;
}

/**
 * @brief A scheduler that accepts new sockets and inserts them into queue(s)
 *        based on a round-robin algorithm.
 *
 * @param[in,out] arg A pointer to a mode object.
 *
 * @return NULL.
 */
static void *modeperf_acceptthread(void *arg)
{
    struct modeobj_priv *mode = (struct modeobj_priv*)arg;
    struct sockobj server, *sock = NULL;
    bool exit = true;
    int32_t formbytes = 0;
    uint32_t acceptsocks = 0, activesocks = 0, i = 0, qid = 0, tid = 0;

    memset(&server, 0, sizeof(server));

    if (UTILDEBUG_VERIFY(mode != NULL))
    {
        for (i = 0; i < mode->args.threads; i++)
        {
            formperf_create(&mode->workerforms[i], 4096);
            modeperf_copy(mode, &mode->workerstats[i], 0);
            mode->workerforms[i].sock = &mode->workerstats[i];
            mode->workerstats[i].tid = i;
        }

        modeperf_copy(mode, &server, 500);
        mode->workerforms[0].sock = &server;
        exit = !sockmod_init(&server);

        if (exit)
        {
            // @todo Call modeperf_call() instead.
            threadpool_wake(&mode->threadpool);
        }
        else
        {
            tid = threadpool_getid(&mode->threadpool);
            logger_printf(LOGGER_LEVEL_INFO,
                          "Accepting sockets on thread id %u\n",
                          tid);
        }

        while ((!exit) && (threadpool_isrunning(&mode->threadpool)))
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
            else if (server.ops.sock_accept(&server, sock))
            {
                if (sock != NULL)
                {
                    if ((mode->args.maxcon > 0) &&
                        (acceptsocks == mode->args.maxcon))
                    {
                        logger_printf(LOGGER_LEVEL_INFO,
                                      "%s: rejected socket on queue %u\n",
                                      __FUNCTION__,
                                      qid);
                        sock->ops.sock_close(sock);
                        sock->ops.sock_destroy(sock);
                        continue;
                    }
                    else
                    {
                        logger_printf(LOGGER_LEVEL_INFO,
                                      "%s: accepted socket on queue %u\n",
                                      __FUNCTION__,
                                      qid);
                    }

                    mutexobj_lock(&mode->mtxarr[qid]);

                    if (dlist_inserttail(&mode->sockq[qid], sock))
                    {
                        sock = NULL;
                    }
                    else
                    {
                        sock->ops.sock_close(sock);
                        sock->ops.sock_destroy(sock);
                    }

                    mutexobj_unlock(&mode->mtxarr[qid]);
                    // @todo Use semaphore instead since thread being signaled
                    //       may not be blocking waiting for a signal.
                    cvobj_signalone(&mode->cvarr[qid]);

                    if (sock == NULL)
                    {
                        acceptsocks++;
                        qid = acceptsocks % mode->args.threads;

                        if (acceptsocks == 1)
                        {
                            formbytes = mode->workerforms[0].ops.form_head(&mode->workerforms[0]);
                            output_if_std_send(mode->workerforms[0].dstbuf, formbytes);
                        }
                    }
                }
            }
            else if (server.event.revents & FIONOBJ_REVENT_ERROR)
            {
                server.ops.sock_close(&server);
                server.ops.sock_destroy(&server);
                exit = true;
            }
            else
            {
                activesocks = 0;

                for (i = 0; i < mode->args.threads; i++)
                {
                    mutexobj_lock(&mode->mtxarr[i]);
                    activesocks += mode->activesocks[i];
                    mutexobj_unlock(&mode->mtxarr[i]);
                }

                if (activesocks == 0)
                {
                    formbytes = formobj_idle(&mode->workerforms[0]);
                    output_if_std_send(mode->workerforms[0].dstbuf, formbytes);
                    formbytes = utilstring_concat(mode->workerforms[0].dstbuf,
                                                  mode->workerforms[0].dstlen,
                                                  "%c",
                                                  '\r');
                    output_if_std_send(mode->workerforms[0].dstbuf, formbytes);
                    acceptsocks = 0;
                }
            }
        }

        if (sock != NULL)
        {
            UTILMEM_FREE(sock);
            sock = NULL;
        }

        for (i = 0; i < mode->args.threads; i++)
        {
            mode->workerforms[i].ops.form_destroy(&mode->workerforms[i]);
        }
    }

    return NULL;
}

/**
 * @brief A scheduler that connects new sockets and inserts them into queue(s)
 *        based on a round-robin algorithm.
 *
 * @param[in,out] arg A pointer to a mode object.
 *
 * @return NULL.
 */
static void *modeperf_connectthread(void *arg)
{
    struct modeobj_priv *mode = (struct modeobj_priv*)arg;
    struct sockobj *sock = NULL;
    uint32_t connectsocks = 0, i = 0, qid = 0, tid = 0;
    int32_t formbytes = 0;

    if (UTILDEBUG_VERIFY(mode != NULL))
    {
        tid = threadpool_getid(&mode->threadpool);

        for (i = 0; i < mode->args.threads; i++)
        {
            formperf_create(&mode->workerforms[i], 4096);
            modeperf_copy(mode, &mode->workerstats[i], 0);
            mode->workerforms[i].sock = &mode->workerstats[i];
            mode->workerstats[i].tid = i;
        }

        logger_printf(LOGGER_LEVEL_INFO,
                      "Connecting sockets on thread id %u\n",
                      tid);

        for (i = 0;
             (threadpool_isrunning(&mode->threadpool)) &&
             (i < mode->args.maxcon);
             i++)
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
                modeperf_copy(mode, sock, 0);

                if (!sockmod_init(sock))
                {
                    break; // @todo Only for multiple connections?
                }
                else
                {
                    sock->ops.sock_connect(sock);
                    logger_printf(LOGGER_LEVEL_INFO,
                                  "%s: connected socket on queue %u\n",
                                  __FUNCTION__,
                                  qid);

                    mutexobj_lock(&mode->mtxarr[qid]);

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
                        if (connectsocks < mode->args.threads)
                        {
                            mode->configsocks[qid] = 1;
                        }
                        else
                        {
                            mode->configsocks[qid]++;
                        }

                        utilstring_concat(mode->workerstats[qid].addrself.sockaddrstr,
                                          sizeof(mode->workerstats[qid].addrself.sockaddrstr),
                                          "%s:*",
                                          sock->conf.ipaddr);
                        utilstring_concat(mode->workerstats[qid].addrpeer.sockaddrstr,
                                          sizeof(mode->workerstats[qid].addrpeer.sockaddrstr),
                                          "%s:%u",
                                          sock->conf.ipaddr, sock->conf.ipport);

                        sock = NULL;
                    }

                    mutexobj_unlock(&mode->mtxarr[qid]);
                    // @todo Use semaphore instead since thread being signaled
                    //       may not be blocking waiting for a signal.
                    cvobj_signalone(&mode->cvarr[qid]);

                    if (sock == NULL)
                    {
                        connectsocks++;
                        qid = connectsocks % mode->args.threads;

                        if (connectsocks == 1)
                        {
                            formbytes = mode->workerforms[0].ops.form_head(&mode->workerforms[0]);
                            output_if_std_send(mode->workerforms[0].dstbuf, formbytes);
                        }
                    }
                }
            }
        }

        if (sock != NULL)
        {
            UTILMEM_FREE(sock);
            sock = NULL;
        }

        for (i = 0; i < mode->args.threads; i++)
        {
            if (mode->configsocks[i] == 0xFFFFFFFF)
            {
                mode->configsocks[i] = 0;
            }

            mode->workerforms[i].ops.form_destroy(&mode->workerforms[i]);
        }
    }

    return NULL;
}

/**
 * @brief Perform a performance mode task.
 *
 * @param[in,out] arg A pointer to a mode object.
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
    struct sockobj *sock = NULL;
    struct formobj form;
    uint8_t *recvbuf = NULL, *sendbuf = NULL;
    int32_t formbytes = 0, recvbytes = 0, sendbytes = 0;
    uint32_t count = 0, tid = 0;

    struct dlist_node *next = NULL, *node = NULL;
    struct sockobj_flowstats *stats = NULL;
    struct utilcpu_info info;
    uint64_t delayus = 0, mindelayus = 0;
    uint64_t tsus = 0;
    uint32_t burstlimit = mode->args.backlog <= 0 ? SOMAXCONN : mode->args.backlog;
    uint32_t burst = 0;
    memset(&form, 0, sizeof(form));
    memset(&fion, 0, sizeof(fion));

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

        while ((!exit) && (threadpool_isrunning(&mode->threadpool)))
        {
            burst = count;

            while ((!exit) && ((count - burst) < burstlimit))
            {
                if ((sock = modeperf_getsock(mode,
                                             tid,
                                             list.size > 0 ? 0 : 500,
                                             &exit)) != NULL)
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
                            mode->workerstats[tid].info.startusec = sock->info.startusec;
                        }
                    }
                    else
                    {
                        // Refuse connection.
                        sock->ops.sock_close(sock);
                        sock->ops.sock_destroy(sock);
                        modeperf_retsock(mode, list.tail->val, tid);
                        dlist_removetail(&list);
                        sock = NULL;
                    }
                }
                else
                {
                    break;
                }
            }

            node = list.head;

            while (node != NULL)
            {
                sock = node->val;
                form.sock = &mode->workerstats[tid];

                // @todo Perform once per iteration?
                tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
                form.tsus = tsus;

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
                        mode->workerstats[tid].info.send.buflen.sum += sendbytes;
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
                            else
                            {
                                fion.timeoutms++;
                                fion.pevents = FIONOBJ_PEVENT_OUT;
                            }
                        }
                        else
                        {
                            fion.timeoutms = 0;
                            fion.pevents = FIONOBJ_PEVENT_IN;
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
                        mode->workerstats[tid].info.recv.buflen.sum += recvbytes;
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
                            else
                            {
                                fion.timeoutms++;
                            }
                        }
                        else
                        {
                            fion.timeoutms = 0;
                        }
                    }
                }

                if (sock->state & SOCKOBJ_STATE_CLOSE)
                {
                    if (list.size == 1)
                    {
                        mode->workerstats[tid].info.stopusec = sock->info.stopusec;
                    }

                    form.sock = sock;
                    formbytes = form.ops.form_foot(&form);
                    output_if_std_send(form.dstbuf, formbytes);

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
                                  "%s: buflen avg/min/max: %" PRIi64
                                  " / %" PRIi64
                                  " / %" PRIi64 "\n",
                                  __FUNCTION__,
                                  stats->buflen.avg,
                                  stats->buflen.min,
                                  stats->buflen.max);

                    next = node->next;
                    modeperf_retsock(mode, node->val, tid);
                    dlist_remove(&list, node);
                    node = next;
                    fion.ops.fion_deletefd(&fion, sock->fd);

                    if (list.size == 0)
                    {
                        mode->workerstats[tid].conf.model = mode->args.arch;
                        mode->workerstats[tid].sid = count;
                        mode->workerstats[tid].tid = tid;
                        form.sock = &mode->workerstats[tid];
                        formbytes = form.ops.form_foot(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        memset(&mode->workerstats[tid],
                               0,
                               sizeof(mode->workerstats[tid]));

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
            mode->priv->configsocks[i] = 0xFFFFFFFF;
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
