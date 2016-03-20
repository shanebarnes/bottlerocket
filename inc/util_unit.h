/**
 * @file   util_unit.h
 * @author Shane Barnes
 * @date   06 Mar 2016
 * @brief  Unit utility interface.
 */

#ifndef _UTIL_UNIT_H_
#define _UTIL_UNIT_H_

#include "system_types.h"

// Metric (base 10) prefixes
enum unit_prefix_metric
{
    UNIT_SI_BASE = 1,
    UNIT_SI_KILO = UNIT_SI_BASE * 1000LL,
    UNIT_SI_MEGA = UNIT_SI_KILO * 1000LL,
    UNIT_SI_GIGA = UNIT_SI_MEGA * 1000LL,
    UNIT_SI_TERA = UNIT_SI_GIGA * 1000LL,
    UNIT_SI_PETA = UNIT_SI_TERA * 1000LL,
    UNIT_SI_EXA  = UNIT_SI_PETA * 1000LL
};

// Binary (base 2) prefixes
enum unit_prefix_binary
{
    UNIT_IEC_BASE = 1,
    UNIT_IEC_KIBI = 1LL << 10,
    UNIT_IEC_MEBI = 1LL << 20,
    UNIT_IEC_GIBI = 1LL << 30,
    UNIT_IEC_TEBI = 1LL << 40,
    UNIT_IEC_PEBI = 1LL << 50,
    UNIT_IEC_EXBI = 1LL << 60
};

enum unit_prefix_bitrate
{
    UNIT_BITRATE_BPS   = UNIT_SI_BASE,  //  bps
    UNIT_BITRATE_KBPS  = UNIT_SI_KILO,  // kbps
    UNIT_BITRATE_KIBPS = UNIT_IEC_KIBI, // Kibps
    UNIT_BITRATE_MBPS  = UNIT_SI_MEGA,  // Mbps
    UNIT_BITRATE_MIBPS = UNIT_IEC_MEBI, // Mibps
    UNIT_BITRATE_GBPS  = UNIT_SI_GIGA,  // Gbps
    UNIT_BITRATE_GIBPS = UNIT_IEC_GIBI, // Gibps
    UNIT_BITRATE_TBPS  = UNIT_SI_TERA,  // Tbps
    UNIT_BITRATE_TIBPS = UNIT_IEC_TEBI, // Tibps
    UNIT_BITRATE_PBPS  = UNIT_SI_PETA,  // Pbps
    UNIT_BITRATE_PIBPS = UNIT_IEC_PEBI, // Pibps
    UNIT_BITRATE_EBPS  = UNIT_SI_EXA,   // Ebps
    UNIT_BITRATE_EIBPS = UNIT_IEC_EXBI  // Eibps
};

enum unit_prefix_byte
{
    UNIT_BYTE_B   = UNIT_SI_BASE,  //  B
    UNIT_BYTE_KB  = UNIT_SI_KILO,  // KB
    UNIT_BYTE_KIB = UNIT_IEC_KIBI, // KiB
    UNIT_BYTE_MB  = UNIT_SI_MEGA,  // MB
    UNIT_BYTE_MIB = UNIT_IEC_MEBI, // MiB
    UNIT_BYTE_GB  = UNIT_SI_GIGA,  // GB
    UNIT_BYTE_GIB = UNIT_IEC_GIBI, // GiB
    UNIT_BYTE_TB  = UNIT_SI_TERA,  // TB
    UNIT_BYTE_TIB = UNIT_IEC_TEBI, // TiB
    UNIT_BYTE_PB  = UNIT_SI_PETA,  // PB
    UNIT_BYTE_PIB = UNIT_IEC_PEBI, // PiB
    UNIT_BYTE_EB  = UNIT_SI_EXA,   // EB
    UNIT_BYTE_EIB = UNIT_IEC_EXBI  // EiB
};

enum unit_prefix_time
{
    UNIT_TIME_SEC  = UNIT_SI_BASE,  //  1 second
    UNIT_TIME_MIN  = 60,            //  1 minute =     1 second/second * 60 seconds
    UNIT_TIME_HOUR = 3600,          //  1 hour   =    60 seconds/minute * 60 minutes
    UNIT_TIME_DAY  = 86400,         //  1 day    =  3600 seconds/hour * 24 hours
    UNIT_TIME_WEEK = 604800,        //  1 week   = 86400 seconds/day * 7 days
    UNIT_TIME_YEAR = 31557600,      //  1 year   = 86400 seconds/day * 365.25 days
    UNIT_TIME_MSEC = UNIT_TIME_SEC  * 1000LL,
    UNIT_TIME_USEC = UNIT_TIME_MSEC * 1000LL,
    UNIT_TIME_NSEC = UNIT_TIME_USEC * 1000LL,
    UNIT_TIME_PSEC = UNIT_TIME_NSEC * 1000LL
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
uint64_t util_unit_get_bitrate(const char * const bitrate);

/**
 * @brief Convert a string representation of a byte count to a numeric
 *        representation.
 *
 * @param[in] bytes A string representation of a byte count.
 *
 * @return An integer value equivalent to the string representation of a byte
 *         count (0 on error).
 */
uint64_t util_unit_get_bytes(const char * const bytes);

/**
 * @brief Convert a string representation of a seconds count to a numeric
 *        representation.
 *
 * @param[in] seconds A string representation of a seconds count.
 *
 * @return An integer value equivalent to the string representation of a seconds
 *         (0 on error).
 */
uint64_t util_unit_get_seconds(const char * const seconds);

/**
 * @brief Convert a number to decimal notation (e.g., 1200 bytes = 1.200 KB).
 *
 * @param[in]     base      The base of the numbering system to be used with
 *                          decimal notation (i.e., 1000 or 1024).
 * @param[in]     precision The number of fraction digits to include in the
 *                          decimal notation.
 * @param[in]     integer   An integer to convert to decimal notation.
 * @param[in,out] buf       A string buffer to store the decimal notation
 *                          representation of a number.
 * @param[in]     len       The size of the string buffer in bytes.
 *
 * @return Void.
 */
void util_unit_get_decimal_notation(const uint64_t base,
                                    const uint8_t precision,
                                    const uint64_t integer,
                                    char * const buf,
                                    const size_t len);

#endif // _UTIL_UNIT_H_
