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

int32_t output_if_std_send(void * const buf, const uint32_t len)
{
    int32_t retval = -1;

    if ((buf != NULL) && (len > 0))
    {
        retval = fputs((const char *)buf, stdout);
#if defined(__CYGWIN__)
        // Zero is returned on success; otherwise, EOF is returned.
        if ((retval == 0) && (retval != EOF))
        {
            retval = len;
        }
#endif
        if (retval > 0)
        {
            fflush(stdout);
        }
    }

    return retval;
}
