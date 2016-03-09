/**
 * @file   manip_date.c
 * @author Shane Barnes
 * @date   08 Mar 2016
 * @brief  Date/time manipulation implementation.
 */

#include "manip_date.h"
#if defined(__APPLE__)
    #include <mach/mach_time.h>
#endif
#include <time.h>

/**
 * @brief Get the monotonic time since some (unspecified) starting point.
 *
 * @param[in] ts A timespec structure to set with the current monotonic time.
 *
 * @return True if the timespec was set with a valid monotonic time value.
 */
static bool manip_date_get_time_monotonic(struct timespec * const ts)
{
    bool retval = true;
#if defined(__APPLE__)
    uint64_t ns = mach_absolute_time();
    ts->tv_sec = ns / 1000000000;
    ts->tv_nsec = ns - (ts->tv_sec * 1000000000);
#else
    if (clock_gettime(CLOCK_MONOTONIC, ts) != 0)
    {
        retval = false;
    }
#endif
    return retval;
}

/**
 * @see See the header file for interface comments.
 */
bool manip_date_get_time_mono(uint64_t * const sec, uint64_t * const nsec)
{
    bool retval = false;
    struct timespec ts;

    if ((sec != NULL) && (nsec != NULL))
    {
        retval = manip_date_get_time_monotonic(&ts);

        if (retval == true)
        {
            *sec  = (uint64_t)ts.tv_sec;
            *nsec = (uint64_t)ts.tv_nsec;
        }
        else
        {
            *sec  = 0;
            *nsec = 0;
        }
    }

    return retval;
}

/**
 * @see See the header file for interface comments.
 */
uint64_t manip_date_get_time_mono_units(const enum units_prefix_time prefix)
{
    uint64_t retval = 0;
    struct timespec ts;

    if (manip_date_get_time_monotonic(&ts) == true)
    {
        retval = (uint64_t)(ts.tv_sec * prefix);

        if (prefix != UNITS_TIME_SEC)
        {
            retval += (uint64_t)(ts.tv_nsec / (UNITS_TIME_NSEC / prefix));
        }
    }

    return retval;
}

/**
 * @see See the header file for interface comments.
 */
uint64_t manip_date_get_time_mono_elapsed(const uint64_t tsref,
                                          const enum units_prefix_time prefix)
{
    uint64_t retval = 0;
    uint64_t tsnow = manip_date_get_time_mono_units(prefix);

    if (tsnow >= tsref)
    {
        retval = tsnow - tsref;
    }

    return retval;
}

/**
 * @see See the header file for interface comments.
 */
uint64_t manip_date_convert_units(const uint64_t ts,
                                  const enum units_prefix_time prefix,
                                  const enum units_prefix_time newprefix)
{
    uint64_t retval = (ts * newprefix) / prefix;

    return retval;
}

/**
 * @see See the header file for interface comments.
 */
bool dateGetTimeFormatted(const uint64_t ts,
                          const enum units_prefix_time prefix,
                          const char * const format)
{
    bool retval = true;
    time_t t;
    struct tm *tm;
    char buf[64];

    t = ts / prefix;

    tm = localtime(&t);

    strftime(buf, sizeof(buf), format, tm);

    return retval;
}

/**
 * @see See the header file for interface comments.
 */
uint64_t manip_date_get_time_diff(const uint64_t ts1,
                                  const uint64_t ts2,
                                  const enum units_prefix_time prefix,
                                  struct manip_date_diff * const diff)
{
    uint64_t diffts = (ts1 > ts2 ? ts1 - ts2 : ts2 - ts1);
    uint64_t diffms = manip_date_convert_units(diffts, prefix, UNITS_TIME_MSEC);

    if (diff != NULL)
    {
        diff->year  = (uint16_t)(diffms / UNITS_TIME_YEAR / UNITS_TIME_MSEC);
        diffms     -= (uint64_t)diff->year * UNITS_TIME_YEAR * UNITS_TIME_MSEC;

        diff->week  = (uint16_t)(diffms / UNITS_TIME_WEEK / UNITS_TIME_MSEC);
        diffms     -= (uint64_t)diff->week * UNITS_TIME_WEEK * UNITS_TIME_MSEC;

        diff->day   = (uint16_t)(diffms / UNITS_TIME_DAY / UNITS_TIME_MSEC);
        diffms     -= (uint64_t)diff->day * UNITS_TIME_DAY * UNITS_TIME_MSEC;

        diff->hour  = (uint16_t)(diffms / UNITS_TIME_HOUR / UNITS_TIME_MSEC);
        diffms     -= (uint64_t)diff->hour * UNITS_TIME_HOUR * UNITS_TIME_MSEC;

        diff->min   = (uint16_t)(diffms / UNITS_TIME_MIN / UNITS_TIME_MSEC);
        diffms     -= (uint64_t)diff->min * UNITS_TIME_MIN * UNITS_TIME_MSEC;

        diff->sec   = (uint16_t)(diffms / UNITS_TIME_SEC / UNITS_TIME_MSEC);
        diffms     -= (uint64_t)diff->sec * UNITS_TIME_SEC * UNITS_TIME_MSEC;

        diff->msec  = diffms;
    }

    return diffts;
}

/**
 * @see See the header file for interface comments.
 */
uint64_t manip_date_get_time_sec_parti(const uint64_t ts,
                                       const enum units_prefix_time prefix)
{
    uint64_t retval = ts / prefix;

    return retval;
}

/**
 * @see See the header file for interface comments.
 */
uint64_t manip_date_get_time_sec_partf(const uint64_t ts,
                                       const enum units_prefix_time prefix)
{
    uint64_t retval = ts - manip_date_get_time_sec_parti(ts, prefix) * prefix;

    return retval;
}
