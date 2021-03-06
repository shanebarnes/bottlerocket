/**
 * @file      sock_obj.c
 * @brief     Socket object implementation.
 * @author    Shane Barnes
 * @date      03 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "fion_poll.h"
#include "logger.h"
#include "sock_obj.h"
#include "util_date.h"
#include "util_debug.h"
#include "util_inet.h"
#include "util_unit.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Get a string representation of a socket option name.
 *
 * @param[in] name A socket option name.
 *
 * @return A string representation of a socket option name.
 */
static char *sockobj_getoptname(const uint32_t name)
{
    char *ret = NULL;

    switch (name)
    {
        case SO_REUSEADDR:
            ret = "SO_REUSEADDR";
            break;
        case SO_KEEPALIVE:
            ret = "SO_KEEPALIVE";
            break;
        case SO_LINGER:
            ret = "SO_LINGER";
            break;
#if defined(SO_REUSEPORT)
        case SO_REUSEPORT:
            ret = "SO_REUSEPORT";
            break;
#endif
#if defined(__APPLE__)
        case SO_NOSIGPIPE:
            ret = "SO_NOSIGPIPE";
            break;
#endif
        default:
            ret = "";
    }

    return ret;
}

bool sockobj_getaddrpeer(struct sockobj * const obj)
{
    bool ret = false;
    socklen_t socklen = 0;

    if (!UTILDEBUG_VERIFY((obj != NULL) &&
                         ((socklen = sizeof(obj->addrpeer.sockaddr)) > 0)))
    {
        // Do nothing.
    }
    else if (getpeername(obj->fd,
                         (struct sockaddr *)&(obj->addrpeer.sockaddr),
                         &socklen) != 0)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %u getpeername failed (%d)\n",
                      __FUNCTION__,
                      obj->sid,
                      errno);
    }
    else
    {
        ret = sockobj_getaddrsock(&obj->addrpeer);
    }

    return ret;
}

bool sockobj_getaddrself(struct sockobj * const obj)
{
    bool ret = false;
    socklen_t socklen = 0;

    if (!UTILDEBUG_VERIFY((obj != NULL) &&
                         ((socklen = sizeof(obj->addrself.sockaddr)) > 0)))
    {
        // Do nothing.
    }
    else if (getsockname(obj->fd,
                         (struct sockaddr *)&(obj->addrself.sockaddr),
                         &socklen) != 0)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %u getsockname failed (%d)\n",
                      __FUNCTION__,
                      obj->sid,
                      errno);
    }
    else
    {
        ret = sockobj_getaddrsock(&obj->addrself);
    }

    return ret;
}

bool sockobj_getaddrsock(struct sockobj_addr * const addr)
{
    bool ret = false;
    uint16_t *port = NULL;

    if (!UTILDEBUG_VERIFY(addr != NULL))
    {
        // Do nothing.
    }
    else if (inet_ntop(addr->sockaddr.ss_family,
                       utilinet_getaddrfromstorage(&addr->sockaddr),
                       addr->ipaddr,
                       sizeof(addr->ipaddr)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to convert address format (%d)\n",
                      __FUNCTION__,
                      errno);
    }
    else if ((port = utilinet_getportfromstorage(&addr->sockaddr)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to convert port format\n",
                      __FUNCTION__);
    }
    else
    {
        addr->ipport = ntohs(*port);

        snprintf(addr->sockaddrstr,
                 sizeof(addr->sockaddrstr),
                 "%s:%u",
                 addr->ipaddr,
                 addr->ipport);

        ret = true;
    }

    return ret;
}

bool sockobj_iserrfatal(const int32_t err)
{
    bool ret = false;

    switch (err)
    {
        // Fatal errors for both TCP and UDP sockets.
        case EBADF:
        case ECONNRESET:
        case EHOSTUNREACH:
        case EPIPE:
        case ENOTSOCK:
            ret = true;
            break;
        // Valid non-fatal errors.
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
            break;
    }

    return ret;
}

bool sockobj_create(struct sockobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(obj != NULL))
    {
        memset(obj, 0, sizeof(struct sockobj));

        if (!fionpoll_create(&obj->event))
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: event allocation failed\n",
                          __FUNCTION__);
        }
        else
        {
            obj->event.pevents = FIONOBJ_PEVENT_IN;
            //obj->event.ops.fion_setflags(&obj->event);
            ret = true;
        }
    }

    return ret;
}

