/**
 * @file      util_cpu.c
 * @brief     System control utility implementation.
 * @author    Shane Barnes
 * @date      01 Aug 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_cpu.h"

#include <errno.h>

#if defined(__APPLE__)
   #include <mach/mach_init.h>
   #include <mach/mach_port.h>
   #include <mach/thread_act.h>
#endif

/**
 * @see See header file for interface comments.
 */
bool utilcpu_getinfo(struct utilcpu_info * const info)
{
    bool ret = false;
#if defined(__APPLE__)
    kern_return_t            err    = KERN_FAILURE;
    mach_port_t              thread = mach_thread_self();
    mach_msg_type_number_t   count  = THREAD_BASIC_INFO_COUNT;
    thread_basic_info_data_t data;
#endif
    if (info == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
#if defined(__APPLE__)
        err = thread_info(thread,
                          THREAD_BASIC_INFO,
                          (thread_info_t)&data,
                          &count);

        if (err != KERN_SUCCESS)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to get thread information (%d)\n",
                          __FUNCTION__,
                          errno);
            info->load = -1;
        }
        else
        {
            info->load = data.cpu_usage / 10;
            ret = true;
        }

        mach_port_deallocate(mach_task_self(), thread);
#endif
    }

    return ret;
}
