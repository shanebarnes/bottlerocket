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
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * @see See header file for interface comments.
 */
bool socktcp_getinfo(const int32_t fd, struct socktcp_info * const info)
{
    bool retval = false;
    int32_t errval = -1;
#if defined (__APPLE__)
    struct tcp_connection_info optval;
    int32_t optname = TCP_CONNECTION_INFO;
#elif defined (LINUX)
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
        errval = getsockopt(fd,
                            IPPROTO_TCP,
                            optname,
                            &optval,
                            (socklen_t *)&optlen);

        if (errval != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to get TCP info (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
#if defined (__APPLE__) || defined (LINUX)
            info->state     = optval.tcpi_state;
            info->sndwscale = optval.tcpi_snd_wscale;
            info->rcvwscale = optval.tcpi_rcv_wscale;
            info->options   = optval.tcpi_options;
            info->flags     = optval.tcpi_flags;
            info->rto       = optval.tcpi_rto;
    #if defined (__APPLE__)
            info->mss       = optval.tcpi_maxseg;
    #else
            info->mss       = optval.tcpi_snd_mss;
    #endif
            info->ssthresh  = optval.tcpi_snd_ssthresh;
            info->cwnd      = optval.tcpi_snd_cwnd;
    #if defined (__APPLE__)
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
    #endif
            retval = true;
#endif
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_create(struct sockobj * const obj)
{
    bool retval = false;

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
            obj->ops.sock_create  = socktcp_create;
            obj->ops.sock_destroy = socktcp_destroy;
            obj->ops.sock_open    = sockobj_open;
            obj->ops.sock_close   = sockobj_close;
            obj->ops.sock_bind    = sockobj_bind;
            obj->ops.sock_listen  = socktcp_listen;
            obj->ops.sock_accept  = socktcp_accept;
            obj->ops.sock_connect = socktcp_connect;
            obj->ops.sock_recv    = socktcp_recv;
            obj->ops.sock_send    = socktcp_send;

            obj->socktype = SOCK_STREAM;

            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_destroy(struct sockobj * const obj)
{
    bool retval = false;

    if ((obj == NULL) || (obj->socktype != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = sockobj_destroy(obj);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_listen(struct sockobj * const obj, const int32_t backlog)
{
    bool retval = false;

    if ((obj == NULL) || (obj->socktype != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    // Backlog check: SOMAXCONN
    else if (listen(obj->sockfd, backlog) == 0)
    {
        obj->state |= SOCKOBJ_STATE_LISTEN;
        sockobj_getaddrself(obj);
        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socked %d failed to listen on %s:%u (%d)\n",
                      __FUNCTION__,
                      obj->sockfd,
                      obj->ipaddr,
                      obj->ipport,
                      errno);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_accept(struct sockobj * const listener, struct sockobj * const obj)
{
    bool      retval     = false;
    socklen_t socklen    = 0;
    bool      sockaccept = false;
    int32_t   sockfd     = -1;
#if defined(LINUX)
    int32_t   flags      = 0;
#endif
    uint64_t  ts         = 0;

    if ((listener == NULL) ||
        (obj == NULL) ||
        (listener->socktype != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        socklen = sizeof(listener->addrpeer.sockaddr);
#if defined(LINUX)
        flags = (listener->event.timeoutms > -1 ? O_NONBLOCK : 0);
#endif
        // @todo If timeout is -1 (blocking), then the poll should occur in a
        //       loop with a small timeout (e.g., 100 ms) or maybe a self-pipe
        //       for signaling shutdown events, etc.

        if (listener->event.ops.foo_poll(&listener->event) == true)
        {
            if (((listener->event.revents & FIONOBJ_REVENT_TIMEOUT) == 0) &&
                ((listener->event.revents & FIONOBJ_REVENT_ERROR) == 0))
            {
                sockaccept = true;
            }
        }

        if (sockaccept == true)
        {
#if defined(LINUX)
            if ((sockfd = accept4(listener->sockfd,
                                  (struct sockaddr *)&(listener->addrpeer.sockaddr),
                                  &socklen,
                                  flags)) > -1)
#else
            if ((sockfd = accept(listener->sockfd,
                                 (struct sockaddr *)&(listener->addrpeer.sockaddr),
                                 &socklen)) > -1)
#endif
            {
                ts = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
                socklen = sizeof(obj->addrself.sockaddr);

                if (socktcp_create(obj) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d accept initialization failed\n",
                                  __FUNCTION__,
                                  obj->sockfd);
                }
                else if (((obj->sockfd = sockfd) != sockfd) ||
                         (obj->event.ops.foo_insertfd(&obj->event, sockfd) == false) ||
                         (obj->event.ops.foo_setflags(&obj->event) == false))
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: invalid socket fd (%d)\n",
                                  __FUNCTION__,
                                  sockfd);
                }
                else if (sockobj_getaddrself(obj) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d self information is unavailable\n",
                                  obj->sockfd,
                                  __FUNCTION__);
                }
                else if (sockobj_getaddrpeer(obj) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d peer information is unavailable\n",
                                  __FUNCTION__,
                                  obj->sockfd);
                }
                else
                {
                    logger_printf(LOGGER_LEVEL_TRACE,
                                  "%s: new socket %d accepted on %s from %s\n",
                                  __FUNCTION__,
                                  obj->sockfd,
                                  obj->addrself.sockaddrstr,
                                  obj->addrpeer.sockaddrstr);
                    obj->state = SOCKOBJ_STATE_OPEN | SOCKOBJ_STATE_CONNECT;
                    obj->info.startusec = ts;
                    retval = true;
                }
            }
            else
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d accept failed (%d)\n",
                              __FUNCTION__,
                              listener->sockfd,
                              errno);
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socktcp_connect(struct sockobj * const obj)
{
    bool retval = false;

    if ((obj == NULL) || (obj->socktype != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->info.startusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                 UNIT_TIME_USEC);

        if (connect(obj->sockfd,
                    obj->ainfo.ai_addr,
                    obj->ainfo.ai_addrlen) == 0)
        {
            retval = true;
        }
        else
        {
            if (errno == EINPROGRESS)
            {
                obj->event.pevents = FIONOBJ_PEVENT_IN | FIONOBJ_PEVENT_OUT;
                obj->event.ops.foo_setflags(&obj->event);

                // @todo If timeout is -1 (blocking), then the poll should occur
                //       in a loop with a small timeout (e.g., 100 ms) or maybe
                //       a self-pipe for signaling shutdown events, etc.

                if (obj->event.ops.foo_poll(&obj->event) == true)
                {
                    if (((obj->event.revents & FIONOBJ_REVENT_ERROR) == 0) &&
                        (obj->event.revents & FIONOBJ_REVENT_OUTREADY))
                    {
                        retval = true;
                    }
                }

                obj->event.pevents = FIONOBJ_PEVENT_IN;
                obj->event.ops.foo_setflags(&obj->event);
            }
            else
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d connect error (%d)\n",
                              __FUNCTION__,
                              obj->sockfd,
                              errno);
            }
        }

        if (retval == true)
        {
            obj->state |= SOCKOBJ_STATE_CONNECT;

            if (sockobj_getaddrpeer(obj) == false)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d peer information is unavailable\n",
                              __FUNCTION__,
                              obj->sockfd);
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t socktcp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t retval = -1;
    int32_t flags  = MSG_DONTWAIT;

    if ((obj == NULL) || (buf == NULL) || (len == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (obj->event.ops.foo_poll(&obj->event) == false)
        {
            retval = -1;
        }
        else if (obj->event.revents & FIONOBJ_REVENT_INREADY)
        {
            retval = recv(obj->sockfd, buf, len, flags);

            if (retval > 0)
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %d received %d bytes\n",
                              __FUNCTION__,
                              obj->sockfd,
                              retval);
                obj->info.recvbytes += retval;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %d fatal error (%d)\n",
                              __FUNCTION__,
                              obj->sockfd,
                              errno);
                retval = -1;
            }
        }
        else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
        {
            retval = -1;
        }
        else
        {
            retval = 0;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t socktcp_send(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t retval = -1;
    int32_t flags  = MSG_DONTWAIT;

#if defined(LINUX)
    flags |= MSG_NOSIGNAL;
#endif

    if ((obj == NULL) || (buf == NULL) || (len == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = send(obj->sockfd, buf, len, flags);

        if (retval > 0)
        {
            logger_printf(LOGGER_LEVEL_TRACE,
                          "%s: socket %d sent %d bytes\n",
                          __FUNCTION__,
                          obj->sockfd,
                          retval);
            obj->info.sendbytes += retval;
        }
        // Check for socket errors if send failed.
        else
        {
            switch (errno)
            {
                // Fatal errors.
                case EBADF:
                case ECONNRESET:
                case EHOSTUNREACH:
                case EPIPE:
                case ENOTSOCK:
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d fatal error (%d)\n",
                                  __FUNCTION__,
                                  obj->sockfd,
                                  errno);
                    retval = -1;
                    break;
                // Non-fatal errors.
                case EACCES:
                case EAGAIN:
                case EFAULT:
                case EINTR:
                case EMSGSIZE:
                case ENETDOWN:
                case ENETUNREACH:
                case ENOBUFS:
                case EOPNOTSUPP:
                default:
                    logger_printf(LOGGER_LEVEL_TRACE,
                                  "%s: socket %d non-fatal error (%d)\n",
                                  __FUNCTION__,
                                  obj->sockfd,
                                  errno);
                    retval = 0;
            }

            if (retval == 0)
            {
                if (obj->event.ops.foo_poll(&obj->event) == false)
                {
                    retval = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
                {
                    retval = -1;
                }
            }
        }
    }

    return retval;
}
