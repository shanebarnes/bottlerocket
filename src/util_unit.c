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
    uint64_t retval = 0;

    if (utilstring_compare(prefix, "BPS", true) ||
        utilstring_compare(prefix, "", true))
    {
        retval = UNIT_BITRATE_BPS;
    }
    else if (utilstring_compare(prefix, "KBPS", true) ||
             utilstring_compare(prefix, "K", true))
    {
        retval = UNIT_BITRATE_KBPS;
    }
    else if (utilstring_compare(prefix, "KIBPS", true) ||
             utilstring_compare(prefix, "KI", true))
    {
        retval = UNIT_BITRATE_KIBPS;
    }
    else if (utilstring_compare(prefix, "MBPS", true) ||
             utilstring_compare(prefix, "M", true))
    {
        retval = UNIT_BITRATE_MBPS;
    }
    else if (utilstring_compare(prefix, "MIBPS", true) ||
             utilstring_compare(prefix, "MI", true))
    {
        retval = UNIT_BITRATE_MIBPS;
    }
    else if (utilstring_compare(prefix, "GBPS", true) ||
             utilstring_compare(prefix, "G", true))
    {
        retval = UNIT_BITRATE_GBPS;
    }
    else if (utilstring_compare(prefix, "GIBPS", true) ||
             utilstring_compare(prefix, "GI", true))
    {
        retval = UNIT_BITRATE_GIBPS;
    }
    else if (utilstring_compare(prefix, "TBPS", true) ||
             utilstring_compare(prefix, "T", true))
    {
        retval = UNIT_BITRATE_TBPS;
    }
    else if (utilstring_compare(prefix, "TIBPS", true) ||
             utilstring_compare(prefix, "TI", true))
    {
        retval = UNIT_BITRATE_TIBPS;
    }
    else if (utilstring_compare(prefix, "PBPS", true) ||
             utilstring_compare(prefix, "P", true))
    {
        retval = UNIT_BITRATE_PBPS;
    }
    else if (utilstring_compare(prefix, "PIBPS", true) ||
             utilstring_compare(prefix, "PI", true))
    {
        retval = UNIT_BITRATE_PIBPS;
    }
    else if (utilstring_compare(prefix, "EBPS", true) ||
             utilstring_compare(prefix, "E", true))
    {
        retval = UNIT_BITRATE_EBPS;
    }
    else if (utilstring_compare(prefix, "EIBPS", true) ||
             utilstring_compare(prefix, "EI", true))
    {
        retval = UNIT_BITRATE_EIBPS;
    }

    return retval;
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
    uint64_t retval = 0;

    if (utilstring_compare(prefix, "B", true) ||
        utilstring_compare(prefix, "", true))
    {
        retval = UNIT_BYTE_B;
    }
    else if (utilstring_compare(prefix, "KB", true) ||
             utilstring_compare(prefix, "K", true))
    {
        retval = UNIT_BYTE_KB;
    }
    else if (utilstring_compare(prefix, "KIB", true) ||
             utilstring_compare(prefix, "KI", true))
    {
        retval = UNIT_BYTE_KIB;
    }
    else if (utilstring_compare(prefix, "MB", true) ||
             utilstring_compare(prefix, "M", true))
    {
        retval = UNIT_BYTE_MB;
    }
    else if (utilstring_compare(prefix, "MIB", true) ||
             utilstring_compare(prefix, "MI", true))
    {
        retval = UNIT_BYTE_MIB;
    }
    else if (utilstring_compare(prefix, "GB", true) ||
             utilstring_compare(prefix, "G", true))
    {
        retval = UNIT_BYTE_GB;
    }
    else if (utilstring_compare(prefix, "GIB", true) ||
             utilstring_compare(prefix, "GI", true))
    {
        retval = UNIT_BYTE_GIB;
    }
    else if (utilstring_compare(prefix, "TB", true) ||
             utilstring_compare(prefix, "T", true))
    {
        retval = UNIT_BYTE_TB;
    }
    else if (utilstring_compare(prefix, "TIB", true) ||
             utilstring_compare(prefix, "TI", true))
    {
        retval = UNIT_BYTE_TIB;
    }
    else if (utilstring_compare(prefix, "PB", true) ||
             utilstring_compare(prefix, "P", true))
    {
        retval = UNIT_BYTE_PB;
    }
    else if (utilstring_compare(prefix, "PIB", true) ||
             utilstring_compare(prefix, "PI", true))
    {
        retval = UNIT_BYTE_PIB;
    }
    else if (utilstring_compare(prefix, "EB", true) ||
             utilstring_compare(prefix, "E", true))
    {
        retval = UNIT_BYTE_EB;
    }
    else if (utilstring_compare(prefix, "EIB", true) ||
             utilstring_compare(prefix, "EI", true))
    {
        retval = UNIT_BYTE_EIB;
    }

    return retval;
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
    uint64_t retval = 0;

    if (utilstring_compare(prefix, "S", true) ||
        utilstring_compare(prefix, "", true))
    {
        retval = UNIT_TIME_SEC;
    }
    else if (utilstring_compare(prefix, "M", true))
    {
        retval = UNIT_TIME_MIN;
    }
    else if (utilstring_compare(prefix, "H", true))
    {
        retval = UNIT_TIME_HOUR;
    }
    else if (utilstring_compare(prefix, "D", true))
    {
        retval = UNIT_TIME_HOUR;
    }
    else if (utilstring_compare(prefix, "W", true))
    {
        retval = UNIT_TIME_WEEK;
    }
    else if (utilstring_compare(prefix, "Y", true))
    {
        retval = UNIT_TIME_YEAR;
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
uint64_t utilunit_getbitrate(const char * const bitrate)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = utilstring_parse(bitrate, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * utilunit_getbitrateprefix(prefix);
            break;
        default:
            // Nothing to do.
            break;
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
uint64_t utilunit_getbytes(const char * const bytes)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = utilstring_parse(bytes, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * utilunit_getbyteprefix(prefix);
            break;
        default:
            // Nothing to do.
            break;
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
uint64_t utilunit_getsecs(const char * const secs)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = utilstring_parse(secs, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * utilunit_getsecsprefix(prefix);
            break;
        default:
            // Nothing to do.
            break;
    }

    return retval;
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
    char retval = '\0';

    switch (prefix)
    {
        case UNIT_SI_KILO:
            retval = 'k';
            break;
        case UNIT_IEC_KIBI:
            retval = 'K';
            break;
        case UNIT_SI_MEGA:
        case UNIT_IEC_MEBI:
            retval = 'M';
            break;
        case UNIT_SI_GIGA:
        case UNIT_IEC_GIBI:
            retval = 'G';
            break;
        case UNIT_SI_TERA:
        case UNIT_IEC_TEBI:
            retval = 'T';
            break;
        case UNIT_SI_PETA:
        case UNIT_IEC_PEBI:
            retval = 'P';
            break;
        case UNIT_SI_EXA:
        case UNIT_IEC_EXBI:
            retval = 'E';
            break;
        default:
            break;
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
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
