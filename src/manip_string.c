/**
 * @file   manip_string.c
 * @author Shane Barnes
 * @date   07 Mar 2016
 * @brief  String manipulation implementation.
 */

#include "manip_string.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * @see See header file for interace comments.
 */
bool manip_string_compare(const char * const str1,
                          const char * const str2,
                          bool ignorecase)
{
    bool retval = false;

    if ((str1 != NULL) && (str2 != NULL))
    {
        if (ignorecase == true)
        {
            retval = (strcasecmp(str1, str2) == 0);
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
int32_t manip_string_concat(char * const buf,
                            const size_t len,
                            const char * const format,
                            ...)
{
    int32_t retval = -1;
    va_list args;

    va_start(args, format);
    retval = vsnprintf(buf, len, format, args);
    va_end(args);

    return retval;
}

/**
 * @see See header file for interace comments.
 */
int32_t manip_string_parse(const char * const str,
                           const char * const format,
                           ...)
{
    int32_t retval = -1;
    va_list args;

    va_start(args, format);
    retval = vsscanf(str, format, args);
    va_end(args);

    return retval;
}

/**
 * @see See header file for interace comments.
 */
void manip_string_to_lower(char * const str)
{
    uint32_t i = 0;

    if (str != NULL)
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
void manip_string_to_upper(char * const str)
{
    uint32_t i = 0;

    if (str != NULL)
    {
        for (i = 0; i < strlen(str); i++)
        {
            str[i] = toupper((unsigned char)str[i]);
        }
    }
}
