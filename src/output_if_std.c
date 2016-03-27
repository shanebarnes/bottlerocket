/**
 * @file      output_if_std.c
 * @brief     Standard output stream interface implementation.
 * @author    Shane Barnes
 * @date      14 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "output_if_std.h"

#include <stdio.h>

/**
 * @see See header file for interace comments.
 */
int32_t output_if_std_send(void * const buf, const uint32_t len)
{
    int32_t retval = -1;

    if ((buf != NULL) && (len > 0))
    {
        retval = fputs((const char *)buf, stdout);
        fflush(stdout);
    }

    return retval;
}