bool sockobj_destroy(struct sockobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(obj != NULL))
    {
        ret = obj->event.ops.fion_destroy(&obj->event);

        obj->ops.sock_create   = NULL;
        obj->ops.sock_destroy  = NULL;
        obj->ops.sock_open     = NULL;
        obj->ops.sock_close    = NULL;
        obj->ops.sock_bind     = NULL;
        obj->ops.sock_getopts  = NULL;
        obj->ops.sock_setopts  = NULL;
        obj->ops.sock_listen   = NULL;
        obj->ops.sock_accept   = NULL;
        obj->ops.sock_connect  = NULL;
        obj->ops.sock_recv     = NULL;
        obj->ops.sock_send     = NULL;
        obj->ops.sock_shutdown = NULL;
    }

    return ret;
}

bool sockobj_open(struct sockobj * const obj)
{
    bool             ret      = false;
    int32_t          portsize = 0, flags;
    struct addrinfo *alist    = NULL, *anext = NULL, ahints;
    socklen_t        optlen, optval;
    char             ipport[6];

    if (UTILDEBUG_VERIFY(obj != NULL))
    {
        memset(&ahints, 0, sizeof(struct addrinfo));
        ahints.ai_family    = obj->conf.family;
        ahints.ai_socktype  = obj->conf.type;
        ahints.ai_flags     = AI_V4MAPPED | AI_ADDRCONFIG;
        ahints.ai_protocol  = 0;
        ahints.ai_canonname = NULL;
        ahints.ai_addr      = NULL;
        ahints.ai_next      = NULL;

        portsize = snprintf(ipport, 6, "%d", obj->conf.ipport);

        if ((portsize > 0) &&
            (portsize < 6) &&
            (getaddrinfo(obj->conf.ipaddr, ipport, &ahints, &alist) == 0))
        {
            for (anext = alist; anext != NULL; anext = anext->ai_next)
            {
                if ((anext->ai_family == obj->conf.family) &&
                    ((obj->fd = socket(anext->ai_family,
                                       anext->ai_socktype,
                                       anext->ai_protocol)) != -1))
                {
                    obj->addrself.sockaddr.ss_family = anext->ai_family;

                    inet_pton(obj->addrself.sockaddr.ss_family,
                              obj->conf.ipaddr,
                              utilinet_getaddrfromstorage(&obj->addrself.sockaddr));
                    *utilinet_getportfromstorage(&obj->addrself.sockaddr) = htons(obj->conf.ipport);
                    obj->addrself.addrlen = anext->ai_addrlen;

                    obj->addrpeer.sockaddr.ss_family = anext->ai_family;
                    inet_pton(obj->addrpeer.sockaddr.ss_family,
                              obj->conf.ipaddr,
                              utilinet_getaddrfromstorage(&obj->addrpeer.sockaddr));
                    *utilinet_getportfromstorage(&obj->addrpeer.sockaddr) = htons(obj->conf.ipport);
                    obj->addrpeer.addrlen = anext->ai_addrlen;

                    optlen = sizeof(obj->info.recv.winsize);
                    optval = 1;
                    obj->event.ops.fion_insertfd(&obj->event, obj->fd);
                    flags = fcntl(obj->fd, F_GETFL, 0);

                    if (!obj->event.ops.fion_setflags(&obj->event))
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u event creation failed\n",
                                      __FUNCTION__,
                                      obj->sid,
                                      errno);
                        sockobj_close(obj);
                    }
                    else if (fcntl(obj->fd, F_SETFL, flags | O_NONBLOCK) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u O_NONBLOCK option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sid,
                                      errno);

                    }
                    // @todo - SO_LINGER? etc
                    else if (setsockopt(obj->fd,
                                        SOL_SOCKET,
                                        SO_REUSEADDR,
                                        &optval,
                                        sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u SO_REUSEADDR option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sid,
                                      errno);
                        sockobj_close(obj);
                    }
#if defined(SO_REUSEPORT)
                    else if (setsockopt(obj->fd,
                                        SOL_SOCKET,
                                        SO_REUSEPORT,
                                        &optval,
                                        sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u SO_REUSEPORT option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sid,
                                      errno);
                        sockobj_close(obj);
                    }
#endif
#if defined(__APPLE__)
                    else if (setsockopt(obj->fd,
                             SOL_SOCKET,
                             SO_NOSIGPIPE,
                             &optval,
                             sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u SO_NOSIGPIPE option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sid,
                                      errno);
                        sockobj_close(obj);
                    }
#endif
                    else if (getsockopt(obj->fd,
                                        SOL_SOCKET,
                                        SO_RCVBUF,
                                        &obj->info.recv.winsize,
                                        &optlen) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u SO_RCVBUF option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sid,
                                      errno);
                        sockobj_close(obj);
                    }
                    else if (getsockopt(obj->fd,
                                        SOL_SOCKET,
                                        SO_SNDBUF,
                                        &obj->info.send.winsize,
                                        &optlen) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u SO_SNDBUF option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sid,
                                      errno);
                        sockobj_close(obj);
                    }
                    else
                    {
                        tokenbucket_init(&obj->tb, obj->conf.ratelimitbps);
                        obj->state = SOCKOBJ_STATE_OPEN;
                        ret = true;
                    }

                    break;
                }
            }

            freeaddrinfo(alist);
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to get address information (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return ret;
}

