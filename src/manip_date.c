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
#include <sys/time.h>
#include <time.h>

/**
 * @brief Get the monotonic or realtime clock time.
 *
 * @param[in]     clock The clock type (monotonic or realtime).
 * @param[in,out] ts    A timespec structure to set with the current monotonic
 *                      time.
 *
 * @return True if the timespec was set with a valid time value.
 */
static bool manip_date_time_clock(const enum manip_date_clock clock,
                                  struct timespec * const ts)
{
    bool retval = true;

    if ((clock > DATE_CLOCK_UNDEFINED) && (clock < DATE_CLOCK_UNSUPPORTED))
    {
#if defined(__APPLE__)
        struct timeval tv;

        if (clock == DATE_CLOCK_MONOTONIC)
        {
            uint64_t ns = mach_absolute_time();
            ts->tv_sec = ns / UNIT_TIME_NSEC;
            ts->tv_nsec = ns - (ts->tv_sec * UNIT_TIME_NSEC);
        }
        else
        {
            if (gettimeofday(&tv, NULL) == 0)
            {
                ts->tv_sec  = tv.tv_sec;
                ts->tv_nsec = tv.tv_usec * 1000;
            }
            else
            {
                retval = false;
            }
        }
#else
        if (clock_gettime(clock == DATE_CLOCK_MONOTONIC ?
                                   CLOCK_MONOTONIC : CLOCK_REALTIME, ts) != 0)
        {
            retval = false;
        }
#endif
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool manip_date_time(const enum manip_date_clock clock,
                     uint64_t * const sec,
                     uint64_t * const nsec)
{
    bool retval = false;
    struct timespec ts;

    if ((sec != NULL) && (nsec != NULL))
    {
        retval = manip_date_time_clock(clock, &ts);

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
 * @see See header file for interface comments.
 */
uint64_t manip_date_time_units(const enum manip_date_clock clock,
                               const enum unit_prefix_time prefix)
{
    uint64_t retval = 0;
    struct timespec ts;

    if (manip_date_time_clock(clock, &ts) == true)
    {
        retval = (uint64_t)(ts.tv_sec * prefix);

        if (prefix != UNIT_TIME_SEC)
        {
            retval += (uint64_t)(ts.tv_nsec / (UNIT_TIME_NSEC / prefix));
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t manip_date_time_mono_elapsed(const uint64_t tsref,
                                      const enum unit_prefix_time prefix)
{
    uint64_t retval = 0;
    uint64_t tsnow = manip_date_time_units(DATE_CLOCK_MONOTONIC, prefix);

    if (tsnow >= tsref)
    {
        retval = tsnow - tsref;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t manip_date_convert_units(const uint64_t ts,
                                  const enum unit_prefix_time prefix,
                                  const enum unit_prefix_time newprefix)
{
    uint64_t retval = (ts * newprefix) / prefix;

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool manip_date_time_format(const uint64_t ts,
                            const enum unit_prefix_time prefix,
                            const char * const format,
                            char * const buf,
                            uint32_t len)
{
    bool retval = false;
    time_t time;
    struct tm *tm;

    if ((buf != NULL) && (len > 0))
    {
        time = ts / prefix;

        tm = localtime(&time);

        if (strftime(buf, len, format, tm) != 0)
        {
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t manip_date_time_diff(const uint64_t ts1,
                              const uint64_t ts2,
                              const enum unit_prefix_time prefix,
                              struct manip_date_diff * const diff)
{
    uint64_t diffts = (ts1 > ts2 ? ts1 - ts2 : ts2 - ts1);
    uint64_t diffms = manip_date_convert_units(diffts, prefix, UNIT_TIME_MSEC);

    if (diff != NULL)
    {
        diff->year  = (uint16_t)(diffms / UNIT_TIME_YEAR / UNIT_TIME_MSEC);
        diffms     -= (uint64_t)diff->year * UNIT_TIME_YEAR * UNIT_TIME_MSEC;

        diff->week  = (uint16_t)(diffms / UNIT_TIME_WEEK / UNIT_TIME_MSEC);
        diffms     -= (uint64_t)diff->week * UNIT_TIME_WEEK * UNIT_TIME_MSEC;

        diff->day   = (uint16_t)(diffms / UNIT_TIME_DAY / UNIT_TIME_MSEC);
        diffms     -= (uint64_t)diff->day * UNIT_TIME_DAY * UNIT_TIME_MSEC;

        diff->hour  = (uint16_t)(diffms / UNIT_TIME_HOUR / UNIT_TIME_MSEC);
        diffms     -= (uint64_t)diff->hour * UNIT_TIME_HOUR * UNIT_TIME_MSEC;

        diff->min   = (uint16_t)(diffms / UNIT_TIME_MIN / UNIT_TIME_MSEC);
        diffms     -= (uint64_t)diff->min * UNIT_TIME_MIN * UNIT_TIME_MSEC;

        diff->sec   = (uint16_t)(diffms / UNIT_TIME_SEC / UNIT_TIME_MSEC);
        diffms     -= (uint64_t)diff->sec * UNIT_TIME_SEC * UNIT_TIME_MSEC;

        diff->msec  = diffms;
    }

    return diffts;
}

/**
 * @see See header file for interface comments.
 */
uint64_t manip_date_time_sec_parti(const uint64_t ts,
                                   const enum unit_prefix_time prefix)
{
    uint64_t retval = ts / prefix;

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t manip_date_time_sec_partf(const uint64_t ts,
                                   const enum unit_prefix_time prefix)
{
    uint64_t retval = ts - manip_date_time_sec_parti(ts, prefix) * prefix;

    return retval;
}
