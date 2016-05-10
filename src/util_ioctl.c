/**
 * @file      util_ioctl.c
 * @brief     I/O control utility implementation.
 * @author    Shane Barnes
 * @date      24 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_ioctl.h"

#include <ifaddrs.h>
#include <errno.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @see See header file for interface comments.
 */
int32_t utilioctl_getbytesavail(const int32_t fd)
{
    int32_t retval = 0;

    if (ioctl(fd, FIONREAD, &retval) == -1)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: ioctl request failed (%d)\n",
                      __FUNCTION__,
                      errno);
        retval = -1;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t utilioctl_getifmtu(char * const ifname)
{
    int32_t retval = -1, fd = -1;
    struct ifreq ifr;

    if (ifname == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to create socket (%d)\n",
                      __FUNCTION__,
                      errno);
    }
    else
    {
        memset(&ifr, 0, sizeof(struct ifreq));
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

        if (ioctl(fd, SIOCGIFMTU, &ifr) == 0)
        {
            retval = ifr.ifr_mtu;
        }

        close(fd);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t utilioctl_getifmaxmtu(void)
{
    int32_t retval = -1, mtu = -1;
    struct ifaddrs *ifaddr = NULL, *ifa = NULL;

    if (getifaddrs(&ifaddr) != 0)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to get network interface list (%d)\n",
                      __FUNCTION__,
                      errno);
    }
    else
    {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if ((ifa->ifa_addr != NULL) &&
                ((ifa->ifa_addr->sa_family == AF_INET) ||
                 (ifa->ifa_addr->sa_family == AF_INET6)))
            {
                if ((mtu = utilioctl_getifmtu(ifa->ifa_name)) > -1)
                {
                    if (mtu > retval)
                    {
                        retval = mtu;
                    }
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t utilioctl_getifminmtu(void)
{
    int32_t retval = -1, mtu = -1;
    struct ifaddrs *ifaddr = NULL, *ifa = NULL;

    if (getifaddrs(&ifaddr) != 0)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to get network interface list (%d)\n",
                      __FUNCTION__,
                      errno);
    }
    else
    {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if ((ifa->ifa_addr != NULL) &&
                ((ifa->ifa_addr->sa_family == AF_INET) ||
                 (ifa->ifa_addr->sa_family == AF_INET6)))
            {
                if ((mtu = utilioctl_getifmtu(ifa->ifa_name)) > -1)
                {
                    if ((retval == -1) || (mtu < retval))
                    {
                        retval = mtu;
                    }
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool utilioctl_gettermsize(uint16_t * const rows, uint16_t * const cols)
{
    int32_t retval = 0;
    struct winsize win;

    if ((rows == NULL) || (cols == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: ioctl request failed (%d)\n",
                      __FUNCTION__,
                      errno);
        *rows = 0;
        *cols = 0;
    }
    else
    {
        *rows = win.ws_row;
        *cols = win.ws_col;
        retval = true;
    }

    return retval;
}
