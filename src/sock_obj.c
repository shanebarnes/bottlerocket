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
    char *retval = NULL;

    switch (name)
    {
        case SO_REUSEADDR:
            retval = "SO_REUSEADDR";
            break;
        case SO_KEEPALIVE:
            retval = "SO_KEEPALIVE";
            break;
        case SO_LINGER:
            retval = "SO_LINGER";
            break;
        case SO_REUSEPORT:
            retval = "SO_REUSEPORT";
            break;
#if defined(__APPLE__)
        case SO_NOSIGPIPE:
            retval = "SO_NOSIGPIPE";
            break;
#endif
        default:
            retval = "";
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_getaddrpeer(struct sockobj * const obj)
{
    bool      retval  = false;
    socklen_t socklen = 0;

    if ((obj == NULL) || ((socklen = sizeof(obj->addrpeer.sockaddr)) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (getpeername(obj->sockfd,
                         (struct sockaddr *)&(obj->addrpeer.sockaddr),
                         &socklen) == 0)
    {
        inet_ntop(AF_INET,
                  &(obj->addrpeer.sockaddr.sin_addr),
                  obj->addrpeer.ipaddr,
                  sizeof(obj->addrpeer.ipaddr));

        obj->addrpeer.ipport = ntohs(obj->addrpeer.sockaddr.sin_port);

        snprintf(obj->addrpeer.sockaddrstr,
                 sizeof(obj->addrpeer.sockaddrstr),
                 "%s:%u",
                 obj->addrpeer.ipaddr,
                 obj->addrpeer.ipport);

        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %d getpeername failed (%d)\n",
                      __FUNCTION__,
                      obj->sockfd,
                      errno);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_getaddrself(struct sockobj * const obj)
{
    bool      retval  = false;
    socklen_t socklen = 0;

    if ((obj == NULL) || ((socklen = sizeof(obj->addrself.sockaddr)) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (getsockname(obj->sockfd,
                         (struct sockaddr *)&(obj->addrself.sockaddr),
                         &socklen) == 0)
    {
        inet_ntop(AF_INET,
                  &(obj->addrself.sockaddr.sin_addr),
                  obj->addrself.ipaddr,
                  sizeof(obj->addrself.ipaddr));

        obj->addrself.ipport = ntohs(obj->addrself.sockaddr.sin_port);

        snprintf(obj->addrself.sockaddrstr,
                 sizeof(obj->addrself.sockaddrstr),
                 "%s:%u",
                 obj->addrself.ipaddr,
                 obj->addrself.ipport);

        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %d getsockname failed (%d)\n",
                      __FUNCTION__,
                      obj->sockfd,
                      errno);
    }

    return retval;
}

/**
 * @see See sock_create() for interface comments.
 */
bool sockobj_create(struct sockobj * const obj)
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
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See sock_destroy() for interface comments.
 */
bool sockobj_destroy(struct sockobj * const obj)
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
        retval = obj->event.ops.fion_destroy(&obj->event);

        obj->ops.sock_create  = NULL;
        obj->ops.sock_destroy = NULL;
        obj->ops.sock_open    = NULL;
        obj->ops.sock_close   = NULL;
        obj->ops.sock_bind    = NULL;
        obj->ops.sock_getopts = NULL;
        obj->ops.sock_setopts = NULL;
        obj->ops.sock_listen  = NULL;
        obj->ops.sock_accept  = NULL;
        obj->ops.sock_connect = NULL;
        obj->ops.sock_recv    = NULL;
        obj->ops.sock_send    = NULL;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_open(struct sockobj * const obj)
{
    bool             retval   = false;
    int32_t          portsize = 0;
    struct addrinfo *anext    = NULL, ahints;
    socklen_t        optval;
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
        ahints.ai_family    = AF_UNSPEC;     // IPv4 or IPv6
        ahints.ai_socktype  = obj->conf.type;
        ahints.ai_flags     = AI_PASSIVE;    // All interfaces
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
                if ((obj->sockfd = socket(anext->ai_family,
                                          anext->ai_socktype,
                                          anext->ai_protocol)) != -1)
                {
                    obj->ainfo = *anext;

                    obj->addrself.sockaddr.sin_family      = anext->ai_family;
                    obj->addrself.sockaddr.sin_addr.s_addr = inet_addr(obj->conf.ipaddr);
                    obj->addrself.sockaddr.sin_port        = htons(obj->conf.ipport);

                    obj->addrpeer.sockaddr.sin_family      = anext->ai_family;
                    obj->addrpeer.sockaddr.sin_addr.s_addr = inet_addr(obj->conf.ipaddr);
                    obj->addrpeer.sockaddr.sin_port        = htons(obj->conf.ipport);

                    optval = 1;
                    obj->event.ops.fion_insertfd(&obj->event, obj->sockfd);

                    if (obj->event.ops.fion_setflags(&obj->event) == false)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %d event creation failed\n",
                                      __FUNCTION__,
                                      obj->sockfd,
                                      errno);
                    }
                    // @todo - SO_REUSEPORT? SO_LINGER? SO_SNDBUF? SO_RCVBUF? etc
                    else if (setsockopt(obj->sockfd,
                                        SOL_SOCKET,
                                        SO_REUSEADDR,
                                        &optval,
                                        sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %d SO_REUSEADDR option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sockfd,
                                      errno);
                        sockobj_close(obj);
                    }
#if defined(__APPLE__)
                    else if (setsockopt(obj->sockfd,
                             SOL_SOCKET,
                             SO_NOSIGPIPE,
                             &optval,
                             sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %d SO_NOSIGPIPE option failed (%d)\n",
                                      __FUNCTION__,
                                      obj->sockfd,
                                      errno);
                        sockobj_close(obj);
                    }
#endif
                    else
                    {
                        obj->state = SOCKOBJ_STATE_OPEN;
                        retval = true;
                    }

                    break;
                }
            }
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to get address information\n",
                          __FUNCTION__);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_close(struct sockobj * const obj)
{
    int32_t retval = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (close(obj->sockfd) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %d could not be closed (%d)\n",
                          __FUNCTION__,
                          obj->sockfd,
                          errno);
        }
        else if (sockobj_destroy(obj) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %d event desruction failed\n",
                          __FUNCTION__,
                          obj->sockfd,
                          errno);
        }
        else
        {
            obj->info.stopusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                    UNIT_TIME_USEC);
            retval = true;
        }

        if (obj->alist != NULL)
        {
            freeaddrinfo(obj->alist);
            obj->alist = NULL;
        }

        obj->state = SOCKOBJ_STATE_CLOSE;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_bind(struct sockobj * const obj)
{
    bool retval = false;

    // @todo Support binding to Ethernet interfaces (e.g., "eth0").

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (bind(obj->sockfd,
                  obj->ainfo.ai_addr,
                  obj->ainfo.ai_addrlen) == 0)
    {
        sockobj_getaddrself(obj);
        obj->state |= SOCKOBJ_STATE_BIND;
        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %d bind failed (%d)\n",
                      __FUNCTION__,
                      obj->sockfd,
                      errno);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_getopts(struct sockobj * const obj, struct vector * const opts)
{
    bool retval = false;
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
        retval = true;

        for (i = 0; i < vector_getsize(opts); i++)
        {
            opt = (struct sockobj_opt *)vector_getval(opts, i);

            if (opt == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d option index %u is empty\n",
                              __FUNCTION__,
                              obj->sockfd,
                              i);
                retval = false;
            }
            else
            {
                errval = getsockopt(obj->sockfd,
                                    opt->level,
                                    opt->name,
                                    &opt->val,
                                    &opt->len);

                if (errval != 0)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d '%s' option failed (%d)\n",
                                  __FUNCTION__,
                                  obj->sockfd,
                                  sockobj_getoptname(opt->name),
                                  errno);
                    retval = false;
                }
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockobj_setopts(struct sockobj * const obj, struct vector * const opts)
{
    bool retval = false;
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
        retval = true;

        for (i = 0; i < vector_getsize(opts); i++)
        {
            opt = (struct sockobj_opt *)vector_getval(opts, i);

            if (opt == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d option index %u is empty\n",
                              __FUNCTION__,
                              obj->sockfd,
                              i);
                retval = false;
            }
            else
            {
                errval = setsockopt(obj->sockfd,
                                    opt->level,
                                    opt->name,
                                    &opt->val,
                                    sizeof(opt->val));

                if (errval != 0)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: socket %d '%s' option failed (%d)\n",
                                  __FUNCTION__,
                                  obj->sockfd,
                                  sockobj_getoptname(opt->name),
                                  errno);
                    retval = false;
                }
            }
        }
    }

    return retval;
}
