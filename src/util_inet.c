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
 * @see See header file for interace comments.
 */
bool utilinet_isipv4(const char * const addr)
{
    bool retval = false;
    int32_t error;
    struct sockaddr_in ipv4;

    if (addr == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        error = inet_pton(AF_INET, addr, &ipv4);

        if (error == 1)
        {
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool utilinet_isipv6(const char * const addr)
{
    bool retval = false;
    int32_t error;
    struct sockaddr_in6 ipv6;

    if (addr == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        error = inet_pton(AF_INET6, addr, &ipv6);

        if (error == 1)
        {
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool utilinet_getaddrfromhost(const char * const hostname, char * const addr)
{
    bool retval = false;
    struct addrinfo hints, *result, *rp;
    struct sockaddr_in *saddr;

    if ((hostname == NULL) || (addr == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family    = AF_UNSPEC;
        hints.ai_socktype  = SOCK_STREAM;
        hints.ai_flags     = AI_PASSIVE;
        hints.ai_protocol  = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr      = NULL;
        hints.ai_next      = NULL;

        if (getaddrinfo(hostname, NULL, &hints, &result) == 0)
        {
            for (rp = result; rp != NULL; rp = rp->ai_next)
            {
                saddr = (struct sockaddr_in *)rp->ai_addr;
                strncpy(addr, inet_ntoa(saddr->sin_addr), strlen(addr));
                retval = true;
                break;
            }

            freeaddrinfo(result);
        }
    }

    return retval;
}
