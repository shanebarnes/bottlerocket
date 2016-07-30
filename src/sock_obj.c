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
#include "util_inet.h"
#include "util_unit.h"

#include <arpa/inet.h>
#include <errno.h>
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

/**
 * @see See header file for interface comments.
 */
bool sockobj_getaddrpeer(struct sockobj * const obj)
{
    bool ret = false;
    socklen_t socklen = 0;

    if ((obj == NULL) || ((socklen = sizeof(obj->addrpeer.sockaddr)) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (getpeername(obj->fd,
                         (struct sockaddr *)&(obj->addrpeer.sockaddr),
                         &socklen) != 0)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %u getpeername failed (%d)\n",
                      __FUNCTION__,
                      obj->id,
                      errno);
    }
    else
    {
        inet_ntop(obj->addrpeer.sockaddr.ss_family,
                  utilinet_getaddrfromstorage(&obj->addrpeer.sockaddr),
                  obj->addrpeer.ipaddr,
                  sizeof(obj->addrpeer.ipaddr));
        obj->addrpeer.ipport = ntohs(*utilinet_getportfromstorage(&obj->addrpeer.sockaddr));

        snprintf(obj->addrpeer.sockaddrstr,
                 sizeof(obj->addrpeer.sockaddrstr),
                 "%s:%u",
                 obj->addrpeer.ipaddr,
                 obj->addrpeer.ipport);

        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_getaddrself(struct sockobj * const obj)
{
    bool ret = false;
    socklen_t socklen = 0;

    if ((obj == NULL) || ((socklen = sizeof(obj->addrself.sockaddr)) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (getsockname(obj->fd,
                         (struct sockaddr *)&(obj->addrself.sockaddr),
                         &socklen) != 0)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %u getsockname failed (%d)\n",
                      __FUNCTION__,
                      obj->id,
                      errno);
    }
    else
    {
        inet_ntop(obj->addrself.sockaddr.ss_family,
                  utilinet_getaddrfromstorage(&obj->addrself.sockaddr),
                  obj->addrself.ipaddr,
                  sizeof(obj->addrself.ipaddr));
        obj->addrself.ipport = ntohs(*utilinet_getportfromstorage(&obj->addrself.sockaddr));

        snprintf(obj->addrself.sockaddrstr,
                 sizeof(obj->addrself.sockaddrstr),
                 "%s:%u",
                 obj->addrself.ipaddr,
                 obj->addrself.ipport);

        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
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

/**
 * @see See sock_create() for interface comments.
 */
bool sockobj_create(struct sockobj * const obj)
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
        memset(obj, 0, sizeof(struct sockobj));

        if (fionpoll_create(&obj->event) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: event allocation failed\n",
                          __FUNCTION__);
        }
        else
        {
            obj->event.pevents = FIONOBJ_PEVENT_IN;
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See sock_destroy() for interface comments.
 */
bool sockobj_destroy(struct sockobj * const obj)
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

/**
 * @see See header file for interface comments.
 */
bool sockobj_open(struct sockobj * const obj)
{
    bool             ret      = false;
    int32_t          portsize = 0;
    struct addrinfo *anext    = NULL, ahints;
    socklen_t        optlen, optval;
    char             ipport[6];

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
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

        obj->alist = NULL;

        if ((portsize > 0) &&
            (portsize < 6) &&
            (getaddrinfo(obj->conf.ipaddr,
                         ipport,
                         &ahints,
                         &(obj->alist)) == 0))
        {
            for (anext = obj->alist; anext != NULL; anext = anext->ai_next)
            {
                if ((anext->ai_family == obj->conf.family) &&
                    ((obj->fd = socket(anext->ai_family,
                                       anext->ai_socktype,
                                       anext->ai_protocol)) != -1))
                {
                    obj->ainfo = *anext;

                    obj->addrself.sockaddr.ss_family = anext->ai_family;

                    inet_pton(obj->addrself.sockaddr.ss_family,
                              obj->conf.ipaddr,
                              utilinet_getaddrfromstorage(&obj->addrself.sockaddr));
                    *utilinet_getportfromstorage(&obj->addrself.sockaddr) = htons(obj->conf.ipport);

                    obj->addrpeer.sockaddr.ss_family = anext->ai_family;
                    inet_pton(obj->addrpeer.sockaddr.ss_family,
                              obj->conf.ipaddr,
                              utilinet_getaddrfromstorage(&obj->addrpeer.sockaddr));
                    *utilinet_getportfromstorage(&obj->addrpeer.sockaddr) = htons(obj->conf.ipport);

                    optlen = sizeof(obj->info.recv.winsize);
                    optval = 1;
                    obj->event.ops.fion_insertfd(&obj->event, obj->fd);

                    if (obj->event.ops.fion_setflags(&obj->event) == false)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u event creation failed\n",
                                      __FUNCTION__,
                                      obj->id,
                                      errno);
                    }
                    // @todo - SO_REUSEPORT? SO_LINGER? etc
                    else if (setsockopt(obj->fd,
                                        SOL_SOCKET,
                                        SO_REUSEADDR,
                                        &optval,
                                        sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %u SO_REUSEADDR option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->id,
                                      errno);
                        sockobj_close(obj);
                    }
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
                                      obj->id,
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
                                      obj->id,
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
                                      obj->id,
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

/**
 * @see See header file for interface comments.
 */
bool sockobj_close(struct sockobj * const obj)
{
    int32_t ret = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (close(obj->fd) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %u could not be closed (%d)\n",
                          __FUNCTION__,
                          obj->id,
                          errno);
        }
        else if (sockobj_destroy(obj) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %u event desruction failed\n",
                          __FUNCTION__,
                          obj->id,
                          errno);
        }
        else
        {
            obj->info.stopusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                    UNIT_TIME_USEC);
            ret = true;
        }

        if (obj->alist != NULL)
        {
            freeaddrinfo(obj->alist);
            obj->alist = NULL;
        }

        obj->state = SOCKOBJ_STATE_CLOSE;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_bind(struct sockobj * const obj)
{
    bool ret = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (bind(obj->fd, obj->ainfo.ai_addr, obj->ainfo.ai_addrlen) == 0)
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
                      obj->id,
                      errno);
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_getopts(struct sockobj * const obj, struct vector * const opts)
{
    bool ret = false;
    int32_t errval = 0;
    struct sockobj_opt *opt = NULL;
    uint32_t i;

    if ((obj == NULL) || (opts == NULL) || (vector_getsize(opts) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
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
                              obj->id,
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
                                  obj->id,
                                  sockobj_getoptname(opt->name),
                                  errno);
                    ret = false;
                }
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_setopts(struct sockobj * const obj, struct vector * const opts)
{
    bool ret = false;
    int32_t err = 0;
    struct sockobj_opt *opt = NULL;
    uint32_t i;

    if ((obj == NULL) || (opts == NULL) || (vector_getsize(opts) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
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
                              obj->id,
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
                                  obj->id,
                                  sockobj_getoptname(opt->name),
                                  errno);
                    ret = false;
                }
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_setstats(struct sockobj_flowstats * const stats, const int32_t len)
{
    bool ret = false;
    uint64_t tsus = 0;

    if (stats == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (stats->lasttsus == 0)
        {
            stats->lasttsus = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                 UNIT_TIME_USEC);
        }

        if (len > 0)
        {
            stats->totalbytes += (uint32_t)len;

            if ((int64_t)len > stats->buflen.max)
            {
                stats->buflen.max = (int64_t)len;
            }

            if (((int64_t)len < stats->buflen.min) || (stats->passedcalls == 0))
            {
                stats->buflen.min = (int64_t)len;
            }

            stats->passedcalls++;

            stats->buflen.avg = stats->totalbytes / stats->passedcalls;

            if (stats->lastcall == false)
            {
                tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                          UNIT_TIME_USEC);

                stats->failedtsus += tsus - stats->lasttsus;
                stats->lasttsus = tsus;
                stats->lastcall = true;
            }
        }
        else
        {
            // @note A UDP send of 0 bytes could be considered a successful
            //       send.
            stats->failedcalls++;

            if (stats->lastcall == true)
            {
                tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                          UNIT_TIME_USEC);

                stats->passedtsus += tsus - stats->lasttsus;
                stats->lasttsus = tsus;
                stats->lastcall = false;
            }
        }

        ret = true;
    }

    return ret;
}
