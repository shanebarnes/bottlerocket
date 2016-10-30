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
#include "util_debug.h"
#include "util_inet.h"
#include "util_ioctl.h"
#if defined(__APPLE__)
    #include "util_sysctl.h"
#endif
#include "util_unit.h"

#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

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

bool sockudp_create(struct sockobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(obj != NULL))
    {
        if (sockobj_create(obj))
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

bool sockudp_destroy(struct sockobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->conf.type == SOCK_DGRAM)))
    {
        ret = sockobj_destroy(obj);
    }

    return ret;
}

bool sockudp_listen(struct sockobj * const obj, const int32_t backlog)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->conf.type == SOCK_DGRAM)))
    {
        logger_printf(LOGGER_LEVEL_INFO,
                      "%s: socket %u listening with a backlog of %d\n",
                      __FUNCTION__,
                      obj->sid,
                      backlog);
        obj->state |= SOCKOBJ_STATE_LISTEN;
        sockobj_getaddrself(obj);
        ret = true;
    }

    return ret;
}

bool sockudp_accept(struct sockobj * const listener, struct sockobj * const obj)
{
    bool     ret = false;
    uint64_t ts  = 0;
    uint8_t  buffer;

    if (UTILDEBUG_VERIFY((listener != NULL) &&
                         (listener->conf.type == SOCK_DGRAM) &&
                         (obj != NULL)))
    {
        // @todo If timeout is -1 (blocking), then the poll should occur in a
        //       loop with a small timeout (e.g., 100 ms) or maybe a self-pipe
        //       for signaling shutdown events, etc.

        if ((listener->event.ops.fion_poll(&listener->event)) &&
            (listener->event.revents & FIONOBJ_REVENT_INREADY))
        {
            ts = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);

            // @todo If SO_REUSEPORT is enabled, then two listeners should be
            //       active at all times to prevent the potential loss of new
            //       incoming "connections."

            if (memcpy(obj, listener, sizeof(*obj)) == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u fd clone failed\n",
                              __FUNCTION__,
                              obj->sid);
            }
            else if (obj->ops.sock_recv(obj, &buffer, sizeof(buffer)) < 0)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u accept initialization failed\n",
                              __FUNCTION__,
                              obj->sid);
            }
            else if (!sockobj_getaddrsock(&obj->addrpeer))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u self information is unavailable\n",
                              __FUNCTION__,
                              obj->sid);
            }
            else if (connect(obj->fd,
                             (struct sockaddr*)&obj->addrpeer.sockaddr,
                             obj->addrpeer.addrlen) != 0)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u peer information is unavailable\n",
                              __FUNCTION__,
                              obj->sid);
            }
            else
            {
                obj->state = SOCKOBJ_STATE_OPEN | SOCKOBJ_STATE_CONNECT;
                obj->info.startusec = ts;
                sockobj_getaddrself(obj);
                tokenbucket_init(&obj->tb, obj->conf.ratelimitbps);

                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: new socket %u accepted on %s from %s\n",
                              obj->sid,
                              obj->addrself.sockaddrstr,
                              obj->addrpeer.sockaddrstr);

                ret = true;
            }

            vector_resize(&listener->event.fds, 0);
            listener->ops.sock_open(listener);
            listener->ops.sock_bind(listener);
            listener->ops.sock_listen(listener, listener->conf.backlog);
        }
    }

    return ret;
}

bool sockudp_connect(struct sockobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->conf.type == SOCK_DGRAM) &&
                         (obj->state == SOCKOBJ_STATE_OPEN)))
    {
        if (obj->info.startusec == 0)
        {
            obj->info.startusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                     UNIT_TIME_USEC);
        }

        // There are a number of benefits to calling connect() on a UDP socket:
        //    1. the local IP port can be retrieved,
        //    2. send() can be used instead of sendto(), and
        //    3. the socket will only accept datagrams from the "connected"
        //       peer.
        // The socket can be disconnected by calling connect() again with the
        // socket family set AF_UNSPEC.
        if (connect(obj->fd,
                    (struct sockaddr*)&obj->addrpeer.sockaddr,
                    obj->addrpeer.addrlen) == 0)
        {
            obj->state |= SOCKOBJ_STATE_CONNECT;

            // @todo Use a 3-way transport layer handshake using zero payload
            //       datagrams.
            obj->ops.sock_send(obj, &ret, 0);

            sockobj_getaddrself(obj);
            sockobj_getaddrpeer(obj);

            ret = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %u connect error (%d)\n",
                          __FUNCTION__,
                          obj->sid,
                          errno);
        }
    }

    return ret;
}

