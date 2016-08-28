/**
 * @file      sock_con.c
 * @brief     Socket connection manager implementation.
 * @author    Shane Barnes
 * @date      23 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "fion_poll.h"
#include "logger.h"
#include "mutex_obj.h"
#include "sock_con.h"
#include "thread_obj.h"
#include "util_date.h"
#include "util_debug.h"
#include "util_inet.h"
#include "util_mem.h"
#include "util_unit.h"
#include "vector.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

static const uint32_t SOCKCON_PEER_INDEX   = 0;
static const uint32_t SOCKCON_SELF_INDEX   = 1;
static const uint32_t SOCKCON_TIMEOUT_USEC = 10 * UNIT_TIME_USEC;

struct sockcon_priv
{
    int32_t          maxcon;
    struct fionobj   fion;
    struct mutexobj  mutex;
    struct threadobj thread;
    struct vector    backlog;
    struct vector    frontlog;
};

struct sockcon_pair
{
    struct sockaddr_storage sockaddr;
    uint32_t                hash; // @note Just port number for now (assume localhost peer)
    int32_t                 fds[2];
    uint64_t                timeoutus;
};

/**
 * @brief Send/route a datagram received from a UDP listener socket to a
 *        "connected" UDP socket via a local socket pair. Garbage collection is
 *        also performed on timed out "connections."
 *
 * @param[in,out] vec  A pointer to a socket pair queue.
 * @param[in]     hash Datagram hash.
 * @param[in]     tsus Current Unix timestamp in microseconds.
 * @param[in]     buf  A pointer to a buffer containing a datagram.
 * @param[in]     len  Datagram length in bytes.
 *
 * @return True if a datagram was sent to a "connection" UDP socket.
 */
bool sockcon_send(struct vector * const vec,
                  const uint32_t hash,
                  const uint64_t tsus,
                  const uint8_t * const buf,
                  const uint32_t len)
{
    bool ret = false;
    int32_t i = 0;
    struct sockcon_pair *pair = NULL;

    for (i = vector_getsize(vec) - 1; i >= 0; i--)
    {
        pair = vector_getval(vec, i);

        if (pair->hash == hash)
        {
            send(pair->fds[SOCKCON_SELF_INDEX], buf, len, 0);
            pair->timeoutus = tsus + SOCKCON_TIMEOUT_USEC;
            ret = true;
        }
        else if (tsus >= pair->timeoutus)
        {
            logger_printf(LOGGER_LEVEL_DEBUG,
                          "%s: pair %u timeout\n",
                          __FUNCTION__,
                          i);
            close(pair->fds[SOCKCON_SELF_INDEX]);
            vector_delete(vec, i);
        }
    }

    return ret;
}

/**
 * @brief Listener thread that inspects and routes all incoming datagrams.
 *
 * @param[in] arg A pointer to a socket connection manager.
 *
 * @return Null.
 */
static void *sockcon_thread(void * const arg)
{
    struct sockcon *con = (struct sockcon *)arg;
    struct sockcon_pair pair;
    bool exit = false;
    uint32_t events = 0, hash = 0;
    uint64_t tsus = 0;
    int32_t recvbytes = 0;
    uint8_t recvbuf[65536];

    if (UTILDEBUG_VERIFY((con != NULL) && (con->sock != NULL)) == true)
    {
        con->priv->fion.timeoutms = 500;
        con->priv->fion.pevents = FIONOBJ_PEVENT_IN;
        con->priv->fion.ops.fion_insertfd(&con->priv->fion, con->sock->fd);
        con->priv->fion.ops.fion_setflags(&con->priv->fion);

        while ((exit == false) &&
               (threadobj_isrunning(&con->priv->thread) == true))
        {
            con->priv->fion.ops.fion_poll(&con->priv->fion);
            tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
            events = con->priv->fion.ops.fion_getevents(&con->priv->fion, 0);

            if (events & FIONOBJ_REVENT_ERROR)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: listening socket has error\n",
                              __FUNCTION__);
                exit = true;
            }
            else if (events & FIONOBJ_REVENT_INREADY)
            {
                recvbytes = con->sock->ops.sock_recv(con->sock,
                                                     recvbuf,
                                                     sizeof(recvbuf));

                if (recvbytes >= 0)
                {
                    hash = ntohs(*utilinet_getportfromstorage(&con->sock->addrpeer.sockaddr));

                    mutexobj_lock(&con->priv->mutex);

                    if (sockcon_send(&con->priv->frontlog,
                                     hash,
                                     tsus,
                                     recvbuf,
                                     recvbytes) == true)
                    {
                        // Do nothing.
                    }
                    else if (sockcon_send(&con->priv->backlog,
                                          hash,
                                          tsus,
                                          recvbuf,
                                          recvbytes) == true)
                    {
                        // Do nothing.
                    }
                    else if ((int32_t)vector_getsize(&con->priv->backlog) <
                             con->priv->maxcon)
                    {
                        if (socketpair(AF_UNIX, SOCK_DGRAM, 0, pair.fds) != 0)
                        {
                            logger_printf(LOGGER_LEVEL_ERROR,
                                          "%s: failed to create socket pair (%d)\n",
                                          __FUNCTION__,
                                          errno);
                        }
                        else
                        {
                            memcpy(&pair.sockaddr,
                                   &con->sock->addrpeer.sockaddr,
                                   sizeof(pair.sockaddr));
                            pair.hash = hash;
                            vector_inserttail(&con->priv->backlog, &pair);
                            sockcon_send(&con->priv->backlog,
                                         hash,
                                         tsus,
                                         recvbuf,
                                         recvbytes);
                        }
                    }
                    else
                    {
                        logger_printf(LOGGER_LEVEL_WARN,
                                      "%s: backlog is full (%d)\n",
                                      __FUNCTION__,
                                      con->priv->maxcon);
                    }

                    mutexobj_unlock(&con->priv->mutex);
                }
            }
            else
            {
                mutexobj_lock(&con->priv->mutex);
                sockcon_send(&con->priv->backlog,
                             0,
                             tsus,
                             NULL,
                             0);
                sockcon_send(&con->priv->frontlog,
                             0,
                             tsus,
                             NULL,
                             0);
                mutexobj_unlock(&con->priv->mutex);
            }
        }
    }

    return NULL;
}

