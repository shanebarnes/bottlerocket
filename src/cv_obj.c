/**
 * @file      cv_obj.c
 * @brief     Condition variable object implementation.
 * @author    Shane Barnes
 * @date      09 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "cv_obj.h"
#include "logger.h"
#include "util_debug.h"

#include <errno.h>

/**
 * @see See header file for interface comments.
 */
bool cvobj_create(struct cvobj * const cv)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(cv != NULL) == true)
    {
        if (pthread_cond_init(&cv->obj, NULL) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create condition variable (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool cvobj_destroy(struct cvobj * const cv)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(cv != NULL) == true)
    {
        if (pthread_cond_destroy(&cv->obj) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to destroy condition variable (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool cvobj_signalall(struct cvobj * const cv)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(cv != NULL) == true)
    {
        if (pthread_cond_broadcast(&cv->obj) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to broadcast condition variable(%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool cvobj_signalone(struct cvobj * const cv)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(cv != NULL) == true)
    {
        if (pthread_cond_signal(&cv->obj) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to signal condition variable(%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool cvobj_wait(struct cvobj * const cv, struct mutexobj * const mtx)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY((cv != NULL) && (mtx != NULL)) == true)
    {
        if (pthread_cond_wait(&cv->obj, &mtx->obj) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to block on condition variable (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}
