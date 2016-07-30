/**
 * @file      util_stats.h
 * @brief     Statistics utility interface.
 * @author    Shane Barnes
 * @date      30 Jul 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_STATS_H_
#define _UTIL_STATS_H_

#include "system_types.h"

struct utilstats_qty
{
    int64_t avg;
    int64_t max;
    int64_t min;
};

#endif // _UTIL_STATS_H_
