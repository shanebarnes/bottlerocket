/**
 * @file      mutex_obj.c
 * @brief     Mutex object implementation.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "mutex_obj.h"
#include "util_debug.h"

#include <errno.h>

/**
 * @see See header file for interface comments.
 */
bool mutexobj_create(struct mutexobj * const mtx)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(mtx != NULL) == true)
    {
        if (pthread_mutex_init(&mtx->obj, NULL) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create mutex (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool mutexobj_destroy(struct mutexobj * const mtx)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(mtx != NULL) == true)
    {
        if (pthread_mutex_destroy(&mtx->obj) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to destroy mutex (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool mutexobj_lock(struct mutexobj * const mtx)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(mtx != NULL) == true)
    {
        if (pthread_mutex_lock(&mtx->obj) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to lock mutex (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool mutexobj_trylock(struct mutexobj * const mtx)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(mtx != NULL) == true)
    {
        if (pthread_mutex_trylock(&mtx->obj) == 0)
        {
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool mutexobj_unlock(struct mutexobj * const mtx)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(mtx != NULL) == true)
    {
        if (pthread_mutex_unlock(&mtx->obj) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to unlock mutex (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}
