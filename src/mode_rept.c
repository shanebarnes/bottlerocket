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

struct modeobj_priv
{
    struct args_obj args;
};

bool moderept_create(struct modeobj * const mode,
                     const struct args_obj * const args)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) &&
                         (mode->priv == NULL) &&
                         (args != NULL)))
    {
        // Do something.
    }

    return ret;
}

bool moderept_destroy(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        // Do something.
    }

    return ret;
}

bool moderept_start(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        // Do something.
    }

    return ret;
}

bool moderept_stop(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        // Do something.
    }

    return ret;
}

bool moderept_cancel(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        // Do something.
    }

    return ret;
}
