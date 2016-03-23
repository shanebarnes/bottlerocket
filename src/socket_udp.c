/**
 * @file   socket_udp.c
 * @author Shane Barnes
 * @date   21 Mar 2016
 * @brief  UDP socket implementation.
 */

#include "logger.h"
#include "socket_udp.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

// temp
#include <unistd.h>
#include <arpa/inet.h>
// temp

/**
 * @see See header file for interface comments.
 */
bool socket_udp_create(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        instance->ops.sio_create  = socket_udp_create;
        instance->ops.sio_destroy = socket_udp_destroy;
        instance->ops.sio_open    = socket_instance_open;
        instance->ops.sio_close   = socket_instance_close;
        instance->ops.sio_bind    = socket_instance_bind;
        instance->ops.sio_listen  = socket_udp_listen;
        instance->ops.sio_accept  = socket_udp_accept;
        instance->ops.sio_connect = socket_udp_connect;
        instance->ops.sio_recv    = socket_udp_recv;
        instance->ops.sio_send    = socket_udp_send;

        instance->socktype = SOCK_DGRAM;

        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_udp_destroy(struct socket_instance * const instance)
{
    bool retval = false;

    if ((instance != NULL) && (instance->socktype == SOCK_DGRAM))
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
bool socket_udp_listen(struct socket_instance * const instance,
                       const int32_t backlog)
{
    bool retval = false;

    // @todo Consider using this API to create the size of the UDP "connection"
    //       poll.

    if ((instance != NULL) && (backlog > 0))
    {
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_udp_accept(struct socket_instance * const listener,
                       struct socket_instance * const instance)
{
    bool retval = false;

    // @todo Consider using this API to poll for datagrams available to be read.
    //       Once read, the datagrams could be filtered and associated with the
    //       appropriate socket instance in the "connection" backlog.

    if ((listener != NULL) && (instance != NULL))
    {
        // @todo If timeout is -1 (blocking), then the poll should occur in a
        //       loop with a small timeout (e.g., 100 ms) or maybe a self-pipe
        //       for signaling shutdown events, etc.

        if (listener->event.ops.ieo_poll(&listener->event) == true)
        {
            if (((listener->event.revents & IO_EVENT_RET_TIMEOUT) == 0) &&
                ((listener->event.revents & IO_EVENT_RET_ERROR) == 0))
            {
                retval = true;
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_udp_connect(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        // There are a number of benefits to calling connect() on a UDP socket:
        //    1. the local IP port can be retrieved,
        //    2. send() can be used instead of sendto(), and
        //    3. the socket will only accept datagrams from the "connected" peer.
        // The socket can be disconnected by calling connect() again with the
        // socket family set AF_UNSPEC.
        if (connect(instance->sockfd,
                    instance->ainfo.ai_addr,
                    instance->ainfo.ai_addrlen) == 0)
        {
            retval = true;
            instance->state |= SOCKET_STATE_CONNECT;

            if (socket_instance_getaddrself(instance) == false)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d peer information is unavailable\n",
                               __FUNCTION__,
                              instance->sockfd);
            }
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
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket instance is not ready to connect\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t socket_udp_recv(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len)
{
    int32_t   retval  = -1;
    int32_t   flags   = MSG_DONTWAIT;
    socklen_t socklen = 0;

    if ((instance != NULL) && (buf != NULL) && (len > 0))
    {
        if (instance->state & SOCKET_STATE_CONNECT)
        {
            retval = recv(instance->sockfd, buf, len, flags);
        }
        else
        {
            socklen  = sizeof(instance->addrpeer.sockaddr);
            retval = recvfrom(instance->sockfd,
                              buf,
                              len,
                              flags,
                              (struct sockaddr *)&(instance->addrpeer.sockaddr),
                              &socklen);
        }

        if (retval > 0)
        {
            inet_ntop(AF_INET,
                      &(instance->addrpeer.sockaddr.sin_addr),
                      instance->addrpeer.ipaddr,
                      sizeof(instance->addrpeer.ipaddr));

            instance->addrpeer.ipport = ntohs(instance->addrpeer.sockaddr.sin_port);

            logger_printf(LOGGER_LEVEL_TRACE,
                          "%s: socket %d received %d bytes from %s:%u\n",
                          __FUNCTION__,
                          instance->sockfd,
                          retval,
                          instance->addrpeer.ipaddr,
                          instance->addrpeer.ipport);
        }
        // Check for socket errors if receive failed.
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
int32_t socket_udp_send(struct socket_instance * const instance,
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
        if (instance->state & SOCKET_STATE_CONNECT)
        {
            // sendto() could be used if the last two arguments were set to NULL
            // and 0, respectively.
            retval = send(instance->sockfd, buf, len, flags);
        }
        else
        {
            retval = sendto(instance->sockfd,
                            buf,
                            len,
                            flags,
                            (struct sockaddr *)&(instance->addrpeer.sockaddr),
                            sizeof(instance->addrpeer.sockaddr));
        }

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
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: buffer is empty\n",
                      __FUNCTION__);
    }

    return retval;
}
