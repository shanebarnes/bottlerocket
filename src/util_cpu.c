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
#include "util_unit.h"

#include <errno.h>

#if defined(__APPLE__)
   #include <mach/mach_init.h>
   #include <mach/mach_port.h>
   #include <mach/thread_act.h>
#elif defined(__linux__)
    #include <sys/time.h>
    #include <sys/resource.h>
#endif

#if defined(__linux__)
/**
 * @brief Calculate CPU usage.
 *
 * @param[in] info A pointer to a CPU information struture.
 *
 * @return The calculated CPU usage as a percentage (-1 on error).
 */
static uint16_t utilcpu_calcusage(struct utilcpu_info * const info)
{
    uint16_t ret = -1;
    uint64_t realtimeusec = (uint64_t)(info->realtime.tv_sec) * UNIT_TIME_USEC +
                            (uint64_t)(info->realtime.tv_usec);

    if (realtimeusec > 0)
    {
        ret = ((uint64_t)(info->systime.tv_sec) * UNIT_TIME_USEC +
               (uint64_t)(info->systime.tv_usec) +
               (uint64_t)(info->usrtime.tv_sec) * UNIT_TIME_USEC +
               (uint64_t)(info->usrtime.tv_usec)) * 100 / realtimeusec;
    }

    return ret;
}
#endif

bool utilcpu_getinfo(struct utilcpu_info * const info)
{
    bool ret = false;
#if defined(__APPLE__)
    mach_port_t              thread = mach_thread_self();
    mach_msg_type_number_t   count  = THREAD_BASIC_INFO_COUNT;
    thread_basic_info_data_t data;
#elif defined(__linux__)
    struct rusage data;
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
        if (thread_info(thread,
                        THREAD_BASIC_INFO,
                        (thread_info_t)&data,
                        &count) != KERN_SUCCESS)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to get thread information (%d)\n",
                          __FUNCTION__,
                          errno);
            info->usage           = -1;
            info->systime.tv_sec  = 0;
            info->systime.tv_usec = 0;
            info->usrtime.tv_sec  = 0;
            info->usrtime.tv_usec = 0;
        }
        else
        {
            info->usage           = data.cpu_usage / 10;
            info->systime.tv_sec  = data.system_time.seconds;
            info->systime.tv_usec = data.system_time.microseconds;
            info->usrtime.tv_sec  = data.user_time.seconds;
            info->usrtime.tv_usec = data.user_time.microseconds;
            ret = true;
        }

        mach_port_deallocate(mach_task_self(), thread);
#elif defined(__linux__)
        if (getrusage(RUSAGE_THREAD, &data) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to get thread information (%d)\n",
                          __FUNCTION__,
                          errno);
            info->usage           = -1;
            info->systime.tv_sec  = 0;
            info->systime.tv_usec = 0;
            info->usrtime.tv_sec  = 0;
            info->usrtime.tv_usec = 0;
        }
        else
        {
            info->systime.tv_sec  = data.ru_stime.tv_sec;
            info->systime.tv_usec = data.ru_stime.tv_usec;
            info->usrtime.tv_sec  = data.ru_utime.tv_sec;
            info->usrtime.tv_usec = data.ru_utime.tv_usec;
            info->usage           = utilcpu_calcusage(info);
            ret = true;
        }
#endif
    }

    return ret;
}
