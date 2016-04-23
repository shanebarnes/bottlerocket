/**
 * @file      thread_obj.c
 * @brief     Thread object implementation.
 * @author    Shane Barnes
 * @date      10 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "mutex_obj.h"
#include "thread_obj.h"

#include <errno.h>
#include <signal.h>
#include <unistd.h>

struct internals
{
    struct mutexobj mutex;
    bool            shutdown;
};

/**
 * @see See header file for interface comments.
 */
bool threadobj_create(struct threadobj * const obj)
{
    bool retval = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->internal = malloc(sizeof(struct internals));

        if (obj->internal == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate internals (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else if (pthread_attr_init(&obj->attributes) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create thread (%d)\n",
                          __FUNCTION__,
                          errno);
            threadobj_destroy(obj);
        }
        else if (mutexobj_create(&obj->internal->mutex) == false)
        {
            threadobj_destroy(obj);
        }
        else
        {
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_destroy(struct threadobj * const obj)
{
    bool retval = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        //pthread_detach
#if 0
        if (pthread_join(obj->handle, NULL) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to suspend the calling thread (%d)\n",
                          __FUNCTION__,
                          errno);
        }
#endif

        mutexobj_destroy(&obj->internal->mutex);
        pthread_attr_destroy(&obj->attributes);
        free(obj->internal);
        obj->internal = NULL;
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_start(struct threadobj * const obj)
{
    bool retval = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        mutexobj_lock(&obj->internal->mutex);
        obj->internal->shutdown = false;
        mutexobj_unlock(&obj->internal->mutex);

        if (pthread_create(&obj->handle,
                           &obj->attributes,
                           obj->function,
                           obj->argument) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to start thread (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_isrunning(struct threadobj * const obj)
{
    bool retval = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        mutexobj_lock(&obj->internal->mutex);
        retval = !obj->internal->shutdown;
        mutexobj_unlock(&obj->internal->mutex);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_stop(struct threadobj * const obj)
{
    bool retval = false;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        mutexobj_lock(&obj->internal->mutex);
        obj->internal->shutdown = true;
        mutexobj_unlock(&obj->internal->mutex);

        // Wait for thread to exit.
        while (pthread_kill(obj->handle, 0) == 0)
        {
            usleep(10 * 1000);
        }

        retval = true;
    }

    return retval;
}
