/**
 * @file      sock_udp.c
 * @brief     UDP socket implementation.
 * @author    Shane Barnes
 * @date      21 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "sock_udp.h"
#include "util_date.h"
#include "util_ioctl.h"
#if defined(__APPLE__)
    #include "util_sysctl.h"
#endif
#include "util_unit.h"

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
bool sockudp_create(struct sockobj * const obj)
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
            obj->ops.sock_create  = sockudp_create;
            obj->ops.sock_destroy = sockudp_destroy;
            obj->ops.sock_open    = sockobj_open;
            obj->ops.sock_close   = sockobj_close;
            obj->ops.sock_bind    = sockobj_bind;
            obj->ops.sock_getopts = sockobj_getopts;
            obj->ops.sock_setopts = sockobj_setopts;
            obj->ops.sock_listen  = sockudp_listen;
            obj->ops.sock_accept  = sockudp_accept;
            obj->ops.sock_connect = sockudp_connect;
            obj->ops.sock_recv    = sockudp_recv;
            obj->ops.sock_send    = sockudp_send;

            obj->conf.type = SOCK_DGRAM;

            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_destroy(struct sockobj * const obj)
{
    bool retval = false;

    if ((obj == NULL) || (obj->conf.type != SOCK_DGRAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = obj->ops.sock_destroy(obj);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_listen(struct sockobj * const obj, const int32_t backlog)
{
    bool retval = false;

    // @todo Consider using this API to create the size of the UDP "connection"
    //       poll.

    // @todo SO_REUSEPORT could be used on kernels that support its use.

    if ((obj == NULL) || (obj->conf.type != SOCK_DGRAM) || (backlog == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_accept(struct sockobj * const listener,
                    struct sockobj * const obj)
{
    bool retval = false;

    // @todo Consider using this API to poll for datagrams available to be read.
    //       Once read, the datagrams could be filtered and associated with the
    //       appropriate socket obj in the "connection" backlog.

    if ((listener == NULL) ||
        (obj == NULL) ||
        (listener->conf.type != SOCK_DGRAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        // @todo If timeout is -1 (blocking), then the poll should occur in a
        //       loop with a small timeout (e.g., 100 ms) or maybe a self-pipe
        //       for signaling shutdown events, etc.

        if (listener->event.ops.fion_poll(&listener->event) == true)
        {
            if (((listener->event.revents & FIONOBJ_REVENT_TIMEOUT) == 0) &&
                ((listener->event.revents & FIONOBJ_REVENT_ERROR) == 0))
            {
                // @todo Clone/duplicate listener socket
                listener->ops.sock_create(obj);
                listener->ops.sock_open(obj);
                obj->sockfd = listener->sockfd;

                retval = true;
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_connect(struct sockobj * const obj)
{
    bool retval = false;

    if ((obj == NULL) || (obj->conf.type != SOCK_DGRAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->info.startusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                 UNIT_TIME_USEC);

        // There are a number of benefits to calling connect() on a UDP socket:
        //    1. the local IP port can be retrieved,
        //    2. send() can be used instead of sendto(), and
        //    3. the socket will only accept datagrams from the "connected"
        //       peer.
        // The socket can be disconnected by calling connect() again with the
        // socket family set AF_UNSPEC.
        if (connect(obj->sockfd,
                    obj->ainfo.ai_addr,
                    obj->ainfo.ai_addrlen) == 0)
        {
            obj->state |= SOCKOBJ_STATE_CONNECT;
            retval = true;

            if (sockobj_getaddrself(obj) == false)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d peer information is unavailable\n",
                              __FUNCTION__,
                              obj->sockfd);
            }
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

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t sockudp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t   retval  = -1;
    int32_t   flags   = MSG_DONTWAIT;
    socklen_t socklen = 0;

    if ((obj == NULL) || (buf == NULL) || (len == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (obj->state & SOCKOBJ_STATE_CONNECT)
        {
            retval = recv(obj->sockfd, buf, len, flags);
        }
        else
        {
            socklen = sizeof(obj->addrpeer.sockaddr);
            retval = recvfrom(obj->sockfd,
                              buf,
                              len,
                              flags,
                              (struct sockaddr *)&(obj->addrpeer.sockaddr),
                              &socklen);

            if (retval > 0)
            {
                if ((obj->state & SOCKOBJ_STATE_CONNECT) == 0)
                {
                    inet_ntop(AF_INET,
                              &(obj->addrpeer.sockaddr.sin_addr),
                              obj->addrpeer.ipaddr,
                              sizeof(obj->addrpeer.ipaddr));

                    obj->addrpeer.ipport = ntohs(obj->addrpeer.sockaddr.sin_port);
                }

                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %d received %d bytes from %s:%u\n",
                              __FUNCTION__,
                              obj->sockfd,
                              retval,
                              obj->addrpeer.ipaddr,
                              obj->addrpeer.ipport);
                obj->info.recvbytes += retval;
            }
            // Check for socket errors if receive failed.
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
                    if (obj->event.ops.fion_poll(&obj->event) == false)
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
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t sockudp_send(struct sockobj * const obj,
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
        if (obj->state & SOCKOBJ_STATE_CONNECT)
        {
            // sendto() could be used if the last two arguments were set to NULL
            // and 0, respectively.
            retval = send(obj->sockfd, buf, len, flags);
        }
        else
        {
            retval = sendto(obj->sockfd,
                            buf,
                            len,
                            flags,
                            (struct sockaddr *)&(obj->addrpeer.sockaddr),
                            sizeof(obj->addrpeer.sockaddr));
        }

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
                case EMSGSIZE:
                    // On Mac, the maximum message size may be smaller than the
                    // the network interface MTU (see sysctl
                    // net.inet.udp.maxdgram).
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: datagram payload (%u) is larger than the"
                                  " maximum message size (%u)\n",
                                  __FUNCTION__,
                                  len,
#if defined(__APPLE__)
                                  utilsysctl_getmaxudpsize());
#else
                                  utilioctl_getifmtubyaddr(obj->addrself.sockaddr));
#endif
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
                if (obj->event.ops.fion_poll(&obj->event) == false)
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
