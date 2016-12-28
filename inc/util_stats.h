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
    int64_t  avg; // Average of all sample values
    uint64_t cnt; // Sample count
    int64_t  max; // Maximum of all sample values
    int64_t  min; // Minimum of all sample values
    int64_t  sum; // Sum of all sample values
    uint64_t tv0; // Unix time value of first sample
    uint64_t tvn; // Unix time value of last sample
};

/**
 * @brief Add a data sample to a statistical quantities structure.
 *
 * @param[in,out] stats A pointer to a statistical quantities data structure.
 * @param[in]     val   A data sample value.
 *
 * @return True if a data sample was added to a statistical quantities
 *         structure.
 */
bool utilstats_add(struct utilstats_qty * const stats, const int64_t val);

#endif // _UTIL_STATS_H_
