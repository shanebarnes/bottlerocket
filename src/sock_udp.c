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
#include "util_inet.h"
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
 * @brief Get the maximum UDP message size in bytes.
 *
 * @param[in] obj A pointer to a socket object.
 *
 * @return The maximum UDP message size in bytes (-1 on error).
 */
static int32_t sockudp_getmaxmsgsize(struct sockobj * const obj)
{
    int32_t ret = -1;
    int32_t mtu = utilioctl_getifmtubyaddr((struct sockaddr_in*)&(obj->addrself.sockaddr));
    int32_t minhdrlen = 28; // IPv4 header (20) + UDP header (8)

    if (mtu > minhdrlen)
    {
        ret = mtu - minhdrlen;
#if defined(__APPLE__)
        // The maximum message size may be smaller than the network interface
        // MTU-derived value (see sysctl net.inet.udp.maxdgram).
        int32_t size = utilsysctl_getmaxudpsize();

        if ((size > -1) && (size < ret))
        {
            ret = size;
        }
#endif
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_create(struct sockobj * const obj)
{
    bool ret = false;

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
            obj->ops.sock_create   = sockudp_create;
            obj->ops.sock_destroy  = sockudp_destroy;
            obj->ops.sock_open     = sockobj_open;
            obj->ops.sock_close    = sockobj_close;
            obj->ops.sock_bind     = sockobj_bind;
            obj->ops.sock_getopts  = sockobj_getopts;
            obj->ops.sock_setopts  = sockobj_setopts;
            obj->ops.sock_listen   = sockudp_listen;
            obj->ops.sock_accept   = sockudp_accept;
            obj->ops.sock_connect  = sockudp_connect;
            obj->ops.sock_recv     = sockudp_recv;
            obj->ops.sock_send     = sockudp_send;
            obj->ops.sock_shutdown = sockudp_shutdown;

            obj->conf.type = SOCK_DGRAM;

            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_destroy(struct sockobj * const obj)
{
    bool ret = false;

    if ((obj == NULL) || (obj->conf.type != SOCK_DGRAM))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        ret = obj->ops.sock_destroy(obj);
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_listen(struct sockobj * const obj, const int32_t backlog)
{
    bool ret = false;

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
        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_accept(struct sockobj * const listener,
                    struct sockobj * const obj)
{
    bool ret = false;

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

                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_connect(struct sockobj * const obj)
{
    bool ret = false;

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
            ret = true;

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

    return ret;
}

/**
 * @see See header file for interface comments.
 */
int32_t sockudp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t   ret     = -1;
    int32_t   flags   = MSG_DONTWAIT;
    socklen_t socklen = 0;

    if ((obj == NULL) || (buf == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (obj->state & SOCKOBJ_STATE_CONNECT)
        {
            ret = recv(obj->sockfd, buf, len, flags);
        }
        else
        {
            socklen = sizeof(obj->addrpeer.sockaddr);
            ret = recvfrom(obj->sockfd,
                              buf,
                              len,
                              flags,
                              (struct sockaddr *)&(obj->addrpeer.sockaddr),
                              &socklen);
            sockobj_setstats(&obj->info.recv, ret);

            if (ret > 0)
            {
                if ((obj->state & SOCKOBJ_STATE_CONNECT) == 0)
                {
                    inet_ntop(obj->conf.family,
                              utilinet_getaddrfromstorage(&obj->addrpeer.sockaddr),
                              obj->addrpeer.ipaddr,
                              sizeof(obj->addrpeer.ipaddr));
                    obj->addrpeer.ipport = ntohs(*utilinet_getportfromstorage(&obj->addrpeer.sockaddr));
                }

                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %d received %d bytes from %s:%u\n",
                              __FUNCTION__,
                              obj->sockfd,
                              ret,
                              obj->addrpeer.ipaddr,
                              obj->addrpeer.ipport);
            }
            else
            {
                if (sockobj_iserrfatal(errno) == true)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d fatal error (%d)\n",
                                  __FUNCTION__,
                                  obj->sockfd,
                                  errno);
                    ret = -1;
                }
                else
                {
                    logger_printf(LOGGER_LEVEL_TRACE,
                                  "%s: socket %d non-fatal error (%d)\n",
                                  __FUNCTION__,
                                  obj->sockfd,
                                  errno);
                    ret = 0;
                }

                if (ret == 0)
                {
                    if (obj->event.ops.fion_poll(&obj->event) == false)
                    {
                        ret = -1;
                    }
                    else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
                    {
                        ret = -1;
                    }
                }
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
int32_t sockudp_send(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t ret   = -1;
    int32_t flags = MSG_DONTWAIT;
#if defined(__linux__)
    flags |= MSG_NOSIGNAL;
#endif

    if ((obj == NULL) || (buf == NULL))
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
            ret = send(obj->sockfd, buf, len, flags);
        }
        else
        {
            ret = sendto(obj->sockfd,
                            buf,
                            len,
                            flags,
                            (struct sockaddr *)&(obj->addrpeer.sockaddr),
                            sizeof(obj->addrpeer.sockaddr));
        }

        sockobj_setstats(&obj->info.send, ret);

        if (ret > 0)
        {
            logger_printf(LOGGER_LEVEL_TRACE,
                          "%s: socket %d sent %d bytes\n",
                          __FUNCTION__,
                          obj->sockfd,
                          ret);
        }
        else
        {
            if (errno == EMSGSIZE)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: datagram payload (%u) is larger than the"
                              " maximum message size (%u)\n",
                              __FUNCTION__,
                              len,
                              sockudp_getmaxmsgsize(obj));
                ret = -1;
            }
            else if (sockobj_iserrfatal(errno) == true)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d fatal error (%d)\n",
                              __FUNCTION__,
                              obj->sockfd,
                              errno);
                ret = -1;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %d non-fatal error (%d)\n",
                              __FUNCTION__,
                              obj->sockfd,
                              errno);
                ret = 0;
            }

            if (ret == 0)
            {
                if (obj->event.ops.fion_poll(&obj->event) == false)
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_INREADY)
                {
                    ret = recv(obj->sockfd, buf, len, flags);
                    sockobj_setstats(&obj->info.recv, ret);

                    // Remote peer is closed if input is ready but no bytes are
                    // received (EOF).
                    if (ret > 0)
                    {
                        logger_printf(LOGGER_LEVEL_TRACE,
                                      "%s: socket %d received %d bytes\n",
                                      __FUNCTION__,
                                      obj->sockfd,
                                      ret);
                    }
                    else
                    {
                        ret = -1;
                    }
                }
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_shutdown(struct sockobj * const obj, const int32_t how)
{
    bool ret = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        switch (how)
        {
            case SHUT_RD:
            case SHUT_RDWR:
            case SHUT_WR:
            default:
                // Implement connection-oriented signaling using zero-length
                // datagrams.
                break;
        }
    }

    return ret;
}
