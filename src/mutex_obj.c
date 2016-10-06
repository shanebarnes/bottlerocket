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

bool mutexobj_create(struct mutexobj * const mtx)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY(mtx != NULL))
    {
        err = pthread_mutex_init(&mtx->obj, NULL);

        if (err != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create mutex (%d)\n",
                          __FUNCTION__,
                          err);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool mutexobj_destroy(struct mutexobj * const mtx)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY(mtx != NULL))
    {
        err = pthread_mutex_destroy(&mtx->obj);

        if (err != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to destroy mutex (%d)\n",
                          __FUNCTION__,
                          err);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool mutexobj_lock(struct mutexobj * const mtx)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY(mtx != NULL))
    {
        err = pthread_mutex_lock(&mtx->obj);

        if (err != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to lock mutex (%d) %p\n",
                          __FUNCTION__,
                          err, mtx);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool mutexobj_trylock(struct mutexobj * const mtx)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(mtx != NULL))
    {
        if (pthread_mutex_trylock(&mtx->obj) == 0)
        {
            ret = true;
        }
    }

    return ret;
}

bool mutexobj_unlock(struct mutexobj * const mtx)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY(mtx != NULL))
    {
        err = pthread_mutex_unlock(&mtx->obj);

        if (err != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to unlock mutex (%d)\n",
                          __FUNCTION__,
                          err);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}
