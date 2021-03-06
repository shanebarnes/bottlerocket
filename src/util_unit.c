/**
 * @file      util_unit.c
 * @brief     Unit utility implementation.
 * @author    Shane Barnes
 * @date      07 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "util_string.h"
#include "util_unit.h"

#include <stdio.h>

/**
 * @brief Convert a string representation of a bit rate prefix to an integer
 *        value.
 *
 * @param[in] prefix A string representation of a bit rate prefix.
 *
 * @return Return an integer value equivalent to the string representation of a
 *         bit rate prefix (0 on error).
 */
static uint64_t utilunit_getbitrateprefix(const char *const prefix)
{
    uint64_t ret = 0;

    if (utilstring_compare(prefix, "BPS", 0, true) ||
        utilstring_compare(prefix, "", 0, true))
    {
        ret = UNIT_BITRATE_BPS;
    }
    else if (utilstring_compare(prefix, "KBPS", 0, true) ||
             utilstring_compare(prefix, "K", 0, true))
    {
        ret = UNIT_BITRATE_KBPS;
    }
    else if (utilstring_compare(prefix, "KIBPS", 0, true) ||
             utilstring_compare(prefix, "KI", 0, true))
    {
        ret = UNIT_BITRATE_KIBPS;
    }
    else if (utilstring_compare(prefix, "MBPS", 0, true) ||
             utilstring_compare(prefix, "M", 0, true))
    {
        ret = UNIT_BITRATE_MBPS;
    }
    else if (utilstring_compare(prefix, "MIBPS", 0, true) ||
             utilstring_compare(prefix, "MI", 0, true))
    {
        ret = UNIT_BITRATE_MIBPS;
    }
    else if (utilstring_compare(prefix, "GBPS", 0, true) ||
             utilstring_compare(prefix, "G", 0, true))
    {
        ret = UNIT_BITRATE_GBPS;
    }
    else if (utilstring_compare(prefix, "GIBPS", 0, true) ||
             utilstring_compare(prefix, "GI", 0, true))
    {
        ret = UNIT_BITRATE_GIBPS;
    }
    else if (utilstring_compare(prefix, "TBPS", 0, true) ||
             utilstring_compare(prefix, "T", 0, true))
    {
        ret = UNIT_BITRATE_TBPS;
    }
    else if (utilstring_compare(prefix, "TIBPS", 0, true) ||
             utilstring_compare(prefix, "TI", 0, true))
    {
        ret = UNIT_BITRATE_TIBPS;
    }
    else if (utilstring_compare(prefix, "PBPS", 0, true) ||
             utilstring_compare(prefix, "P", 0, true))
    {
        ret = UNIT_BITRATE_PBPS;
    }
    else if (utilstring_compare(prefix, "PIBPS", 0, true) ||
             utilstring_compare(prefix, "PI", 0, true))
    {
        ret = UNIT_BITRATE_PIBPS;
    }
    else if (utilstring_compare(prefix, "EBPS", 0, true) ||
             utilstring_compare(prefix, "E", 0, true))
    {
        ret = UNIT_BITRATE_EBPS;
    }
    else if (utilstring_compare(prefix, "EIBPS", 0, true) ||
             utilstring_compare(prefix, "EI", 0, true))
    {
        ret = UNIT_BITRATE_EIBPS;
    }

    return ret;
}

/**
 * @brief Convert a string representation of a byte prefix to an integer value.
 *
 * @param[in] prefix A string representation of a byte prefix.
 *
 * @return Return an integer value equivalent to the string representation of a
 *         byte prefix (0 on error).
 */
