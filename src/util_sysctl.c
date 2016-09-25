/**
 * @file      util_sysctl.c
 * @brief     System control utility implementation.
 * @author    Shane Barnes
 * @date      11 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_sysctl.h"

#if defined(__APPLE__)
    #include <errno.h>
    #include <netinet/in.h>
    #include <netinet/ip_var.h>
    #include <netinet/udp.h>
    #include <netinet/udp_var.h>
    #include <sys/sysctl.h>
#endif
#include <unistd.h>

/**
 * @see See header file for interface comments.
 */
uint32_t utilsysctl_getcpusavail(void)
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

/**
 * @see See header file for interface comments.
 */
int32_t utilsysctl_getmaxudpsize(void)
{
    int32_t ret = -1;
#if defined(__APPLE__)
    size_t len = sizeof(ret);
    int32_t mib[] = { CTL_NET, PF_INET, IPPROTO_UDP, UDPCTL_MAXDGRAM };

    if (sysctl(mib,
               sizeof(mib) / sizeof(int32_t),
               &ret,
               &len,
               NULL,
               0) == -1)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to get maximum udp size (%d)\n",
                      __FUNCTION__,
                      errno);
    }
#endif
    return ret;
}

/**
 * @see See header file for interface comments.
 */
int32_t utilsysctl_getmaxsockbufsize(void)
{
    int32_t ret = -1;
#if defined(__APPLE__)
    size_t len = sizeof(ret);
    int32_t mib[] = { CTL_KERN, KERN_IPC, KIPC_MAXSOCKBUF };

    if (sysctl(mib,
               sizeof(mib) / sizeof(int32_t),
               &ret,
               &len,
               NULL,
               0) == -1)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to get maximum socket buffer size (%d)\n",
                      __FUNCTION__,
                      errno);
    }
#endif
    return ret;
}

