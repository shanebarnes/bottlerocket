/**
 * @file      util_debug.c
 * @brief     Debug utility implementation.
 * @author    Shane Barnes
 * @date      21 Jul 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_debug.h"

#if defined(__APPLE__) || defined(__linux__)
    #include <execinfo.h>
#endif
#include <stdio.h>

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

bool utildebug_verify(const bool eval,
                      const char * const expr,
                      const char * const func,
                      const uint32_t line)
{
    bool ret = eval;

    if ((eval == false) && (expr != NULL) && (func != NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "verification failed: func %s, line %u, expr %s\n",
                      func,
                      line,
                      expr);

        //utildebug_backtrace();
    }

    return ret;
}
