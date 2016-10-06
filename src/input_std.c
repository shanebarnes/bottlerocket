/**
 * @file      input_std.c
 * @brief     Standard input stream implementation.
 * @author    Shane Barnes
 * @date      24 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "input_std.h"
#include "logger.h"
#include "util_debug.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int32_t inputstd_recv(void * const buf,
                      const uint32_t len,
                      const int32_t timeoutms)
{
    int32_t retval = -1;

    if (UTILDEBUG_VERIFY((buf != NULL) && (len > 0) && (timeoutms == 0)))
    {
        if (fgets((char*)buf, len, stdin) == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to receive bytes from (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            retval = (int32_t)strlen((char*)buf);

            if (retval > 0)
            {
                if (*((char*)buf + retval - 1) == '\n')
                {
                    *((char*)buf + retval - 1) = '\0';
                    retval--;
                }
            }
        }
    }

    return retval;
}
