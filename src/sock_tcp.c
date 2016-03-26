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

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

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
        obj->ops.soo_create  = socktcp_create;
        obj->ops.soo_destroy = socktcp_destroy;
        obj->ops.soo_open    = sockobj_open;
        obj->ops.soo_close   = sockobj_close;
        obj->ops.soo_bind    = sockobj_bind;
        obj->ops.soo_listen  = socktcp_listen;
        obj->ops.soo_accept  = socktcp_accept;
        obj->ops.soo_connect = socktcp_connect;
        obj->ops.soo_recv    = socktcp_recv;
        obj->ops.soo_send    = socktcp_send;

        obj->socktype = SOCK_STREAM;

        retval = true;
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
        obj->ops.soo_create  = NULL;
        obj->ops.soo_destroy = NULL;
        obj->ops.soo_open    = NULL;
        obj->ops.soo_close   = NULL;
        obj->ops.soo_bind    = NULL;
        obj->ops.soo_listen  = NULL;
        obj->ops.soo_accept  = NULL;
        obj->ops.soo_connect = NULL;
        obj->ops.soo_recv    = NULL;
        obj->ops.soo_send    = NULL;

        retval = true;
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
#if defined(LINUX)
    int32_t   flags      = 0;
#endif

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
            if ((obj->sockfd = accept4(listener->sockfd,
                                       (struct sockaddr *)&(listener->addrpeer.sockaddr),
                                       &socklen,
                                       flags)) > -1)
#else
            if ((obj->sockfd = accept(listener->sockfd,
                                      (struct sockaddr *)&(listener->addrpeer.sockaddr),
                                      &socklen)) > -1)
#endif
            {
                socklen = sizeof(obj->addrself.sockaddr);

                if ((socktcp_create(obj) == false) ||
                    (sockobj_create(obj)) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d accept initialization failed\n",
                                  __FUNCTION__,
                                  obj->sockfd);
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
                obj->event.pevents = IO_EVENT_POLL_IN | IO_EVENT_POLL_OUT;
                obj->event.ops.ieo_setflags(&obj->event);

                // @todo If timeout is -1 (blocking), then the poll should occur
                //       in a loop with a small timeout (e.g., 100 ms) or maybe
                //       a self-pipe for signaling shutdown events, etc.

                if (obj->event.ops.ieo_poll(&obj->event) == true)
                {
                    if (((obj->event.revents & IO_EVENT_RET_ERROR) == 0) &&
                        (obj->event.revents & IO_EVENT_RET_OUTREADY))
                    {
                        retval = true;
                    }
                }

                obj->event.pevents = IO_EVENT_POLL_IN;
                obj->event.ops.ieo_setflags(&obj->event);
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
        if (obj->event.ops.ieo_poll(&obj->event) == false)
        {
            retval = -1;
        }
        else if (obj->event.revents & IO_EVENT_RET_INREADY)
        {
            retval = recv(obj->sockfd, buf, len, flags);

            if (retval > 0)
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %d received %d bytes\n",
                              __FUNCTION__,
                              obj->sockfd,
                              retval);
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
        else if (obj->event.revents & IO_EVENT_RET_ERROR)
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
                if (obj->event.ops.ieo_poll(&obj->event) == false)
                {
                    retval = -1;
                }
                else if (obj->event.revents & IO_EVENT_RET_ERROR)
                {
                    retval = -1;
                }
            }
        }
    }

    return retval;
}
