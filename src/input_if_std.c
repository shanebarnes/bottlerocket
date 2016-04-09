/**
 * @file      input_if_std.c
 * @brief     Standard input stream interface implementation.
 * @author    Shane Barnes
 * @date      24 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "input_if_std.h"
#include "io_event_poll.h"
#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @see See header file for interface comments.
 */
int32_t input_if_std_recv(void * const buf,
                          const uint32_t len,
                          const int32_t timeoutms)
{
    int32_t retval = -1;
    int32_t fd = STDIN_FILENO;
    struct io_event_instance instance;

    if ((buf == NULL) || (len == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        instance.fds       = &fd;
        instance.size      = 1;
        instance.timeoutms = timeoutms;
        instance.pevents   = IO_EVENT_POLL_IN;
        instance.internal  = NULL;

        if (io_event_poll_create(&instance) == true)
        {
            if (io_event_poll_poll(&instance) ==  true)
            {
                if (instance.revents & IO_EVENT_RET_INREADY)
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
            }

            io_event_poll_destroy(&instance);
        }
    }

    return retval;
}
