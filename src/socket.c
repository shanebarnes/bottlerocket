/**
 * @file   socket.c
 * @author Shane Barnes
 * @date   03 Mar 2016
 * @brief  Network socket implementation.
 */

#include "logger.h"
#include "socket.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @see See header file for interface comments.
 */
bool socket_open(struct socket_instance * const instance)
{
    bool              retval   = false;
    int32_t           portsize = 0;
    struct addrinfo  *anext = NULL, ahints;
    char              ipport[6];

    if (instance != NULL)
    {
        memset(&ahints, 0, sizeof(struct addrinfo));
        ahints.ai_family    = AF_UNSPEC;          // IPv4 or IPv6
        ahints.ai_socktype  = instance->socktype;
        ahints.ai_flags     = AI_PASSIVE;         // All interfaces
        ahints.ai_protocol  = 0;
        ahints.ai_canonname = NULL;
        ahints.ai_addr      = NULL;
        ahints.ai_next      = NULL;

        portsize = snprintf(ipport, 6, "%d", instance->addrself.ipport);

        instance->alist = NULL;

        if ((portsize > 0) && (portsize < 6) &&
            (getaddrinfo(instance->addrself.ipaddr,
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
                    retval = true;
                    instance->listenfd                = instance->sockfd;
                    instance->ainfo                   = *anext;

                    instance->addrself.sockaddr.sin_family       = anext->ai_family;
                    instance->addrself.sockaddr.sin_addr.s_addr  = inet_addr(instance->addrself.ipaddr);
                    instance->addrself.sockaddr.sin_port         = htons(instance->addrself.ipport);

                    instance->addrpeer.sockaddr.sin_family      = anext->ai_family;
                    instance->addrpeer.sockaddr.sin_addr.s_addr = inet_addr(instance->addrself.ipaddr);
                    instance->addrpeer.sockaddr.sin_port        = htons(instance->addrself.ipport);

                    //info->timeConnectUsec            = 0;
                    //info->timeIdleRecvUsec           = 0;
                    //info->timeIdleSendUsec           = 0;

                    // @todo - SO_REUSEPORT? SO_LINGER?
                    //if ((retval = socketOptionSet(info->fdSocket, SOL_SOCKET, SO_REUSEADDR, 1)) == false)
                    //{
                    //    perror(__FUNCTION__);
                    //}
#if defined(__APPLE__)
                    //if ((retval = socketOptionSet(info->fdSocket, SOL_SOCKET, SO_NOSIGPIPE, 1)) == false)
                    //{
                    //  perror(__FUNCTION__);
                    //}
#endif
                    if (retval == false)
                    {
                        socket_close(instance);
                    }

                    break;
                }
            }
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_close(struct socket_instance * const instance)
{
    int32_t retval = false;

    if (instance != NULL)
    {
        if (close(instance->sockfd) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %d could not be closed\n",
                          __FUNCTION__,
                          instance->sockfd);
        }

        close(instance->listenfd);

        if (instance->alist != NULL)
        {
            freeaddrinfo(instance->alist);
            instance->alist = NULL;
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_bind(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (bind(instance->sockfd,
                 instance->ainfo.ai_addr,
                 instance->ainfo.ai_addrlen) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s; bind failed\n",
                          __FUNCTION__);
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}
