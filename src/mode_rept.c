/**
 * @file      mode_rept.c
 * @brief     Repeater mode implementation.
 * @author    Shane Barnes
 * @date      06 Oct 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "mode_rept.h"
#include "util_debug.h"

static struct args_obj *opts = NULL;

bool moderept_init(struct args_obj * const args)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(args != NULL))
    {
        opts = args;
        ret = true;
    }

    return ret;
}

bool moderept_start(void)
{
    return false;
}

bool moderept_stop(void)
{
    return false;
}

bool moderept_cancel(void)
{
    return false;
}
