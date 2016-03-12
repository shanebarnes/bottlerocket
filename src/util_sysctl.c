/**
 * @file   util_sysctl.c
 * @author Shane Barnes
 * @date   11 Mar 2016
 * @brief  System control utility implementation.
 */

#include "util_sysctl.h"

#if defined(__APPLE__)
    #include <sys/sysctl.h>
#elif defined(LINUX)
    #include <unistd.h>
#endif

/**
 * @see See header file for interace comments.
 */
uint32_t util_sysctl_cpuavail(void)
{
    uint32_t retval = 0;
#if defined(__APPLE__)
    size_t i = 0, len = 4;
    int32_t hwcpu[2] = { HW_AVAILCPU, HW_NCPU };
    int32_t nm[2];

    for (i = 0; i < 2; i++)
    {
        nm[0] = CTL_HW;
        nm[1] = hwcpu[i];

        if (sysctl(nm, 2, &retval, &len, NULL, 0) == 0)
        {
            break;
        }
    }
#elif defined(LINUX)
    retval = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    return retval;
}