int32_t sockudp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t    ret     = -1;
    int32_t    flags   = MSG_DONTWAIT;
    socklen_t  socklen = 0;
    uint16_t  *port    = NULL;

    if (UTILDEBUG_VERIFY((obj != NULL) && (buf != NULL)))
    {
        if (obj->state & SOCKOBJ_STATE_CONNECT)
        {
            ret = recv(obj->fd, buf, len, flags);
        }
        else
        {
            socklen = sizeof(obj->addrpeer.sockaddr);
            ret = recvfrom(obj->fd,
                           buf,
                           len,
                           flags,
                           (struct sockaddr *)&(obj->addrpeer.sockaddr),
                           &socklen);
        }

        sockobj_setstats(&obj->info.recv, ret);

        if (ret > 0)
        {
            if ((obj->state & SOCKOBJ_STATE_CONNECT) == 0)
            {
                inet_ntop(obj->conf.family,
                          utilinet_getaddrfromstorage(&obj->addrpeer.sockaddr),
                          obj->addrpeer.ipaddr,
                          sizeof(obj->addrpeer.ipaddr));

                port = utilinet_getportfromstorage(&obj->addrpeer.sockaddr);

                if (port != NULL)
                {
                    obj->addrpeer.ipport = ntohs(*port);
                }
            }

            logger_printf(LOGGER_LEVEL_TRACE,
                          "%s: socket %u received %d bytes from %s:%u\n",
                          __FUNCTION__,
                          obj->sid,
                          ret,
                          obj->addrpeer.ipaddr,
                          obj->addrpeer.ipport);
        }
        else if (ret < 0)
        {
            if (sockobj_iserrfatal(errno))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u fatal error (%d)\n",
                              __FUNCTION__,
                              obj->sid,
                              errno);
                ret = -1;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %u non-fatal error (%d)\n",
                              __FUNCTION__,
                              obj->sid,
                              errno);
                ret = 0;
            }

            if (ret == 0)
            {
                if (!obj->event.ops.fion_poll(&obj->event))
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
                {
                    ret = -1;
                }
            }
        }
        else
        {
            // For UDP sockets, a return value 0 does not mean a peer has closed
            // its half side of the "connection."
        }
    }

    return ret;
}

int32_t sockudp_send(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len)
{
    int32_t ret   = -1;
    int32_t flags = MSG_DONTWAIT;
#if defined(__linux__)
    flags |= MSG_NOSIGNAL;
#endif

    if (UTILDEBUG_VERIFY((obj != NULL) && (buf != NULL)))
    {
        if (obj->state & SOCKOBJ_STATE_CONNECT)
        {
            // sendto() could be used if the last two arguments were set to NULL
            // and 0, respectively.
            ret = send(obj->fd, buf, len, flags);
        }
        else
        {
            ret = sendto(obj->fd,
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
                          "%s: socket %u sent %d bytes\n",
                          __FUNCTION__,
                          obj->sid,
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
            else if (sockobj_iserrfatal(errno))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u fatal error (%d)\n",
                              __FUNCTION__,
                              obj->sid,
                              errno);
                ret = -1;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %u non-fatal error (%d)\n",
                              __FUNCTION__,
                              obj->sid,
                              errno);
                ret = 0;
            }

            if (ret == 0)
            {
                if (!obj->event.ops.fion_poll(&obj->event))
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_ERROR)
                {
                    ret = -1;
                }
                else if (obj->event.revents & FIONOBJ_REVENT_INREADY)
                {
                    ret = recv(obj->fd, buf, len, flags);
                    sockobj_setstats(&obj->info.recv, ret);

                    // Remote peer is closed if input is ready but no bytes are
                    // received (EOF).
                    if (ret > 0)
                    {
                        logger_printf(LOGGER_LEVEL_TRACE,
                                      "%s: socket %u received %d bytes\n",
                                      __FUNCTION__,
                                      obj->sid,
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

bool sockudp_shutdown(struct sockobj * const obj, const int32_t how)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(obj != NULL))
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
