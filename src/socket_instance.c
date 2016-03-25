/**
 * @file      socket_instance.c
 * @brief     Network socket instance implementation.
 * @author    Shane Barnes
 * @date      03 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "io_event_poll.h"
#include "logger.h"
#include "socket_instance.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//#include <sys/ioctl.h>
//static int32_t socket_instance_availbytes(struct socket_instance * const instance)
//{
//    int32_t retval = 0;
//
//    if (instance != NULL)
//    {
//        if ((ioctl(instance->sockfd, FIONREAD, &retval) != -1) && (retval > 0))
//        {
//            logger_printf(LOGGER_LEVEL_TRACE,
//                          "%s: socket %d has %d bytes available\n",
//                          __FUNCTION__,
//                          instance->sockfd,
//                          retval);
//        }
//    }
//
//    return retval;
//}

/**
 * @see See header file for interface comments.
 */
bool socket_instance_getaddrpeer(struct socket_instance * const instance)
{
    bool      retval  = false;
    socklen_t socklen = 0;

    if ((instance == NULL) ||
        ((socklen = sizeof(instance->addrpeer.sockaddr)) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (getpeername(instance->sockfd,
                         (struct sockaddr *)&(instance->addrpeer.sockaddr),
                         &socklen) == 0)
    {
        inet_ntop(AF_INET,
                  &(instance->addrpeer.sockaddr.sin_addr),
                  instance->addrpeer.ipaddr,
                  sizeof(instance->addrpeer.ipaddr));

        instance->addrpeer.ipport = ntohs(instance->addrpeer.sockaddr.sin_port);

        snprintf(instance->addrpeer.sockaddrstr,
                 sizeof(instance->addrpeer.sockaddrstr),
                 "%s:%u",
                 instance->addrpeer.ipaddr,
                 instance->addrpeer.ipport);

        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %d getpeername failed (%d)\n",
                      __FUNCTION__,
                      instance->sockfd,
                      errno);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_instance_getaddrself(struct socket_instance * const instance)
{
    bool      retval  = false;
    socklen_t socklen = 0;

    if ((instance == NULL) ||
        ((socklen = sizeof(instance->addrself.sockaddr)) == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (getsockname(instance->sockfd,
                         (struct sockaddr *)&(instance->addrself.sockaddr),
                         &socklen) == 0)
    {
        inet_ntop(AF_INET,
                  &(instance->addrself.sockaddr.sin_addr),
                  instance->addrself.ipaddr,
                  sizeof(instance->addrself.ipaddr));

        instance->addrself.ipport = ntohs(instance->addrself.sockaddr.sin_port);

        snprintf(instance->addrself.sockaddrstr,
                 sizeof(instance->addrself.sockaddrstr),
                 "%s:%u",
                 instance->addrself.ipaddr,
                 instance->addrself.ipport);

        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %d getsockname failed (%d)\n",
                      __FUNCTION__,
                      instance->sockfd,
                      errno);
    }

    return retval;
}

/**
 * @see See sio_create() for interface comments.
 */
bool socket_instance_create(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        instance->event.fds       = &instance->sockfd;
        instance->event.size      = 1;
        instance->event.timeoutms = 0;
        instance->event.pevents   = IO_EVENT_POLL_IN;
        instance->event.internal  = NULL;

        instance->event.ops.ieo_create   = io_event_poll_create;
        instance->event.ops.ieo_destroy  = io_event_poll_destroy;
        instance->event.ops.ieo_setflags = io_event_poll_setflags;
        instance->event.ops.ieo_poll     = io_event_poll_poll;

        retval = instance->event.ops.ieo_create(&instance->event);
    }

    return retval;
}

/**
 * @see See sio_destroy() for interface comments.
 */
bool socket_instance_destroy(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = instance->event.ops.ieo_destroy(&instance->event);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_instance_open(struct socket_instance * const instance)
{
    bool              retval   = false;
    int32_t           portsize = 0;
    struct addrinfo  *anext = NULL, ahints;
    socklen_t         optval;
    char              ipport[6];

    if (instance == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        memset(&ahints, 0, sizeof(struct addrinfo));
        ahints.ai_family    = AF_UNSPEC;          // IPv4 or IPv6
        ahints.ai_socktype  = instance->socktype;
        ahints.ai_flags     = AI_PASSIVE;         // All interfaces
        ahints.ai_protocol  = 0;
        ahints.ai_canonname = NULL;
        ahints.ai_addr      = NULL;
        ahints.ai_next      = NULL;

        portsize = snprintf(ipport, 6, "%d", instance->ipport);

        instance->alist = NULL;

        if ((portsize > 0) &&
            (portsize < 6) &&
            (getaddrinfo(instance->ipaddr,
                         ipport,
                         &ahints,
                         &(instance->alist)) == 0))
        {
            for (anext = instance->alist; anext != NULL; anext = anext->ai_next)
            {
                if ((instance->sockfd = socket(anext->ai_family,
                                               anext->ai_socktype,
                                               anext->ai_protocol)) != 0)
                {
                    instance->ainfo = *anext;

                    instance->addrself.sockaddr.sin_family      = anext->ai_family;
                    instance->addrself.sockaddr.sin_addr.s_addr = inet_addr(instance->ipaddr);
                    instance->addrself.sockaddr.sin_port        = htons(instance->ipport);

                    instance->addrpeer.sockaddr.sin_family      = anext->ai_family;
                    instance->addrpeer.sockaddr.sin_addr.s_addr = inet_addr(instance->ipaddr);
                    instance->addrpeer.sockaddr.sin_port        = htons(instance->ipport);

                    optval = 1;

                    if (socket_instance_create(instance) == false)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %d event creation failed\n",
                                      __FUNCTION__,
                                      instance->sockfd,
                                      errno);
                    }
                    // @todo - SO_REUSEPORT? SO_LINGER? SO_SNDBUF? SO_RCVBUF? etc
                    else if (setsockopt(instance->sockfd,
                                        SOL_SOCKET,
                                        SO_REUSEADDR,
                                        &optval,
                                        sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %d SO_REUSEADDR option failed (%d)\n",
                                      __FUNCTION__,
                                      instance->sockfd,
                                      errno);
                        socket_instance_close(instance);
                    }
#if defined(__APPLE__)
                    else if (setsockopt(instance->sockfd,
                             SOL_SOCKET,
                             SO_NOSIGPIPE,
                             &optval,
                             sizeof(optval)) != 0)
                    {
                        logger_printf(LOGGER_LEVEL_ERROR,
                                      "%s: socket %d SO_NOSIGPIPE option failed (%d)\n",
                                      __FUNCTION__,
                                      instance->sockfd,
                                      errno);
                        socket_instance_close(instance);
                    }
#endif
                    else
                    {
                        instance->state = SOCKET_STATE_OPEN;
                        retval = true;
                    }

                    break;
                }
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_instance_close(struct socket_instance * const instance)
{
    int32_t retval = false;

    if (instance == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (close(instance->sockfd) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %d could not be closed (%d)\n",
                          __FUNCTION__,
                          instance->sockfd,
                          errno);
        }
        else if (socket_instance_destroy(instance) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %d event desruction failed\n",
                          __FUNCTION__,
                          instance->sockfd,
                          errno);
        }
        else
        {
            retval = true;
        }

        if (instance->alist != NULL)
        {
            freeaddrinfo(instance->alist);
            instance->alist = NULL;
        }

        instance->state = SOCKET_STATE_CLOSE;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_instance_bind(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (bind(instance->sockfd,
                  instance->ainfo.ai_addr,
                  instance->ainfo.ai_addrlen) == 0)
    {
        socket_instance_getaddrself(instance);
        instance->state |= SOCKET_STATE_BIND;
        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %d bind failed (%d)\n",
                      __FUNCTION__,
                      instance->sockfd,
                      errno);
    }

    return retval;
}
