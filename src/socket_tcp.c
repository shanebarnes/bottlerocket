/**
 * @file   socket_tcp.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  TCP socket implementation.
 */

#include "socket_tcp.h"

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

    if ((buf !=  NULL) && (len > 0))
    {
        if (sockfd)
        {

        }
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

    if ((buf !=  NULL) && (len > 0))
    {
        if (sockfd)
        {

        }
    }

    return retval;
}