static uint64_t utilunit_getbyteprefix(const char * const prefix)
{
    uint64_t ret = 0;

    if (utilstring_compare(prefix, "B", 0, true) ||
        utilstring_compare(prefix, "", 0, true))
    {
        ret = UNIT_BYTE_B;
    }
    else if (utilstring_compare(prefix, "KB", 0, true) ||
             utilstring_compare(prefix, "K", 0, true))
    {
        ret = UNIT_BYTE_KB;
    }
    else if (utilstring_compare(prefix, "KIB", 0, true) ||
             utilstring_compare(prefix, "KI", 0, true))
    {
        ret = UNIT_BYTE_KIB;
    }
    else if (utilstring_compare(prefix, "MB", 0, true) ||
             utilstring_compare(prefix, "M", 0, true))
    {
        ret = UNIT_BYTE_MB;
    }
    else if (utilstring_compare(prefix, "MIB", 0, true) ||
             utilstring_compare(prefix, "MI", 0, true))
    {
        ret = UNIT_BYTE_MIB;
    }
    else if (utilstring_compare(prefix, "GB", 0, true) ||
             utilstring_compare(prefix, "G", 0, true))
    {
        ret = UNIT_BYTE_GB;
    }
    else if (utilstring_compare(prefix, "GIB", 0, true) ||
             utilstring_compare(prefix, "GI", 0, true))
    {
        ret = UNIT_BYTE_GIB;
    }
    else if (utilstring_compare(prefix, "TB", 0, true) ||
             utilstring_compare(prefix, "T", 0, true))
    {
        ret = UNIT_BYTE_TB;
    }
    else if (utilstring_compare(prefix, "TIB", 0, true) ||
             utilstring_compare(prefix, "TI", 0, true))
    {
        ret = UNIT_BYTE_TIB;
    }
    else if (utilstring_compare(prefix, "PB", 0, true) ||
             utilstring_compare(prefix, "P", 0, true))
    {
        ret = UNIT_BYTE_PB;
    }
    else if (utilstring_compare(prefix, "PIB", 0, true) ||
             utilstring_compare(prefix, "PI", 0, true))
    {
        ret = UNIT_BYTE_PIB;
    }
    else if (utilstring_compare(prefix, "EB", 0, true) ||
             utilstring_compare(prefix, "E", 0, true))
    {
        ret = UNIT_BYTE_EB;
    }
    else if (utilstring_compare(prefix, "EIB", 0, true) ||
             utilstring_compare(prefix, "EI", 0, true))
    {
        ret = UNIT_BYTE_EIB;
    }

    return ret;
}

/**
 * @brief Convert a string representation of a seconds prefix to an integer
 *        value.
 *
 * @param[in] prefix A string representation of a seconds prefix.
 *
 * @return Return an integer value equivalent to the string representation of a
 *         seconds prefix (0 on error).
 */
static uint64_t utilunit_getsecsprefix(const char * const prefix)
{
    uint64_t ret = 0;

    if (utilstring_compare(prefix, "PS", 0, true))
    {
        ret = UNIT_TIME_PSEC;
    }
    else if (utilstring_compare(prefix, "NS", 0, true))
    {
        ret = UNIT_TIME_NSEC;
    }
    else if (utilstring_compare(prefix, "US", 0, true))
    {
        ret = UNIT_TIME_USEC;
    }
    else if (utilstring_compare(prefix, "MS", 0, true))
    {
        ret = UNIT_TIME_MSEC;
    }
    else if (utilstring_compare(prefix, "S", 0, true) ||
             utilstring_compare(prefix, "", 0, true))
    {
        ret = UNIT_TIME_SEC;
    }
    else if (utilstring_compare(prefix, "M", 0, true))
    {
        ret = UNIT_TIME_MIN;
    }
    else if (utilstring_compare(prefix, "H", 0, true))
    {
        ret = UNIT_TIME_HOUR;
    }
    else if (utilstring_compare(prefix, "D", 0, true))
    {
        ret = UNIT_TIME_DAY;
    }
    else if (utilstring_compare(prefix, "W", 0, true))
    {
        ret = UNIT_TIME_WEEK;
    }
    else if (utilstring_compare(prefix, "Y", 0, true))
    {
        ret = UNIT_TIME_YEAR;
    }

    return ret;
}

