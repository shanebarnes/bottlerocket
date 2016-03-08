/**
 * @file   manip_unit.c
 * @author Shane Barnes
 * @date   07 Mar 2016
 * @brief  Unit manipulation implementation.
 */

#include "manip_string.h"
#include "manip_unit.h"
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
static uint64_t manip_unit_get_bitrate_prefix(const char *const prefix)
{
    uint64_t retval = 0;

    if (manip_string_compare(prefix, "BPS", true) ||
        manip_string_compare(prefix, "", true))
    {
        retval = UNITS_BITRATE_BPS;
    }
    else if (manip_string_compare(prefix, "KBPS", true) ||
             manip_string_compare(prefix, "K", true))
    {
        retval = UNITS_BITRATE_KBPS;
    }
    else if (manip_string_compare(prefix, "KIBPS", true) ||
             manip_string_compare(prefix, "KI", true))
    {
        retval = UNITS_BITRATE_KIBPS;
    }
    else if (manip_string_compare(prefix, "MBPS", true) ||
             manip_string_compare(prefix, "M", true))
    {
        retval = UNITS_BITRATE_MBPS;
    }
    else if (manip_string_compare(prefix, "MIBPS", true) ||
             manip_string_compare(prefix, "MI", true))
    {
        retval = UNITS_BITRATE_MIBPS;
    }
    else if (manip_string_compare(prefix, "GBPS", true) ||
             manip_string_compare(prefix, "G", true))
    {
        retval = UNITS_BITRATE_GBPS;
    }
    else if (manip_string_compare(prefix, "GIBPS", true) ||
             manip_string_compare(prefix, "GI", true))
    {
        retval = UNITS_BITRATE_GIBPS;
    }
    else if (manip_string_compare(prefix, "TBPS", true) ||
             manip_string_compare(prefix, "T", true))
    {
        retval = UNITS_BITRATE_TBPS;
    }
    else if (manip_string_compare(prefix, "TIBPS", true) ||
             manip_string_compare(prefix, "TI", true))
    {
        retval = UNITS_BITRATE_TIBPS;
    }
    else if (manip_string_compare(prefix, "PBPS", true) ||
             manip_string_compare(prefix, "P", true))
    {
        retval = UNITS_BITRATE_PBPS;
    }
    else if (manip_string_compare(prefix, "PIBPS", true) ||
             manip_string_compare(prefix, "PI", true))
    {
        retval = UNITS_BITRATE_PIBPS;
    }
    else if (manip_string_compare(prefix, "EBPS", true) ||
             manip_string_compare(prefix, "E", true))
    {
        retval = UNITS_BITRATE_EBPS;
    }
    else if (manip_string_compare(prefix, "EIBPS", true) ||
             manip_string_compare(prefix, "EI", true))
    {
        retval = UNITS_BITRATE_EIBPS;
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
static uint64_t manip_unit_get_byte_prefix(const char * const prefix)
{
    uint64_t retval = 0;

    if (manip_string_compare(prefix, "B", true) ||
        manip_string_compare(prefix, "", true))
    {
        retval = UNITS_BYTE_B;
    }
    else if (manip_string_compare(prefix, "KB", true) ||
             manip_string_compare(prefix, "K", true))
    {
        retval = UNITS_BYTE_KB;
    }
    else if (manip_string_compare(prefix, "KIB", true) ||
             manip_string_compare(prefix, "KI", true))
    {
        retval = UNITS_BYTE_KIB;
    }
    else if (manip_string_compare(prefix, "MB", true) ||
             manip_string_compare(prefix, "M", true))
    {
        retval = UNITS_BYTE_MB;
    }
    else if (manip_string_compare(prefix, "MIB", true) ||
             manip_string_compare(prefix, "MI", true))
    {
        retval = UNITS_BYTE_MIB;
    }
    else if (manip_string_compare(prefix, "GB", true) ||
             manip_string_compare(prefix, "G", true))
    {
        retval = UNITS_BYTE_GB;
    }
    else if (manip_string_compare(prefix, "GIB", true) ||
             manip_string_compare(prefix, "GI", true))
    {
        retval = UNITS_BYTE_GIB;
    }
    else if (manip_string_compare(prefix, "TB", true) ||
             manip_string_compare(prefix, "T", true))
    {
        retval = UNITS_BYTE_TB;
    }
    else if (manip_string_compare(prefix, "TIB", true) ||
             manip_string_compare(prefix, "TI", true))
    {
        retval = UNITS_BYTE_TIB;
    }
    else if (manip_string_compare(prefix, "PB", true) ||
             manip_string_compare(prefix, "P", true))
    {
        retval = UNITS_BYTE_PB;
    }
    else if (manip_string_compare(prefix, "PIB", true) ||
             manip_string_compare(prefix, "PI", true))
    {
        retval = UNITS_BYTE_PIB;
    }
    else if (manip_string_compare(prefix, "EB", true) ||
             manip_string_compare(prefix, "E", true))
    {
        retval = UNITS_BYTE_EB;
    }
    else if (manip_string_compare(prefix, "EIB", true) ||
             manip_string_compare(prefix, "EI", true))
    {
        retval = UNITS_BYTE_EIB;
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
static uint64_t manip_unit_get_seconds_prefix(const char * const prefix)
{
    uint64_t retval = 0;

    if (manip_string_compare(prefix, "S", true) ||
        manip_string_compare(prefix, "", true))
    {
        retval = UNITS_TIME_SEC;
    }
    else if (manip_string_compare(prefix, "M", true))
    {
        retval = UNITS_TIME_MIN;
    }
    else if (manip_string_compare(prefix, "H", true))
    {
        retval = UNITS_TIME_HOUR;
    }
    else if (manip_string_compare(prefix, "D", true))
    {
        retval = UNITS_TIME_HOUR;
    }
    else if (manip_string_compare(prefix, "W", true))
    {
        retval = UNITS_TIME_WEEK;
    }
    else if (manip_string_compare(prefix, "Y", true))
    {
        retval = UNITS_TIME_YEAR;
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
uint64_t manip_unit_get_bitrate(const char * const bitrate)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = manip_string_parse(bitrate, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * manip_unit_get_bitrate_prefix(prefix);
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
uint64_t manip_unit_get_bytes(const char * const bytes)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = manip_string_parse(bytes, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * manip_unit_get_byte_prefix(prefix);
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
uint64_t manip_unit_get_seconds(const char * const seconds)
{
    uint64_t retval = 0;
    int32_t  matchcount = 0;
    char     prefix[80];

    matchcount = manip_string_parse(seconds, "%" PRIu64 "%s", &retval, prefix);

    switch (matchcount)
    {
        case 1:
            // Nothing to do.
           break;
        case 2:
            // @todo Protect against overflow.
            retval = retval * manip_unit_get_seconds_prefix(prefix);
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
static char manip_unit_get_prefix_symbol(const uint64_t prefix)
{
    char retval = '\0';

    switch (prefix)
    {
        case UNITS_SI_KILO:
            retval = 'k';
            break;
        case UNITS_IEC_KIBI:
            retval = 'K';
            break;
        case UNITS_SI_MEGA:
        case UNITS_IEC_MEBI:
            retval = 'M';
            break;
        case UNITS_SI_GIGA:
        case UNITS_IEC_GIBI:
            retval = 'G';
            break;
        case UNITS_SI_TERA:
        case UNITS_IEC_TEBI:
            retval = 'T';
            break;
        case UNITS_SI_PETA:
        case UNITS_IEC_PEBI:
            retval = 'P';
            break;
        case UNITS_SI_EXA:
        case UNITS_IEC_EXBI:
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
void manip_unit_get_decimal_notation(const uint64_t base,
                                     const uint8_t precision,
                                     const uint64_t integer,
                                     char * const buf)
{
    uint64_t parti          = integer;
    uint64_t partf          = 0;
    uint64_t prefix         = UNITS_SI_BASE;
    uint64_t scale          = UNITS_SI_BASE;
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

        while ((parti >= (prefix * scale)) && (prefix <= (uint64_t)(UNITS_SI_PETA)))
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
            manip_string_concat(buf, "%" PRIu64 ".%.*u %c%c",
                                parti,
                                precision > digitcount ?
                                    (uint8_t)digitcount : precision,
                                (uint32_t)partf,
                                manip_unit_get_prefix_symbol(prefix),
                                scale == 1024 ? 'i' : '\0');
        }
        else
        {
            manip_string_concat(buf, "%" PRIu64 " %c%c",
                                parti,
                                manip_unit_get_prefix_symbol(prefix),
                                scale == 1024 ? 'i' : '\0');
        }
    }
}
