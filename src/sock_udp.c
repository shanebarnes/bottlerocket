/**
 * @file      sock_udp.c
 * @brief     UDP socket implementation.
 * @author    Shane Barnes
 * @date      21 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "sock_con.h"
#include "sock_udp.h"
#include "util_date.h"
#include "util_debug.h"
#include "util_inet.h"
#include "util_ioctl.h"
#if defined(__APPLE__)
    #include "util_sysctl.h"
#endif
#include "util_unit.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

// temp
#include <unistd.h>
#include <arpa/inet.h>
// temp

static struct sockcon con = {.sock = NULL, .priv = NULL};

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

    if (UTILDEBUG_VERIFY(obj != NULL) == true)
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

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->conf.type == SOCK_DGRAM)) == true)
    {
        if (obj->state & SOCKOBJ_STATE_LISTEN)
        {
            sockcon_destroy(&con);
            con.sock = NULL;
            con.priv = NULL;
        }

        ret = sockobj_destroy(obj);
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_listen(struct sockobj * const obj, const int32_t backlog)
{
    bool ret = false;

    // @todo SO_REUSEPORT could be used on kernels that support its use.

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->conf.type == SOCK_DGRAM)) == false)
    {
        // Do nothing.
    }
    else if (sockcon_create(&con) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to create listener\n",
                      __FUNCTION__);
    }
    else if (sockcon_listen(&con, obj, backlog) == true)
    {
        logger_printf(LOGGER_LEVEL_INFO,
                      "%s: socket %u listening with a backlog of %d\n",
                      __FUNCTION__,
                      obj->id,
                      backlog);
        obj->state |= SOCKOBJ_STATE_LISTEN;
        sockobj_getaddrself(obj);
        ret = true;
    }
    else
    {
        sockcon_destroy(&con);
        con.sock = NULL;
        con.priv = NULL;

        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %u failed to listen on %s:%u (%d)\n",
                      __FUNCTION__,
                      obj->fd,
                      obj->conf.ipaddr,
                      obj->conf.ipport,
                      errno);
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockudp_accept(struct sockobj * const listener,
                    struct sockobj * const obj)
{
    bool     ret  = false;
    int32_t  fd   = -1;
    uint64_t ts   = 0;
    socklen_t len = sizeof(struct sockaddr_storage), optval = 0;
    struct sockaddr_storage addr;

    if (UTILDEBUG_VERIFY((listener != NULL) &&
                         (listener->conf.type == SOCK_DGRAM) &&
                         (obj != NULL)) == true)
    {
        // @todo If timeout is -1 (blocking), then the poll should occur in a
        //       loop with a small timeout (e.g., 100 ms) or maybe a self-pipe
        //       for signaling shutdown events, etc.

        if ((fd = sockcon_accept(&con, (struct sockaddr *)&(addr), &len)) > - 1)
        {
            ts = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);

            if (sockudp_create(obj) == false)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u accept initialization failed\n",
                              __FUNCTION__,
                              obj->id);
            }
            else if (((obj->fd = fd) != fd) ||
                     (obj->event.ops.fion_insertfd(&obj->event, fd) == false) ||
                     (obj->event.ops.fion_setflags(&obj->event) == false))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u fd clone failed\n",
                              __FUNCTION__,
                              obj->id);
            }
            else if (memcpy(&obj->conf,
                            &listener->conf,
                            sizeof(listener->conf)) == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u configuration clone failed\n",
                              __FUNCTION__,
                              obj->id);
            }
            //else if (sockobj_getaddrself(listener) == false) // this requires that a datagram has been sent (zero payload handshaking?)
            //{
            //    logger_printf(LOGGER_LEVEL_ERROR,
            //                  "%s: socket %u self information is unavailable\n",
            //                  __FUNCTION__,
            //                  obj->id);
            //}
            else
            {
                memcpy(&obj->addrpeer.sockaddr, &addr, sizeof(obj->addrpeer.sockaddr));
                sockobj_getaddrsock(&obj->addrpeer);
                memcpy(&obj->addrself, &listener->addrself, sizeof(obj->addrself));

                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: new socket %u accepted on %s from %s\n",
                              obj->id,
                              obj->addrself.sockaddrstr,
                              obj->addrpeer.sockaddrstr);

                tokenbucket_init(&obj->tb, obj->conf.ratelimitbps);
                obj->state = SOCKOBJ_STATE_OPEN | SOCKOBJ_STATE_CONNECT;
                obj->info.startusec = ts;

                // @todo Need to invoke sockobj_open() without creating a file
                //       descriptor.
                // @todo Listener and child sockets may need to have larger send
                //       and/or receive buffers than the system defaults.
                optval = listener->info.recv.winsize;
                setsockopt(obj->fd,
                           SOL_SOCKET,
                           SO_RCVBUF,
                           &optval,
                           sizeof(optval));
                optval = listener->info.send.winsize;
                setsockopt(obj->fd,
                           SOL_SOCKET,
                           SO_SNDBUF,
                           &optval,
                           sizeof(optval));

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

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->conf.type == SOCK_DGRAM) &&
                         (obj->state == SOCKOBJ_STATE_OPEN)) == true)
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
                    obj->ainfo.ai_addr,
                    obj->ainfo.ai_addrlen) == 0)
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
                          obj->id,
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
    int32_t    ret     = -1;
    int32_t    flags   = MSG_DONTWAIT;
    socklen_t  socklen = 0;
    uint16_t  *port    = NULL;

    if (UTILDEBUG_VERIFY((obj != NULL) && (buf != NULL)) == true)
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
                          obj->id,
                          ret,
                          obj->addrpeer.ipaddr,
                          obj->addrpeer.ipport);
        }
        else if (ret < 0)
        {
            if (sockobj_iserrfatal(errno) == true)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);
                ret = -1;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %u non-fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
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
        else
        {
            // For UDP sockets, a return value 0 does not mean a peer has closed
            // its half side of the "connection."
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

    if (UTILDEBUG_VERIFY((obj != NULL) && (buf != NULL)) == true)
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
                          obj->id,
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
                              "%s: socket %u fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
                              errno);
                ret = -1;
            }
            else
            {
                logger_printf(LOGGER_LEVEL_TRACE,
                              "%s: socket %u non-fatal error (%d)\n",
                              __FUNCTION__,
                              obj->id,
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
                    ret = recv(obj->fd, buf, len, flags);
                    sockobj_setstats(&obj->info.recv, ret);

                    // Remote peer is closed if input is ready but no bytes are
                    // received (EOF).
                    if (ret > 0)
                    {
                        logger_printf(LOGGER_LEVEL_TRACE,
                                      "%s: socket %u received %d bytes\n",
                                      __FUNCTION__,
                                      obj->id,
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

    if (UTILDEBUG_VERIFY(obj != NULL) == true)
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
