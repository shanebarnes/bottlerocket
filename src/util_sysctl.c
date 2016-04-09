/**
 * @file      util_sysctl.c
 * @brief     System control utility implementation.
 * @author    Shane Barnes
 * @date      11 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "util_sysctl.h"

#if defined(__APPLE__)
    #include <sys/sysctl.h>
#elif defined(LINUX)
    #include <unistd.h>
#endif

/**
 * @see See header file for interface comments.
 */
uint32_t utilsysctl_getcpusavail(void)
{
    uint32_t retval = 0;
#if defined(__APPLE__)
    size_t i = 0, len = 4;
    int32_t com[2] = { HW_AVAILCPU, HW_NCPU };
    int32_t mib[2];

    for (i = 0; i < 2; i++)
    {
        mib[0] = CTL_HW;
        mib[1] = com[i];

        if (sysctl(mib, 2, &retval, &len, NULL, 0) == 0)
        {
            break;
        }
    }
#elif defined(LINUX)
    retval = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    return retval;
}
