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

#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

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

/**
 * @see See header file for interace comments.
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
