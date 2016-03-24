/**
 * @file   util_ioctl.c
 * @author Shane Barnes
 * @date   24 Mar 2016
 * @brief  I/O control utility implementation.
 */

#include "logger.h"
#include "util_ioctl.h"

#include <errno.h>
#include <sys/ioctl.h>

/**
 * @see See header file for interace comments.
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