bool sockobj_close(struct sockobj * const obj)
{
    int32_t ret = false;

    if (UTILDEBUG_VERIFY(obj != NULL))
    {
        if (close(obj->fd) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %u could not be closed (%d)\n",
                          __FUNCTION__,
                          obj->sid,
                          errno);
        }
        else
        {
            obj->info.stopusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                    UNIT_TIME_USEC);
            ret = true;
        }

        obj->state = SOCKOBJ_STATE_CLOSE;
    }

    return ret;
}

bool sockobj_bind(struct sockobj * const obj)
{
    bool ret = false;

    if (!UTILDEBUG_VERIFY(obj != NULL))
    {
        // Do nothing.
    }
    else if (bind(obj->fd,
                  (struct sockaddr*)&obj->addrself.sockaddr,
                  obj->addrself.addrlen) == 0)
    {
        sockobj_getaddrself(obj);
        obj->state |= SOCKOBJ_STATE_BIND;
        ret = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %u bind failed (%d)\n",
                      __FUNCTION__,
                      obj->sid,
                      errno);
    }

    return ret;
}

bool sockobj_getopts(struct sockobj * const obj, struct vector * const opts)
{
    bool ret = false;
    int32_t errval = 0;
    struct sockobj_opt *opt = NULL;
    uint32_t i;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (opts != NULL) &&
                         (vector_getsize(opts) > 0)))
    {
        ret = true;

        for (i = 0; i < vector_getsize(opts); i++)
        {
            opt = (struct sockobj_opt *)vector_getval(opts, i);

            if (opt == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u option index %u is empty\n",
                              __FUNCTION__,
                              obj->sid,
                              i);
                ret = false;
            }
            else
            {
                errval = getsockopt(obj->fd,
                                    opt->level,
                                    opt->name,
                                    &opt->val,
                                    &opt->len);

                if (errval != 0)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %u '%s' option failed (%d)\n",
                                  __FUNCTION__,
                                  obj->sid,
                                  sockobj_getoptname(opt->name),
                                  errno);
                    ret = false;
                }
            }
        }
    }

    return ret;
}

bool sockobj_setopts(struct sockobj * const obj, struct vector * const opts)
{
    bool ret = false;
    int32_t err = 0;
    struct sockobj_opt *opt = NULL;
    uint32_t i;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (opts != NULL) &&
                         (vector_getsize(opts) > 0)))
    {
        ret = true;

        for (i = 0; i < vector_getsize(opts); i++)
        {
            opt = (struct sockobj_opt *)vector_getval(opts, i);

            if (opt == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %u option index %u is empty\n",
                              __FUNCTION__,
                              obj->sid,
                              i);
                ret = false;
            }
            else
            {
                err = setsockopt(obj->fd,
                                 opt->level,
                                 opt->name,
                                 &opt->val,
                                 sizeof(opt->val));

                if (err != 0)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %u '%s' option failed (%d)\n",
                                  __FUNCTION__,
                                  obj->sid,
                                  sockobj_getoptname(opt->name),
                                  errno);
                    ret = false;
                }
            }
        }
    }

    return ret;
}