/**
 * @see See header file for interface comments.
 */
bool sockcon_create(struct sockcon * const con)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((con != NULL) &&
                         (con->sock == NULL) &&
                         (con->priv == NULL)) == true)
    {
        con->priv = UTILMEM_CALLOC(struct sockcon_priv,
                                   sizeof(struct sockcon_priv),
                                   1);
con->priv->thread.function = sockcon_thread;
con->priv->thread.argument = con;

        if (con->priv == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate private memory (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else if (vector_create(&con->priv->backlog,
                               0,
                               sizeof(struct sockcon_pair)) == false)
        {
            sockcon_destroy(con);
        }
        else if (vector_create(&con->priv->frontlog,
                               0,
                               sizeof(struct sockcon_pair)) == false)
        {
            sockcon_destroy(con);
        }
        else if (fionpoll_create(&con->priv->fion) == false)
        {
            sockcon_destroy(con);
        }
        else if (mutexobj_create(&con->priv->mutex) == false)
        {
            sockcon_destroy(con);
        }
        else if (threadobj_create(&con->priv->thread) == false)
        {
            sockcon_destroy(con);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockcon_destroy(struct sockcon * const con)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((con != NULL) && (con->priv != NULL)) == true)
    {
        threadobj_stop(&con->priv->thread);
        threadobj_destroy(&con->priv->thread);
        mutexobj_destroy(&con->priv->mutex);
        fionpoll_destroy(&con->priv->fion);
        vector_destroy(&con->priv->frontlog);
        vector_destroy(&con->priv->backlog);
        UTILMEM_FREE(con->priv);
        con->priv = NULL;

        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockcon_listen(struct sockcon * const con,
                    struct sockobj * const sock,
                    const int32_t backlog)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((con != NULL) &&
                         (con->priv != NULL) &&
                         (sock != NULL)) == true)
    {
        con->sock         = sock;
        con->priv->maxcon = (backlog > 0 ? backlog : SOMAXCONN);

        ret = threadobj_start(&con->priv->thread);;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
int32_t sockcon_accept(struct sockcon * const con,
                       struct sockaddr * const addr,
                       socklen_t * const len)
{
    int32_t ret = -1;
    uint32_t backlog = 0;
    struct sockcon_pair *pair = NULL;

    if (UTILDEBUG_VERIFY((con != NULL) &&
                         (con->priv != NULL) &&
                         (addr != NULL) &&
                         (len > 0)) == true)
    {
        mutexobj_lock(&con->priv->mutex);
        backlog = vector_getsize(&con->priv->backlog);
        mutexobj_unlock(&con->priv->mutex);

        if ((backlog > 0) ||
            (con->sock->event.ops.fion_poll(&con->sock->event) == true))
        {
            mutexobj_lock(&con->priv->mutex);

            if ((backlog > 0) || (vector_getsize(&con->priv->backlog) > 0))
            {
                pair = vector_getval(&con->priv->backlog, 0);

                if (pair == NULL)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: failed to get pair\n",
                                  __FUNCTION__);
                }
                else
                {
                    if (*len > sizeof(pair->sockaddr))
                    {
                        *len = sizeof(pair->sockaddr);
                    }

                    memcpy(addr, &pair->sockaddr, *len);
                    ret = pair->fds[SOCKCON_PEER_INDEX];
                    vector_inserttail(&con->priv->frontlog, pair);
                    vector_delete(&con->priv->backlog, 0);
                }
            }

            mutexobj_unlock(&con->priv->mutex);
        }
    }

    return ret;
}
