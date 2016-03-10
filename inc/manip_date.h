/**
 * @file   manip_date.h
 * @author Shane Barnes
 * @date   08 Mar 2016
 * @brief  Date/time manipulation interface.
 */

#ifndef _MANIP_DATE_H_
#define _MANIP_DATE_H_

#include "manip_unit.h"
#include "system_types.h"

enum manip_date_clock
{
    DATE_CLOCK_UNDEFINED   = 0,
    DATE_CLOCK_MONOTONIC   = 1,
    DATE_CLOCK_REALTIME    = 2,
    DATE_CLOCK_UNSUPPORTED = 3
};

struct manip_date_diff
{
    uint16_t msec;
    uint16_t sec;
    uint16_t min;
    uint16_t hour;
    uint16_t day;
    uint16_t week;
    uint16_t year;
};

/**
 * @brief Get the monotonic or realtime clock time.
 *
 * @param[in]     clock The clock type (monotonic or realtime).
 * @param[in,out] sec   A seconds time buffer to set with the current time.
 * @param[in,out] nsec  A nanoseconds time buffer to set with the current time.
 *
 * @return True if the input time buffers were set with valid time values.
 *         Otherwise, the input time buffers were set to zero.
 */
bool manip_date_time(const enum manip_date_clock clock,
                     uint64_t * const sec,
                     uint64_t * const nsec);

/**
 * @brief Get the monotonic or realtime clock time.
 *
 * @param[in] clock The clock type (monotonic or realtime).
 * @param[in] prefix The monotonic time unit or prefix.
 *
 * @return The time in the time unit specified (0 on error).
 */
uint64_t manip_date_time_units(const enum manip_date_clock clock,
                               const enum unit_prefix_time prefix);

/**
 * @brief Get the elapsed time given a monotonic start time reference.
 *
 * @param[in] tsref  The monotonic start time reference.
 * @param[in] prefix The monotonic time unit or prefix.
 *
 * @return The elapsed time in the specified time units (0 on error).
 */
uint64_t manip_date_time_mono_elapsed(const uint64_t tsref,
                                      const enum unit_prefix_time prefix);

/**
 * @brief Convert a Unix timestamp into different time units.
 *
 * @param[in] ts        A Unix timestamp.
 * @param[in] prefix    The Unix timestamp time unit or prefix.
 * @param[in] newprefix The new time unit or prefix for the Unix timestamp.
 *
 * @return The converted Unix timestamp.
 */
uint64_t manip_date_convert_units(const uint64_t ts,
                                  const enum unit_prefix_time prefix,
                                  const enum unit_prefix_time newprefix);

/**
 * @brief Get a formatted string representation of a Unix timestamp.
 *
 * @param[in]     ts     A Unix timestamp.
 * @param[in]     prefix The Unix timestamp time unit or prefix.
 * @param[in]     format The date/time format (e.g., "%Y-%m-%d %H:%M:%S").
 * @param[in,out] buf    The buffer to fill with the formatted string.
 * @param[in]     len    The maximum size of the buffer in bytes.
 *
 * @return True if a formatted string representation was created.
 */
bool manip_date_time_format(const uint64_t ts,
                            const enum unit_prefix_time prefix,
                            const char * const format,
                            char * const buf,
                            const uint32_t len);

/**
 * @brief Get the (absolute) time difference between two Unix timestamps.
 *        Optionally, populate a manip_date_diff structure with the time
 *        difference.
 *
 * @param[in]     ts1    The first Unix timestamp.
 * @param[in]     ts2    The second Unix timestamp.
 * @param[in]     prefix The time unit or prefix common to both Unix timestamps.
 * @param[in,out] diff   A manip_date_diff structure to populate (optional).
 *
 * @return The (absolute) time difference between two Unix timestamps.
 */
uint64_t manip_date_time_diff(const uint64_t ts1,
                              const uint64_t ts2,
                              const enum unit_prefix_time prefix,
                              struct manip_date_diff * const diff);

/**
 * @brief Get the integer-part of a Unix timestamp in seconds.
 *
 * @param[in] ts     A Unix timestamp.
 * @param[in] prefix The Unix timestamp time unit or prefix.
 *
 * @return The integer-part of a Unix timestamp in seconds.
 */
uint64_t manip_date_time_sec_parti(const uint64_t ts,
                                   const enum unit_prefix_time prefix);

/**
 * @brief Get the fractional-part of a Unix timestamp.
 *
 * @param[in] ts     A Unix timestamp.
 * @param[in] prefix The Unix timestamp time unit or prefix.
 *
 * @return The fractional-part of a Unix timestamp in the specified time unit.
 */
uint64_t manip_date_time_sec_partf(const uint64_t ts,
                                   const enum unit_prefix_time prefix);

#endif // _MANIP_DATE_H_
