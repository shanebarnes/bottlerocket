/**
 * @file   output_if_std.c
 * @author Shane Barnes
 * @date   14 Mar 2016
 * @brief  Standard output stream interface implementation.
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
        retval = fputs(buf, stdout);
    }

    return retval;
}
