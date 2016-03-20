/**
 * @file   util_unit.c
 * @author Shane Barnes
 * @date   07 Mar 2016
 * @brief  Unit utility implementation.
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
static uint64_t util_unit_get_bitrate_prefix(const char *const prefix)
{
    uint64_t retval = 0;

    if (util_string_compare(prefix, "BPS", true) ||
        util_string_compare(prefix, "", true))
    {
        retval = UNIT_BITRATE_BPS;
    }
    else if (util_string_compare(prefix, "KBPS", true) ||
             util_string_compare(prefix, "K", true))
    {
        retval = UNIT_BITRATE_KBPS;
    }
    else if (util_string_compare(prefix, "KIBPS", true) ||
             util_string_compare(prefix, "KI", true))
    {
        retval = UNIT_BITRATE_KIBPS;
    }
    else if (util_string_compare(prefix, "MBPS", true) ||
             util_string_compare(prefix, "M", true))
    {
        retval = UNIT_BITRATE_MBPS;
    }
    else if (util_string_compare(prefix, "MIBPS", true) ||
             util_string_compare(prefix, "MI", true))
    {
        retval = UNIT_BITRATE_MIBPS;
    }
    else if (util_string_compare(prefix, "GBPS", true) ||
             util_string_compare(prefix, "G", true))
    {
        retval = UNIT_BITRATE_GBPS;
    }
    else if (util_string_compare(prefix, "GIBPS", true) ||
             util_string_compare(prefix, "GI", true))
    {
        retval = UNIT_BITRATE_GIBPS;
    }
    else if (util_string_compare(prefix, "TBPS", true) ||
             util_string_compare(prefix, "T", true))
    {
        retval = UNIT_BITRATE_TBPS;
    }
    else if (util_string_compare(prefix, "TIBPS", true) ||
             util_string_compare(prefix, "TI", true))
    {
        retval = UNIT_BITRATE_TIBPS;
    }
    else if (util_string_compare(prefix, "PBPS", true) ||
             util_string_compare(prefix, "P", true))
    {
        retval = UNIT_BITRATE_PBPS;
    }
    else if (util_string_compare(prefix, "PIBPS", true) ||
             util_string_compare(prefix, "PI", true))
    {
        retval = UNIT_BITRATE_PIBPS;
    }
    else if (util_string_compare(prefix, "EBPS", true) ||
             util_string_compare(prefix, "E", true))
    {
        retval = UNIT_BITRATE_EBPS;
    }
    else if (util_string_compare(prefix, "EIBPS", true) ||
             util_string_compare(prefix, "EI", true))
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
static uint64_t util_unit_get_byte_prefix(const char * const prefix)
{
    uint64_t retval = 0;

    if (util_string_compare(prefix, "B", true) ||
        util_string_compare(prefix, "", true))
    {
        retval = UNIT_BYTE_B;
    }
    else if (util_string_compare(prefix, "KB", true) ||
             util_string_compare(prefix, "K", true))
    {
        retval = UNIT_BYTE_KB;
    }
    else if (util_string_compare(prefix, "KIB", true) ||
             util_string_compare(prefix, "KI", true))
    {
        retval = UNIT_BYTE_KIB;
    }
    else if (util_string_compare(prefix, "MB", true) ||
             util_string_compare(prefix, "M", true))
    {
        retval = UNIT_BYTE_MB;
    }
    else if (util_string_compare(prefix, "MIB", true) ||
             util_string_compare(prefix, "MI", true))
    {
        retval = UNIT_BYTE_MIB;
    }
    else if (util_string_compare(prefix, "GB", true) ||
             util_string_compare(prefix, "G", true))
    {
        retval = UNIT_BYTE_GB;
    }
    else if (util_string_compare(prefix, "GIB", true) ||
             util_string_compare(prefix, "GI", true))
    {
        retval = UNIT_BYTE_GIB;
    }
    else if (util_string_compare(prefix, "TB", true) ||
             util_string_compare(prefix, "T", true))
    {
        retval = UNIT_BYTE_TB;
    }
    else if (util_string_compare(prefix, "TIB", true) ||
             util_string_compare(prefix, "TI", true))
    {
        retval = UNIT_BYTE_TIB;
    }
    else if (util_string_compare(prefix, "PB", true) ||
             util_string_compare(prefix, "P", true))
    {
        retval = UNIT_BYTE_PB;
    }
    else if (util_string_compare(prefix, "PIB", true) ||
             util_string_compare(prefix, "PI", true))
    {
        retval = UNIT_BYTE_PIB;
    }
    else if (util_string_compare(prefix, "EB", true) ||
             util_string_compare(prefix, "E", true))
    {
        retval = UNIT_BYTE_EB;
    }
    else if (util_string_compare(prefix, "EIB", true) ||
             util_string_compare(prefix, "EI", true))
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
static uint64_t util_unit_get_seconds_prefix(const char * const prefix)
{
    uint64_t retval = 0;

    if (util_string_compare(prefix, "S", true) ||
        util_string_compare(prefix, "", true))
    {
        retval = UNIT_TIME_SEC;
    }
    else if (util_string_compare(prefix, "M", true))
    {
        retval = UNIT_TIME_MIN;
    }
    else if (util_string_compare(prefix, "H", true))
    {
        retval = UNIT_TIME_HOUR;
    }
    else if (util_string_compare(prefix, "D", true))
    {
        retval = UNIT_TIME_HOUR;
    }
    else if (util_string_compare(prefix, "W", true))
    {
        retval = UNIT_TIME_WEEK;
    }
    else if (util_string_compare(prefix, "Y", true))
    {
        retval = UNIT_TIME_YEAR;
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
uint64_t util_unit_get_bitrate(const char * const bitrate)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = util_string_parse(bitrate, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * util_unit_get_bitrate_prefix(prefix);
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
uint64_t util_unit_get_bytes(const char * const bytes)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = util_string_parse(bytes, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * util_unit_get_byte_prefix(prefix);
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
uint64_t util_unit_get_seconds(const char * const seconds)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = util_string_parse(seconds, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * util_unit_get_seconds_prefix(prefix);
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
static char util_unit_get_prefix_symbol(const uint64_t prefix)
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
void util_unit_get_decimal_notation(const uint64_t base,
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
            util_string_concat(buf,
                               len,
                               "%" PRIu64 ".%.*u %c%c",
                               parti,
                               precision > digitcount ?
                                   (uint8_t)digitcount : precision,
                               (uint32_t)partf,
                               util_unit_get_prefix_symbol(prefix),
                               scale == 1024 ? 'i' : '\0');
        }
        else
        {
            util_string_concat(buf,
                               len,
                               "%" PRIu64 " %c%c",
                               parti,
                               util_unit_get_prefix_symbol(prefix),
                               scale == 1024 ? 'i' : '\0');
        }
    }
}
