/**
 * @file      util_stats.c
 * @brief     Statistics utility implementation.
 * @author    Shane Barnes
 * @date      27 Dec 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "util_date.h"
#include "util_debug.h"
#include "util_stats.h"

bool utilstats_add(struct utilstats_qty * const stats, const int64_t val)
{
    bool ret = false;
    uint64_t tvus = 0;

    if (UTILDEBUG_VERIFY(stats != NULL))
    {
        tvus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
        stats->cnt++;
        stats->sum += val;

        if (stats->cnt != 0)
        {
            stats->avg = stats->sum / stats->cnt;
        }

        if (val > stats->max)
        {
            stats->max = val;
        }

        if (val < stats->min)
        {
            stats->min = val;
        }

        if (stats->tv0 == 0)
        {
            stats->tv0 = tvus;
        }

        stats->tvn = tvus;

        ret = true;
    }

    return ret;
}