int64_t utilunit_getbitrate(const char * const bitrate)
{
    int64_t ret = -1;
    int32_t matchcount = 0;
    char    prefix[16];

    matchcount = utilstring_parse(bitrate, "%" PRIi64 "%s", &ret, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            ret = ret * utilunit_getbitrateprefix(prefix);
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

uint64_t utilunit_getbytes(const char * const bytes)
{
    uint64_t ret = 0;
    int32_t  matchcount = 0;
    char     prefix[16];

    matchcount = utilstring_parse(bytes, "%" PRIu64 "%s", &ret, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            ret = ret * utilunit_getbyteprefix(prefix);
            break;
        default:
            // Nothing to do.
            break;
    }

    return ret;
}

uint64_t utilunit_getsecs(const char * const secs,
                          const enum unit_prefix_time units)
{
    uint64_t ret = 0;
    uint64_t scale = 0;
    int32_t  matchcount = 0;
    bool     sfraction = false, ufraction = false;
    char     prefix[16];

    matchcount = utilstring_parse(secs, "%" PRIu64 "%s", &ret, prefix);

    switch (matchcount)
    {
        case 1:
           scale = UNIT_TIME_SEC;
           break;
        case 2:
            // @todo Protect against overflow.
            scale = utilunit_getsecsprefix(prefix);
            break;
        default:
            // Nothing to do.
            break;
    }

    if (scale > 0)
    {
        // Fractional time units (with respect to the seconds base unit) have
        // prefixes that are multiples of 1000.
        ufraction = (units % 1000 == 0 ? true : false);
        sfraction = (scale % 1000 == 0 ? true : false);

        if (sfraction)
        {
            if (ufraction)
            {
                ret *= units;
                ret /= scale;
            }
            else
            {
                ret /= units;
                ret /= scale;
            }
        }
        else
        {
            if (ufraction)
            {
                ret *= scale;
                ret *= units;
            }
            else
            {
                ret *= scale;
                ret /= units;
            }
        }
    }

    return ret;
}

/**
 * @brief Get the prefix (multiple) symbol.
 *
 * @param[in] prefix A prefix (multiple).
 *
 * @return A character representing the prefix value (null character on error).
 */
static char utilunit_getprefixsymbol(const uint64_t prefix)
{
    char ret = '\0';

    switch (prefix)
    {
        case UNIT_SI_KILO:
            ret = 'k';
            break;
        case UNIT_IEC_KIBI:
            ret = 'K';
            break;
        case UNIT_SI_MEGA:
        case UNIT_IEC_MEBI:
            ret = 'M';
            break;
        case UNIT_SI_GIGA:
        case UNIT_IEC_GIBI:
            ret = 'G';
            break;
        case UNIT_SI_TERA:
        case UNIT_IEC_TEBI:
            ret = 'T';
            break;
        case UNIT_SI_PETA:
        case UNIT_IEC_PEBI:
            ret = 'P';
            break;
        case UNIT_SI_EXA:
        case UNIT_IEC_EXBI:
            ret = 'E';
            break;
        default:
            break;
    }

    return ret;
}

void utilunit_getdecformat(const uint64_t base,
                           const uint8_t precision,
                           const uint64_t integer,
                           char * const buf,
                           const size_t len)
{
    uint64_t parti          = integer;
    uint64_t partf          = 0;
    uint64_t prefix         = UNIT_SI_BASE;
    uint64_t scale          = UNIT_SI_BASE;
    uint64_t digitcount     = 0;
    uint64_t precisionwidth = precision > 0 ? 1 : 0;

    if (buf != NULL)
    {
        switch (base)
        {
            case 2:
                scale = 1024;
                break;
            case 10:
            default:
                scale = 1000;
                break;
        }

        while ((parti >= (prefix * scale)) && (prefix <= (uint64_t)(UNIT_SI_PETA)))
        {
            if (digitcount <= precision)
            {
                switch (precision - digitcount)
                {
                    case 0:
                        break;
                    case 1:
                        precisionwidth *= 10;
                        break;
                    case 2:
                        precisionwidth *= 100;
                        break;
                    case 3:
                    default:
                        precisionwidth *= 1000;
                        break;
                }
            }

            prefix     *= scale;
            digitcount += 3;
        }

        parti /= prefix;
        partf  = integer - (parti * prefix);
        partf  = partf * precisionwidth / prefix;

        if ((precision > 0) && (digitcount > 0))
        {
            utilstring_concat(buf,
                              len,
                              "%" PRIu64 ".%.*u %c%c",
                              parti,
                              precision > digitcount ?
                                  (uint8_t)digitcount : precision,
                              (uint32_t)partf,
                              utilunit_getprefixsymbol(prefix),
                              scale == 1024 ? 'i' : '\0');
        }
        else
        {
            utilstring_concat(buf,
                              len,
                              "%" PRIu64 " %c%c",
                              parti,
                              utilunit_getprefixsymbol(prefix),
                              scale == 1024 ? 'i' : '\0');
        }
    }
}
