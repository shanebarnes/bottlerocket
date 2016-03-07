/**
 * @file   socket_tcp.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  TCP socket implementation.
 */

#include "logger.h"
#include "socket_tcp.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_init(struct socket_api * const api)
{
    bool retval = false;

    if (api != NULL)
    {
        api->socket_api_open    = socket_open;
        api->socket_api_close   = socket_close;
        api->socket_api_bind    = socket_bind;
        api->socket_api_listen  = socket_tcp_listen;
        api->socket_api_accept  = socket_tcp_accept;
        api->socket_api_connect = socket_tcp_connect;
        api->socket_api_recv    = socket_tcp_recv;
        api->socket_api_send    = socket_tcp_send;

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

    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t socket_tcp_accept(struct socket_instance * const instance,
                          const int32_t timeoutms)
{
    int32_t        retval     = -1;
    socklen_t      socklen    = 0;
    bool           sockaccept = true;
#if defined(LINUX)
    int32_t        flags      = 0;
#endif
    fd_set         rfds;
    struct timeval tv;

    if (instance != NULL)
    {
        socklen = sizeof(instance->addrpeer.sockaddr);

        if (timeoutms > 0)
        {
#if defined(LINUX)
            flags = O_NONBLOCK;
#endif
            //tv.tv_sec  = dateGetTimeSecIpart((uint64_t)timeoutMs, UNIT_MSEC);
            //tv.tv_usec = dateGetTimeSecFpart((uint64_t)timeoutMs, UNIT_MSEC) * 1000;
            FD_ZERO(&rfds);
            FD_SET(instance->listenfd, &rfds);

            if (select(instance->listenfd + 1, &rfds, NULL, NULL, &tv) <= 0)
            {
                sockaccept = false;
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
                //else if (getsockname(info->fdSocket, (struct sockaddr *)&(info->addrLocal), &sockLen) == 0)
                //{
                //    info->sockAccept = true;
                //    socketGetAddress(info);
                //    retval = true;
                //}
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
