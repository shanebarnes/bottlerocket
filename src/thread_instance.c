/**
 * @file   thread_instance.c
 * @author Shane Barnes
 * @date   10 Mar 2016
 * @brief  Thread instance implementation.
 */

#include "logger.h"
#include "rwlock_instance.h"
#include "thread_instance.h"

#include <errno.h>
#include <unistd.h>

struct internals
{
    struct rwlock_instance lock;
    bool                   shutdown;
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
        }
        else if (rwlock_instance_create(&instance->internal->lock) == false)
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

        rwlock_instance_destroy(&instance->internal->lock);
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
        rwlock_instance_wrlock(&instance->internal->lock);
        instance->internal->shutdown = false;
        rwlock_instance_unlock(&instance->internal->lock);

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
        rwlock_instance_rdlock(&instance->internal->lock);
        retval = instance->internal->shutdown;
        rwlock_instance_unlock(&instance->internal->lock);
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
        rwlock_instance_wrlock(&instance->internal->lock);
        instance->internal->shutdown = true;
        rwlock_instance_unlock(&instance->internal->lock);

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
