/**
 * @file      util_debug.c
 * @brief     Debug utility implementation.
 * @author    Shane Barnes
 * @date      21 Jul 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "system_types.h"
#include "util_debug.h"

#if defined(__APPLE__) || defined(__linux__)
    #include <execinfo.h>
#endif
#include <stdio.h>

/**
 * @see See header file for interface comments.
 */
void utildebug_backtrace(void)
{
#if defined(__APPLE__) || defined(__linux__)
    void *callstack[128];
    int32_t i, frames;
    char **strs = NULL;

    frames = backtrace(callstack, 128);
    strs = backtrace_symbols(callstack, frames);

    for (i = 0; i < frames; ++i)
    {
        fprintf(stdout, "%s\n", strs[i]);
    }

    free(strs);
#endif
}
