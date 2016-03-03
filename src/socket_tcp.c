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
#include <sys/socket.h>

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_init(socket_api_t * const api)
{
    bool retval = false;

    if (api != NULL)
    {
        api->socket_api_open    = NULL;//socket_open;
        api->socket_api_close   = NULL;//socket_close;
        api->socket_api_bind    = NULL;//socket_bind;
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
bool socket_tcp_listen(const int32_t sockfd, const int32_t backlog)
{
    bool retval = false;

    if ((sockfd) && (backlog))
    {

    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t socket_tcp_accept(const int32_t sockfd, const int32_t timeoutms)
{
    int32_t retval = -1;

    if ((sockfd) && (timeoutms))
    {

    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_tcp_connect(const int32_t sockfd, const int32_t timeoutms)
{
    bool retval = false;

    if ((sockfd) && (timeoutms))
    {

    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t socket_tcp_recv(const int32_t sockfd,
                        void * const buf,
                        const uint32_t len)
{
    int32_t retval = -1;
    int32_t flags  = MSG_DONTWAIT;

    if ((buf != NULL) && (len > 0))
    {
        retval = recv(sockfd, buf, len, flags);

        logger_printf(LOGGER_LEVEL_TRACE,
                      "%s: %d bytes received on socket %d\n",
                      __FUNCTION__,
                      retval,
                      sockfd);

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
int32_t socket_tcp_send(const int32_t sockfd,
                        void * const buf,
                        const uint32_t len)
{
    int32_t retval = -1;
    int32_t flags  = MSG_DONTWAIT;

#if defined(LINUX)
    flags |= MSG_NOSIGNAL;
#endif

    if ((buf != NULL) && (len > 0))
    {
        retval = send(sockfd, buf, len, flags);

        logger_printf(LOGGER_LEVEL_TRACE,
                      "%s: %d bytes sent on socket %d\n",
                      __FUNCTION__,
                      retval,
                      sockfd);

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
