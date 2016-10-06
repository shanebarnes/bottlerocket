/**
 * @file      util_string.c
 * @brief     String utility implementation.
 * @author    Shane Barnes
 * @date      07 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_debug.h"
#include "util_string.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool utilstring_compare(const char * const str1,
                        const char * const str2,
                        const size_t len,
                        bool ignorecase)
{
    bool retval = false;

    if (!UTILDEBUG_VERIFY((str1 != NULL) && (str2 != NULL)))
    {
        // Do nothing.
    }
    else if (ignorecase)
    {
        if (len != 0)
        {
            retval = (strncasecmp(str1, str2, len) == 0);
        }
        else
        {
            retval = (strcasecmp(str1, str2) == 0);
        }
    }
    else
    {
        if (len != 0)
        {
            retval = (strncmp(str1, str2, len) == 0);
        }
        else
        {
            retval = (strcmp(str1, str2) == 0);
        }
    }

    return retval;
}

int32_t utilstring_concat(char * const buf,
                          const size_t len,
                          const char * const format,
                          ...)
{
    int32_t retval = -1;
    va_list args;

    if (UTILDEBUG_VERIFY((buf != NULL) && (len > 0) && (format != NULL)))
    {
        va_start(args, format);
        retval = vsnprintf(buf, len, format, args);
        va_end(args);

        if ((retval > 0) && ((size_t)retval > len))
        {
            retval = len;
        }
    }

    return retval;
}

int32_t utilstring_parse(const char * const str, const char * const format, ...)
{
    int32_t retval = -1;
    va_list args;

    if (UTILDEBUG_VERIFY((str != NULL) && (format != NULL)))
    {
        va_start(args, format);
        retval = vsscanf(str, format, args);
        va_end(args);
    }

    return retval;
}

void utilstring_tolower(char * const str)
{
    uint32_t i = 0;

    if (UTILDEBUG_VERIFY(str != NULL))
    {
        for (i = 0; i < strlen(str); i++)
        {
            str[i] = tolower((unsigned char)str[i]);
        }
    }
}

void utilstring_toupper(char * const str)
{
    uint32_t i = 0;

    if (UTILDEBUG_VERIFY(str != NULL))
    {
        for (i = 0; i < strlen(str); i++)
        {
            str[i] = toupper((unsigned char)str[i]);
        }
    }
}

bool utilstring_fromi32(const int32_t num, char * const str, const size_t len)
{
    bool retval = false;
    int32_t errval = 0;

    if (UTILDEBUG_VERIFY((str != NULL) && (len > 0)))
    {
        errval = snprintf(str, len, "%d", num);

        if ((errval == 0) || (errval >= (int32_t)len))
        {
            //logger_printf(LOGGER_LEVEL_ERROR,
            //              "%s: '%d' could not be converted to a string of size %d\n",
            //              __FUNCTION__,
            //              num,
            //              len);
        }
        else
        {
            retval = true;
        }
    }

    return retval;
}
