/**
 * @file   manip_unit.h
 * @author Shane Barnes
 * @date   06 Mar 2016
 * @brief  Unit manipulation interface.
 */

#ifndef _MANIP_UNIT_H_
#define _MANIP_UNIT_H_

#include "system_types.h"

// Metric (base 10) prefixes
enum units_prefix_metric
{
    UNITS_SI_BASE = 1,
    UNITS_SI_KILO = UNITS_SI_BASE * 1000,
    UNITS_SI_MEGA = UNITS_SI_KILO * 1000,
    UNITS_SI_GIGA = UNITS_SI_MEGA * 1000,
    UNITS_SI_TERA = UNITS_SI_GIGA * 1000,
    UNITS_SI_PETA = UNITS_SI_TERA * 1000,
    UNITS_SI_EXA  = UNITS_SI_PETA * 1000
};

// Binary (base 2) prefixes
enum units_prefix_binary
{
    UNITS_IEC_BASE = 1,
    UNITS_IEC_KIBI = 1LL << 10,
    UNITS_IEC_MEBI = 1LL << 20,
    UNITS_IEC_GIBI = 1LL << 30,
    UNITS_IEC_TEBI = 1LL << 40,
    UNITS_IEC_PEBI = 1LL << 50,
    UNITS_IEC_EXBI = 1LL << 60
};

enum units_prefix_bitrate
{
    UNITS_BITRATE_BPS   = UNITS_SI_BASE,  //  bps
    UNITS_BITRATE_KBPS  = UNITS_SI_KILO,  // kbps
    UNITS_BITRATE_KIBPS = UNITS_IEC_KIBI, // Kibps
    UNITS_BITRATE_MBPS  = UNITS_SI_MEGA,  // Mbps
    UNITS_BITRATE_MIBPS = UNITS_IEC_MEBI, // Mibps
    UNITS_BITRATE_GBPS  = UNITS_SI_GIGA,  // Gbps
    UNITS_BITRATE_GIBPS = UNITS_IEC_GIBI, // Gibps
    UNITS_BITRATE_TBPS  = UNITS_SI_TERA,  // Tbps
    UNITS_BITRATE_TIBPS = UNITS_IEC_TEBI, // Tibps
    UNITS_BITRATE_PBPS  = UNITS_SI_PETA,  // Pbps
    UNITS_BITRATE_PIBPS = UNITS_IEC_PEBI, // Pibps
    UNITS_BITRATE_EBPS  = UNITS_SI_EXA,   // Ebps
    UNITS_BITRATE_EIBPS = UNITS_IEC_EXBI  // Eibps
};

enum units_prefix_byte
{
    UNITS_BYTE_B   = UNITS_SI_BASE,  //  B
    UNITS_BYTE_KB  = UNITS_SI_KILO,  // KB
    UNITS_BYTE_KIB = UNITS_IEC_KIBI, // KiB
    UNITS_BYTE_MB  = UNITS_SI_MEGA,  // MB
    UNITS_BYTE_MIB = UNITS_IEC_MEBI, // MiB
    UNITS_BYTE_GB  = UNITS_SI_GIGA,  // GB
    UNITS_BYTE_GIB = UNITS_IEC_GIBI, // GiB
    UNITS_BYTE_TB  = UNITS_SI_TERA,  // TB
    UNITS_BYTE_TIB = UNITS_IEC_TEBI, // TiB
    UNITS_BYTE_PB  = UNITS_SI_PETA,  // PB
    UNITS_BYTE_PIB = UNITS_IEC_PEBI, // PiB
    UNITS_BYTE_EB  = UNITS_SI_EXA,   // EB
    UNITS_BYTE_EIB = UNITS_IEC_EXBI  // EiB
};

enum units_conversion_time
{
    UNITS_TIME_SEC  = UNITS_SI_BASE, //  1 second
    UNITS_TIME_MIN  = 60,            //  1 minute =     1 second/second * 60 seconds
    UNITS_TIME_HOUR = 3600,          //  1 hour   =    60 seconds/minute * 60 minutes
    UNITS_TIME_DAY  = 86400,         //  1 day    =  3600 seconds/hour * 24 hours
    UNITS_TIME_WEEK = 604800,        //  1 week   = 86400 seconds/day * 7 days
    UNITS_TIME_YEAR = 31557600       //  1 year   = 86400 seconds/day * 365.25 days
};

enum units_prefix_time
{
    UNITS_TIME_MSEC = UNITS_TIME_SEC  * 1000,
    UNITS_TIME_USEC = UNITS_TIME_MSEC * 1000,
    UNITS_TIME_NSEC = UNITS_TIME_USEC * 1000,
    UNITS_TIME_PSEC = UNITS_TIME_NSEC * 1000
};

/**
 * @brief Convert a string representation of a bit rate count to a numeric
 *        representation.
 *
 * @param[in] bitrate A string representation of a bit rate count.
 *
 * @return An integer value equivalent to the string representation of a bit
 *         rate count (0 on error).
 */
uint64_t manip_unit_get_bitrate(const char * const bitrate);

/**
 * @brief Convert a string representation of a byte count to a numeric
 *        representation.
 *
 * @param[in] bytes A string representation of a byte count.
 *
 * @return An integer value equivalent to the string representation of a byte
 *         count (0 on error).
 */
uint64_t manip_unit_get_bytes(const char * const bytes);

/**
 * @brief Convert a string representation of a seconds count to a numeric
 *        representation.
 *
 * @param[in] seconds A string representation of a seconds count.
 *
 * @return An integer value equivalent to the string representation of a seconds
 *         (0 on error).
 */
uint64_t manip_unit_get_seconds(const char * const seconds);

/**
 * @brief Convert a number to decimal notation (e.g., 1200 bytes = 1.200 KB).
 *
 * @param[in]     base      The base of the numbering system to be used with
 *                          decimal notation (i.e., 1000 or 1024).
 * @parami[in]    precision The number of fraction digits to include in the
 *                          decimal notation.
 * @param[in]     integer   An integer to convert to decimal notation.
 * @param[in,out] buf       A string buffer to store the decimal notation
 *                          representation of a number.
 */
void manip_unit_get_decimal_notation(const uint64_t base,
                                     const uint8_t precision,
                                     const uint64_t integer,
                                     char * const buf);

#endif // _MANIP_UNIT_H_
