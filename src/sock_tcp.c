/**
 * @file      sock_tcp.c
 * @brief     TCP socket implementation.
 * @author    Shane Barnes
 * @date      01 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "sock_tcp.h"
#include "util_date.h"
#include "util_unit.h"

#include <errno.h>
#if defined(__linux__)
    #include <fcntl.h>
#endif
#include <netinet/tcp.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * @see See header file for interface comments.
 */
bool socktcp_getinfo(const int32_t fd, struct socktcp_info * const info)
{
    bool ret = false;
    int32_t err = -1;
#if defined (__APPLE__) && defined(TCP_CONNECTION_INFO)
    struct tcp_connection_info optval;
    int32_t optname = TCP_CONNECTION_INFO;
#elif defined (__linux__)
    struct tcp_info optval;
    int32_t optname = TCP_INFO;
#else
    uint32_t optval;
    uint32_t optname = 0;
#endif
    socklen_t optlen = sizeof(optval);

    if ((fd < 0) || (info == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        err = getsockopt(fd,
                         IPPROTO_TCP,
                         optname,
                         &optval,
                         (socklen_t *)&optlen);

        if (err != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to get TCP info (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
#if (defined (__APPLE__) && defined(TCP_CONNECTION_INFO)) || defined (__linux__)
            info->state     = optval.tcpi_state;
            info->sndwscale = optval.tcpi_snd_wscale;
            info->rcvwscale = optval.tcpi_rcv_wscale;
            info->options   = optval.tcpi_options;
            info->rto       = optval.tcpi_rto;
    #if defined (__APPLE__) && defined(TCP_CONNECTION_INFO)
            info->flags     = optval.tcpi_flags;
            info->mss       = optval.tcpi_maxseg;
            info->sndbuf    = optval.tcpi_snd_sbbytes;
            info->rcvwin    = optval.tcpi_rcv_wnd;
            info->rttcur    = optval.tcpi_rttcur;
            info->rttavg    = optval.tcpi_srtt;
            info->rttvar    = optval.tcpi_rttvar;
            info->txpackets = optval.tcpi_txpackets;
            info->txbytes   = optval.tcpi_txbytes;
            info->retxbytes = optval.tcpi_txretransmitbytes;
            info->rxpackets = optval.tcpi_rxpackets;
            info->rxbytes   = optval.tcpi_rxbytes;
            info->rxoobytes = optval.tcpi_rxoutoforderbytes;
    #else
            info->mss       = optval.tcpi_snd_mss;
    #endif
            info->ssthresh  = optval.tcpi_snd_ssthresh;
            info->cwnd      = optval.tcpi_snd_cwnd;

            ret = true;
#endif
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_create(struct sockobj * const obj)
{
    bool ret = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (sockobj_create(obj) == true)
        {
            obj->ops.sock_create   = socktcp_create;
            obj->ops.sock_destroy  = socktcp_destroy;
            obj->ops.sock_open     = sockobj_open;
            obj->ops.sock_close    = sockobj_close;
            obj->ops.sock_bind     = sockobj_bind;
            obj->ops.sock_getopts  = sockobj_getopts;
            obj->ops.sock_setopts  = sockobj_setopts;
            obj->ops.sock_listen   = socktcp_listen;
            obj->ops.sock_accept   = socktcp_accept;
            obj->ops.sock_connect  = socktcp_connect;
            obj->ops.sock_recv     = socktcp_recv;
            obj->ops.sock_send     = socktcp_send;
            obj->ops.sock_shutdown = socktcp_shutdown;

            obj->conf.type = SOCK_STREAM;

            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_destroy(struct sockobj * const obj)
{
    bool ret = false;

    if ((obj == NULL) || (obj->conf.type != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        ret = obj->ops.sock_destroy(obj);
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_listen(struct sockobj * const obj, const int32_t backlog)
{
    bool ret = false;

    if ((obj == NULL) || (obj->conf.type != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    // Backlog check: SOMAXCONN
    else if (listen(obj->fd, backlog) == 0)
    {
        obj->state |= SOCKOBJ_STATE_LISTEN;
        sockobj_getaddrself(obj);
        ret = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socked %d failed to listen on %s:%u (%d)\n",
                      __FUNCTION__,
                      obj->fd,
                      obj->conf.ipaddr,
                      obj->conf.ipport,
                      errno);
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_accept(struct sockobj * const listener, struct sockobj * const obj)
{
    bool      ret        = false;
    socklen_t socklen    = 0;
    bool      sockaccept = false;
    int32_t   fd         = -1;
#if defined(__linux__)
    int32_t   flags      = 0;
#endif
    uint64_t  ts         = 0;

    if ((listener == NULL) ||
        (obj == NULL) ||
        (listener->conf.type != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        socklen = sizeof(listener->addrpeer.sockaddr);
#if defined(__linux__)
        flags = (listener->event.timeoutms > -1 ? O_NONBLOCK : 0);
#endif
        // @todo If timeout is -1 (blocking), then the poll should occur in a
        //       loop with a small timeout (e.g., 100 ms) or maybe a self-pipe
        //       for signaling shutdown events, etc.

        if (listener->event.ops.fion_poll(&listener->event) == true)
        {
            if (((listener->event.revents & FIONOBJ_REVENT_TIMEOUT) == 0) &&
                ((listener->event.revents & FIONOBJ_REVENT_ERROR) == 0))
            {
                sockaccept = true;
            }
        }

        if (sockaccept == true)
        {
#if defined(__linux__)
            if ((fd = accept4(listener->fd,
                              (struct sockaddr *)&(listener->addrpeer.sockaddr),
                              &socklen,
                              flags)) > -1)
#else
            if ((fd = accept(listener->fd,
                             (struct sockaddr *)&(listener->addrpeer.sockaddr),
                             &socklen)) > -1)
#endif
            {
                ts = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
                socklen = sizeof(obj->addrself.sockaddr);

                if (socktcp_create(obj) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %u accept initialization failed\n",
                                  __FUNCTION__,
                                  obj->id);
                }
                else if (((obj->fd = fd) != fd) ||
                         (obj->event.ops.fion_insertfd(&obj->event, fd) == false) ||
                         (obj->event.ops.fion_setflags(&obj->event) == false))
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %u fd clone failed\n",
                                  __FUNCTION__,
                                  obj->id);
                }
                else if (memcpy(&obj->conf,
                                &listener->conf,
                                sizeof(listener->conf)) == NULL)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %u configuration clone failed\n",
                                  __FUNCTION__,
                                  obj->id);
                }
                else if (sockobj_getaddrself(obj) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %u self information is unavailable\n",
                                  obj->id,
                                  __FUNCTION__);
                }
                else if (sockobj_getaddrpeer(obj) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %u peer information is unavailable\n",
                                  __FUNCTION__,
                                  obj->id);
                }
                else
                {
                    logger_printf(LOGGER_LEVEL_TRACE,
                                  "%s: new socket %u accepted on %s from %s\n",
                                  __FUNCTION__,
                                  obj->id,
                                  obj->addrself.sockaddrstr,
                                  obj->addrpeer.sockaddrstr);

                    tokenbucket_init(&obj->tb, obj->conf.ratelimitbps);
                    obj->state = SOCKOBJ_STATE_OPEN | SOCKOBJ_STATE_CONNECT;
                    obj->info.startusec = ts;
                    ret = true;
                }
            }
            else
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u accept failed (%d)\n",
                              __FUNCTION__,
                              listener->id,
                              errno);
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_connect(struct sockobj * const obj)
{
    bool ret = false;

    if ((obj == NULL) || (obj->conf.type != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->info.startusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                 UNIT_TIME_USEC);

        if (obj->state == SOCKOBJ_STATE_OPEN && connect(obj->fd,
                    obj->ainfo.ai_addr,
                    obj->ainfo.ai_addrlen) == 0)
        {
            ret = true;
        }
        else
        {
            if (errno == EINPROGRESS)
            {
                logger_printf(LOGGER_LEVEL_DEBUG,
                              "%s: socket %u connect now in progress\n",
                              __FUNCTION__,
                              obj->id);

                obj->event.pevents = FIONOBJ_PEVENT_IN | FIONOBJ_PEVENT_OUT;
                obj->event.ops.fion_setflags(&obj->event);

                // @todo If timeout is -1 (blocking), then the poll should occur
                //       in a loop with a small timeout (e.g., 100 ms) or maybe
                //       a self-pipe for signaling shutdown events, etc.

                if (obj->event.ops.fion_poll(&obj->event) == true)
                {
                    if (((obj->event.revents & FIONOBJ_REVENT_ERROR) == 0) &&
                        (obj->event.revents & FIONOBJ_REVENT_OUTREADY))
                    {
                        ret = true;
                    }
                }

                obj->event.pevents = FIONOBJ_PEVENT_IN;
                obj->event.ops.fion_setflags(&obj->event);
            }
            else if (errno == EINVAL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u connect fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);

                obj->ops.sock_close(obj);
            }
            else if (errno == EISCONN)
            {
                ret = true;
            }
            else if (errno == EALREADY)
            {
                logger_printf(LOGGER_LEVEL_DEBUG,
                              "%s: socket %u connect already in progress\n",
                              __FUNCTION__,
                              obj->id);
            }
            else
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u connect error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);
            }
        }

        if (ret == true)
        {
            obj->state |= SOCKOBJ_STATE_CONNECT;

            sockobj_getaddrself(obj);
            sockobj_getaddrpeer(obj);
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
int32_t socktcp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t ret   = -1;
    int32_t flags = MSG_DONTWAIT;

    if ((obj == NULL) || (buf == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        ret = recv(obj->fd, buf, len, flags);
        sockobj_setstats(&obj->info.recv, ret);

        if (ret > 0)
        {
            logger_printf(LOGGER_LEVEL_TRACE,
                          "%s: socket %u received %d bytes\n",
                          __FUNCTION__,
                          obj->id,
                          ret);
        }
        else
        {
            if (sockobj_iserrfatal(errno) == true)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);
                ret = -1;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %u non-fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);
                ret = 0;
            }

            if (ret == 0)
            {
                if (obj->event.ops.fion_poll(&obj->event) == false)
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_INREADY)
                {
                    ret = recv(obj->fd, buf, len, flags);
                    sockobj_setstats(&obj->info.recv, ret);

                    // Remote peer is closed if input is ready but no bytes are
                    // received (EOF).
                    if (ret > 0)
                    {
                        logger_printf(LOGGER_LEVEL_TRACE,
                                      "%s: socket %u received %d bytes\n",
                                      __FUNCTION__,
                                      obj->id,
                                      ret);
                    }
                    else
                    {
                        ret = -1;
                    }
                }
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
int32_t socktcp_send(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t ret   = -1;
    int32_t flags = MSG_DONTWAIT;

#if defined(__linux__)
    flags |= MSG_NOSIGNAL;
#endif

    if ((obj == NULL) || (buf == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        ret = send(obj->fd, buf, len, flags);
        sockobj_setstats(&obj->info.send, ret);

        if (ret > 0)
        {
            logger_printf(LOGGER_LEVEL_TRACE,
                          "%s: socket %u sent %d bytes\n",
                          __FUNCTION__,
                          obj->id,
                          ret);
        }
        else
        {
            if (sockobj_iserrfatal(errno) == true)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);
                ret = -1;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %u non-fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);
                ret = 0;
            }

            if (ret == 0)
            {
                if (obj->event.ops.fion_poll(&obj->event) == false)
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
                {
                    ret = -1;
                }
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_shutdown(struct sockobj * const obj, const int32_t how)
{
    bool ret = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (shutdown(obj->fd, how) == -1)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to shutdown socket %u (%d)\n",
                          __FUNCTION__,
                          obj->id,
                          errno);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}
