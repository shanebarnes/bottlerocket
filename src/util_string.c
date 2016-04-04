/**
 * @file      util_string.c
 * @brief     String utility implementation.
 * @author    Shane Barnes
 * @date      07 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_string.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * @see See header file for interace comments.
 */
bool utilstring_compare(const char * const str1,
                        const char * const str2,
                        const size_t len,
                        bool ignorecase)
{
    bool retval = false;

    if ((str1 == NULL) || (str2 == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (ignorecase == true)
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

/**
 * @see See header file for interace comments.
 */
int32_t utilstring_concat(char * const buf,
                          const size_t len,
                          const char * const format,
                          ...)
{
    int32_t retval = -1;
    va_list args;

    if ((buf == NULL) || (len == 0) || (format == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
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

/**
 * @see See header file for interace comments.
 */
int32_t utilstring_parse(const char * const str,
                         const char * const format,
                         ...)
{
    int32_t retval = -1;
    va_list args;

    if ((str == NULL) || (format == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        va_start(args, format);
        retval = vsscanf(str, format, args);
        va_end(args);
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
void utilstring_tolower(char * const str)
{
    uint32_t i = 0;

    if (str == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 0; i < strlen(str); i++)
        {
            str[i] = tolower((unsigned char)str[i]);
        }
    }
}

/**
 * @see See header file for interace comments.
 */
void utilstring_toupper(char * const str)
{
    uint32_t i = 0;

    if (str == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 0; i < strlen(str); i++)
        {
            str[i] = toupper((unsigned char)str[i]);
        }
    }
}
