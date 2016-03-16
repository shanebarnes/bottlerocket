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
        instance->sockapi.socket_api_open    = socket_instance_open;
        instance->sockapi.socket_api_close   = socket_instance_close;
        instance->sockapi.socket_api_bind    = socket_instance_bind;
        instance->sockapi.socket_api_listen  = socket_tcp_listen;
        instance->sockapi.socket_api_accept  = socket_tcp_accept;
        instance->sockapi.socket_api_connect = socket_tcp_connect;
        instance->sockapi.socket_api_recv    = socket_tcp_recv;
        instance->sockapi.socket_api_send    = socket_tcp_send;

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
        if (listen(instance->listenfd, backlog) == 0)
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
int32_t socket_tcp_accept(struct socket_instance * const instance,
                          const int32_t timeoutms)
{
    int32_t                  retval     = -1;
    socklen_t                socklen    = 0;
    bool                     sockaccept = (timeoutms == 0 ? true : false);
#if defined(LINUX)
    int32_t                  flags      = 0;
#endif
    struct io_event_instance poll;

    if (instance != NULL)
    {
        socklen = sizeof(instance->addrpeer.sockaddr);

        if (timeoutms > 0)
        {
#if defined(LINUX)
            flags = O_NONBLOCK;
#endif
            poll.fds       = &instance->listenfd;
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
        }

        if (sockaccept == true)
        {
#if defined(LINUX)
            if ((instance->sockfd = accept4(instance->listenfd,
                                            (struct sockaddr *)&(instance->addrpeer.sockaddr),
                                            &socklen,
                                            flags)) > -1)
#else
            if ((instance->sockfd = accept(instance->listenfd,
                                           (struct sockaddr *)&(instance->addrpeer.sockaddr),
                                           &socklen)) > -1)
#endif
            {
                socklen = sizeof(instance->addrself.sockaddr);

                //if (info->eventApi.socketIoEventApiInitialize(&(info->fdEvent), info->fdSocket) != true)
                //{
                //    // Nothing to do
                //}
                if (false)
                {

                }
                else if (getsockname(instance->sockfd,
                                     (struct sockaddr *)&(instance->addrself.sockaddr),
                                     &socklen) == 0)
                {
                    //info->sockAccept = true;
                    socket_instance_address(instance, false);
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
            if (errno == EPIPE)
            {
                retval = -1;
            }
            else
            {
                // Socket receive buffer is full. Poll until timeout is reached
                // or receive buffer is at least partially drained.
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
            if (errno == EPIPE)
            {
                retval = -1;
            }
            else
            {
                // Socket send buffer is full. Poll until timeout is reached or
                // send buffer space is at least partially drained.
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
