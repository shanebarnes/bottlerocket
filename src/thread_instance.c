/**
 * @file      thread_instance.c
 * @brief     Thread instance implementation.
 * @author    Shane Barnes
 * @date      10 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "mutex_instance.h"
#include "thread_instance.h"

#include <errno.h>
#include <signal.h>
#include <unistd.h>

struct internals
{
    struct mutex_instance mutex;
    bool                  shutdown;
};

/**
 * @see See header file for interface comments.
 */
bool thread_instance_create(struct thread_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        instance->internal = malloc(sizeof(struct internals));

        if (instance->internal == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to allocate internals (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else if (pthread_attr_init(&instance->attributes) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to create thread (%d)\n",
                          __FUNCTION__,
                          errno);
            thread_instance_destroy(instance);
        }
        else if (mutex_instance_create(&instance->internal->mutex) == false)
        {
            thread_instance_destroy(instance);
        }
        else
        {
            retval = true;
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: Instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool thread_instance_destroy(struct thread_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        //pthread_detach
#if 0
        if (pthread_join(instance->handle, NULL) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to suspend the calling thread (%d)\n",
                          __FUNCTION__,
                          errno);
        }
#endif

        mutex_instance_destroy(&instance->internal->mutex);
        pthread_attr_destroy(&instance->attributes);
        free(instance->internal);
        instance->internal = NULL;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: Instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool thread_instance_start(struct thread_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        mutex_instance_lock(&instance->internal->mutex);
        instance->internal->shutdown = false;
        mutex_instance_unlock(&instance->internal->mutex);

        if (pthread_create(&instance->handle,
                           &instance->attributes,
                           instance->function,
                           instance->argument) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to start thread (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: Instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool thread_instance_isrunning(struct thread_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        mutex_instance_lock(&instance->internal->mutex);
        retval = instance->internal->shutdown;
        mutex_instance_unlock(&instance->internal->mutex);
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: Instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool thread_instance_stop(struct thread_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        mutex_instance_lock(&instance->internal->mutex);
        instance->internal->shutdown = true;
        mutex_instance_unlock(&instance->internal->mutex);

        // Wait for thread to exit.
        while (pthread_kill(instance->handle, 0) == 0)
        {
            usleep(10 * 1000);
        }

        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: Instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}
