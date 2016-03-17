/**
 * @file   socket_tcp.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  TCP socket implementation.
 */

#include "io_event_poll.h"
#include "logger.h"
#include "socket_tcp.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_init(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        instance->sockapi.open    = socket_instance_open;
        instance->sockapi.close   = socket_instance_close;
        instance->sockapi.bind    = socket_instance_bind;
        instance->sockapi.listen  = socket_tcp_listen;
        instance->sockapi.accept  = socket_tcp_accept;
        instance->sockapi.connect = socket_tcp_connect;
        instance->sockapi.recv    = socket_tcp_recv;
        instance->sockapi.send    = socket_tcp_send;

        instance->socktype = SOCK_STREAM;

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

    if ((instance != NULL) && (backlog))
    {
        // Backlog check: SOMAXCONN
        if (listen(instance->sockfd, backlog) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to listen on %s:%u (%d)\n",
                          __FUNCTION__,
                          instance->ipaddr,
                          instance->ipport,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_accept(struct socket_instance * const listener,
                       struct socket_instance * const instance,
                       const int32_t timeoutms)
{
    bool                     retval     = false;
    socklen_t                socklen    = 0;
    bool                     sockaccept = (timeoutms == 0 ? true : false);
#if defined(LINUX)
    int32_t                  flags      = 0;
#endif
    struct io_event_instance poll;

    if ((listener != NULL) && (instance != NULL))
    {
        socklen = sizeof(listener->addrpeer.sockaddr);

        if (timeoutms > 0)
        {
#if defined(LINUX)
            flags = O_NONBLOCK;
#endif
            poll.fds       = &listener->sockfd;
            poll.size      = 1;
            poll.timeoutms = timeoutms;
            poll.pevents   = IO_EVENT_POLL_IN;
            poll.internal  = NULL;

            if ((io_event_poll_create(&poll) == true) &&
                (io_event_poll_poll(&poll) == true))
            {
                if (((poll.revents & IO_EVENT_RET_TIMEOUT) == 0) &&
                    ((poll.revents & IO_EVENT_RET_ERROR) == 0))
                {
                    sockaccept = true;
                }
            }

            io_event_poll_destroy(&poll);
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

                //if (info->eventApi.socketIoEventApiInitialize(&(info->fdEvent), info->fdSocket) != true)
                //{
                //    // Nothing to do
                //}
                if (socket_tcp_init(instance) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: accepted socket initialization failed\n",
                                  __FUNCTION__);
                }
                else if (socket_instance_getaddrself(instance) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: self socket information is unavailable\n",
                                  __FUNCTION__);
                }
                else if (socket_instance_getaddrpeer(instance) == false)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: peer socket information is unavailable\n",
                                  __FUNCTION__);
                }
                else
                {
                    logger_printf(LOGGER_LEVEL_TRACE,
                                  "%s: accepted connection on %s from %s\n",
                                  __FUNCTION__,
                                  instance->addrself.sockaddrstr,
                                  instance->addrpeer.sockaddrstr);
                    retval = true;
                }
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_connect(struct socket_instance * const instance,
                        const int32_t timeoutms)
{
    bool retval = false;

    if ((instance != NULL) && (timeoutms))
    {

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

    if ((instance != NULL) && (buf != NULL) && (len > 0))
    {
        retval = recv(instance->sockfd, buf, len, flags);

        logger_printf(LOGGER_LEVEL_TRACE,
                      "%s: %d bytes received on socket %d\n",
                      __FUNCTION__,
                      retval,
                      instance->sockfd);

        // Check for socket errors if receive failed.
        if (retval <= 0)
        {
            switch (errno)
            {
                // Fatal errors.
                case EBADF:
                case ECONNRESET:
                case EPIPE:
                case ENOTSOCK:
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: fatal error (%d)\n",
                                  __FUNCTION__,
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
                    logger_printf(LOGGER_LEVEL_DEBUG,
                                  "%s: non-fatal error (%d)\n",
                                  __FUNCTION__,
                                  errno);
                    retval = 0;
            }
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: buffer is empty\n",
                      __FUNCTION__);
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

    if ((instance != NULL) && (buf != NULL) && (len > 0))
    {
        retval = send(instance->sockfd, buf, len, flags);

        logger_printf(LOGGER_LEVEL_TRACE,
                      "%s: %d bytes sent on socket %d\n",
                      __FUNCTION__,
                      retval,
                      instance->sockfd);

        // Check for socket errors if send failed.
        if (retval <= 0)
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
                                  "%s: fatal error (%d)\n",
                                  __FUNCTION__,
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
                    logger_printf(LOGGER_LEVEL_DEBUG,
                                  "%s: non-fatal error (%d)\n",
                                  __FUNCTION__,
                                  errno);
                    retval = 0;
            }
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: buffer is empty\n",
                      __FUNCTION__);
    }

    return retval;
}
