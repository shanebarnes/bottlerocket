/**
 * @file      util_cpu.h
 * @brief     CPU utility interface.
 * @author    Shane Barnes
 * @date      01 Aug 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_CPU_H_
#define _UTIL_CPU_H_

#include "system_types.h"

#include <sys/time.h>

struct utilcpu_info
{
    int16_t        usage;
    uint64_t       startusec;
    struct timeval realtime;
    struct timeval systime;
    struct timeval usrtime;
};

/**
 * @brief Get the current thread CPU information.
 *
 * @param[in,out] info A pointer to a CPU information struture.
 *
 * @return True if the current thread CPU informationw as retrieved..
 */
bool utilcpu_getinfo(struct utilcpu_info * const info);

#endif // _UTIL_CPU_H_
