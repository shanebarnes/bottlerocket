/**
 * @file      socket_tcp.c
 * @brief     TCP socket implementation.
 * @author    Shane Barnes
 * @date      01 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "socket_tcp.h"
#include "util_ioctl.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_create(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        instance->ops.sio_create  = socket_tcp_create;
        instance->ops.sio_destroy = socket_tcp_destroy;
        instance->ops.sio_open    = socket_instance_open;
        instance->ops.sio_close   = socket_instance_close;
        instance->ops.sio_bind    = socket_instance_bind;
        instance->ops.sio_listen  = socket_tcp_listen;
        instance->ops.sio_accept  = socket_tcp_accept;
        instance->ops.sio_connect = socket_tcp_connect;
        instance->ops.sio_recv    = socket_tcp_recv;
        instance->ops.sio_send    = socket_tcp_send;

        instance->socktype = SOCK_STREAM;

        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_destroy(struct socket_instance * const instance)
{
    bool retval = false;

    if ((instance == NULL) || (instance->socktype != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        instance->ops.sio_create  = NULL;
        instance->ops.sio_destroy = NULL;
        instance->ops.sio_open    = NULL;
        instance->ops.sio_close   = NULL;
        instance->ops.sio_bind    = NULL;
        instance->ops.sio_listen  = NULL;
        instance->ops.sio_accept  = NULL;
        instance->ops.sio_connect = NULL;
        instance->ops.sio_recv    = NULL;
        instance->ops.sio_send    = NULL;

        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_listen(struct socket_instance * const instance,
                       const int32_t backlog)
{
    bool retval = false;

    if ((instance == NULL) || (instance->socktype != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    // Backlog check: SOMAXCONN
    else if (listen(instance->sockfd, backlog) == 0)
    {
        instance->state |= SOCKET_STATE_LISTEN;
        socket_instance_getaddrself(instance);
        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socked %d failed to listen on %s:%u (%d)\n",
                      __FUNCTION__,
                      instance->sockfd,
                      instance->ipaddr,
                      instance->ipport,
                      errno);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_accept(struct socket_instance * const listener,
                       struct socket_instance * const instance)
{
    bool      retval     = false;
    socklen_t socklen    = 0;
    bool      sockaccept = false;
#if defined(LINUX)
    int32_t   flags      = 0;
#endif

    if ((listener == NULL) ||
        (instance == NULL) ||
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

        if (listener->event.ops.ieo_poll(&listener->event) == true)
        {
            if (((listener->event.revents & IO_EVENT_RET_TIMEOUT) == 0) &&
                ((listener->event.revents & IO_EVENT_RET_ERROR) == 0))
            {
                sockaccept = true;
            }
        }

        if (sockaccept == true)
        {
#if defined(LINUX)
            if ((instance->sockfd = accept4(listener->sockfd,
                                            (struct sockaddr *)&(listener->addrpeer.sockaddr),
                                            &socklen,
                                            flags)) > -1)
#else
            if ((instance->sockfd = accept(listener->sockfd,
                                           (struct sockaddr *)&(listener->addrpeer.sockaddr),
                                           &socklen)) > -1)
#endif
            {
                socklen = sizeof(instance->addrself.sockaddr);

                if ((socket_tcp_create(instance) == false) ||
                    (socket_instance_create(instance)) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d accept initialization failed\n",
                                  __FUNCTION__,
                                  instance->sockfd);
                }
                else if (socket_instance_getaddrself(instance) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d self information is unavailable\n",
                                  instance->sockfd,
                                  __FUNCTION__);
                }
                else if (socket_instance_getaddrpeer(instance) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d peer information is unavailable\n",
                                  __FUNCTION__,
                                  instance->sockfd);
                }
                else
                {
                    logger_printf(LOGGER_LEVEL_TRACE,
                                  "%s: new socket %d accepted on %s from %s\n",
                                  __FUNCTION__,
                                  instance->sockfd,
                                  instance->addrself.sockaddrstr,
                                  instance->addrpeer.sockaddrstr);
                    instance->state = SOCKET_STATE_OPEN | SOCKET_STATE_CONNECT;
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
bool socket_tcp_connect(struct socket_instance * const instance)
{
    bool retval = false;

    if ((instance == NULL) || (instance->socktype != SOCK_STREAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (connect(instance->sockfd,
                    instance->ainfo.ai_addr,
                    instance->ainfo.ai_addrlen) == 0)
        {
            retval = true;
        }
        else
        {
            if (errno == EINPROGRESS)
            {
                instance->event.pevents = IO_EVENT_POLL_IN | IO_EVENT_POLL_OUT;
                instance->event.ops.ieo_setflags(&instance->event);

                // @todo If timeout is -1 (blocking), then the poll should occur
                //       in a loop with a small timeout (e.g., 100 ms) or maybe
                //       a self-pipe for signaling shutdown events, etc.

                if (instance->event.ops.ieo_poll(&instance->event) == true)
                {
                    if (((instance->event.revents & IO_EVENT_RET_ERROR) == 0) &&
                        (instance->event.revents & IO_EVENT_RET_OUTREADY))
                    {
                        retval = true;
                    }
                }

                instance->event.pevents = IO_EVENT_POLL_IN;
                instance->event.ops.ieo_setflags(&instance->event);
            }
            else
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d connect error (%d)\n",
                              __FUNCTION__,
                              instance->sockfd,
                              errno);
            }
        }

        if (retval == true)
        {
            instance->state |= SOCKET_STATE_CONNECT;

            if (socket_instance_getaddrpeer(instance) == false)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d peer information is unavailable\n",
                               __FUNCTION__,
                              instance->sockfd);
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t socket_tcp_recv(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len)
{
    int32_t retval = -1;
    int32_t flags  = MSG_DONTWAIT;

    if ((instance == NULL) || (buf == NULL) || (len == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (instance->event.ops.ieo_poll(&instance->event) == false)
        {
            retval = -1;
        }
        else if ((instance->event.revents & IO_EVENT_RET_ERROR) &&
                 (utilioctl_getbytesavail(instance->sockfd) <= 0))
        {
            retval = -1;
        }
        else if (instance->event.revents & IO_EVENT_RET_INREADY)
        {
            retval = recv(instance->sockfd, buf, len, flags);

            if (retval > 0)
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %d received %d bytes\n",
                              __FUNCTION__,
                              instance->sockfd,
                              retval);
            }
            else
            {
                switch (errno)
                {
                    // Fatal errors.
                    case EBADF:
                    case ECONNRESET:
                    case EPIPE:
                    case ENOTSOCK:
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %d fatal error (%d)\n",
                                      __FUNCTION__,
                                      instance->sockfd,
                                      errno);
                        retval = -1;
                        break;
                    // Non-fatal errors.
                    case EAGAIN:
                    case EFAULT:
                    case EINTR:
                    case EINVAL:
                    case ENOBUFS:
                    case ENOTCONN:
                    case EOPNOTSUPP:
                    case ETIMEDOUT:
                    default:
                        logger_printf(LOGGER_LEVEL_TRACE,
                                      "%s: socket %d non-fatal error (%d)\n",
                                      __FUNCTION__,
                                      instance->sockfd,
                                      errno);
                        retval = 0;
                }
            }
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
int32_t socket_tcp_send(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len)
{
    int32_t retval = -1;
    int32_t flags  = MSG_DONTWAIT;

#if defined(LINUX)
    flags |= MSG_NOSIGNAL;
#endif

    if ((instance == NULL) || (buf == NULL) || (len == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = send(instance->sockfd, buf, len, flags);

        if (retval > 0)
        {
            logger_printf(LOGGER_LEVEL_TRACE,
                          "%s: socket %d sent %d bytes\n",
                          __FUNCTION__,
                          instance->sockfd,
                          retval);
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
                                  instance->sockfd,
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
                                  instance->sockfd,
                                  errno);
                    retval = 0;
            }

            if (retval == 0)
            {
                if (instance->event.ops.ieo_poll(&instance->event) == false)
                {
                    retval = -1;
                }
                else if (instance->event.revents & IO_EVENT_RET_ERROR)
                {
                    retval = -1;
                }
            }
        }
    }

    return retval;
}
