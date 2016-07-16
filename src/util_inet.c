/**
 * @file      util_inet.c
 * @brief     IP netwrk utility implementation.
 * @author    Shane Barnes
 * @date      01 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_inet.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

/**
 * @see See header file for interface comments.
 */
bool utilinet_isipv4(const char * const addr)
{
    bool ret = false;
    int32_t err;
    struct sockaddr_in ipv4;

    if (addr == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        err = inet_pton(AF_INET, addr, &ipv4);

        if (err == 1)
        {
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool utilinet_isipv6(const char * const addr)
{
    bool ret = false;
    int32_t err;
    struct sockaddr_in6 ipv6;

    if (addr == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        err = inet_pton(AF_INET6, addr, &ipv6);

        if (err == 1)
        {
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool utilinet_getaddrfromhost(const char * const host,
                              const int32_t family,
                              char * const addr,
                              const uint32_t len)
{
    bool ret = false;
    struct addrinfo hints, *res, *rp;
    struct sockaddr_in  *ipv4;
    struct sockaddr_in6 *ipv6;

    if ((host == NULL) || (addr == NULL) || (len == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family    = family;
        hints.ai_socktype  = SOCK_STREAM;
        hints.ai_flags     = AI_PASSIVE;
        hints.ai_protocol  = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr      = NULL;
        hints.ai_next      = NULL;

        if (getaddrinfo(host, NULL, &hints, &res) == 0)
        {
            for (rp = res; rp != NULL; rp = rp->ai_next)
            {
                if (rp->ai_family == AF_INET)
                {
                    ipv4 = (struct sockaddr_in*)rp->ai_addr;
                    inet_ntop(AF_INET,
                              &ipv4->sin_addr,
                              addr,
                              len);
                    ret = true;
                    break;
                }
                else if (rp->ai_family == AF_INET6)
                {
                    ipv6 = (struct sockaddr_in6*)rp->ai_addr;
                    inet_ntop(AF_INET6,
                              &ipv6->sin6_addr,
                              addr,
                              len);
                    ret = true;
                    break;
                }
            }

            freeaddrinfo(res);
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
void *utilinet_getaddrfromstorage(const struct sockaddr_storage * const addr)
{
    void *ret = NULL;

    if (addr == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        switch (addr->ss_family)
        {
            case AF_INET:
                ret = &(((struct sockaddr_in*)addr)->sin_addr.s_addr);
                break;
            case AF_INET6:
                ret = &(((struct sockaddr_in6*)addr)->sin6_addr.s6_addr);
                break;
            default:
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: invalid family (%d)\n",
                              __FUNCTION__,
                              addr->ss_family);
                break;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
uint16_t *utilinet_getportfromstorage(const struct sockaddr_storage * const addr)
{
    uint16_t *ret = NULL;

    if (addr == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        switch (addr->ss_family)
        {
            case AF_INET:
                ret = &(((struct sockaddr_in*)addr)->sin_port);
                break;
            case AF_INET6:
                ret = &(((struct sockaddr_in6*)addr)->sin6_port);
                break;
            default:
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: invalid family (%d)\n",
                              __FUNCTION__,
                              addr->ss_family);
                break;
        }
    }

    return ret;
}
