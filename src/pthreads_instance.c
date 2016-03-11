/**
 * @file   pthreads_instance.c
 * @author Shane Barnes
 * @date   10 Mar 2016
 * @brief  Pthreads instance implementation.
 */

#include "logger.h"
#include "pthreads_instance.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @see See header file for interface comments.
 */
bool pthreads_instance_create(struct pthreads_instance * const instance)
{
    bool retval = false;
    int32_t error = 0;

    if (instance != NULL)
    {
        if ((error = pthread_attr_init(&instance->attributes)) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to create thread (%d)\n",
                          __FUNCTION__,
                          error);
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
bool pthreads_instance_destroy(struct pthreads_instance * const instance)
{
    bool retval = false;
    int error = 0;

    if (instance != NULL)
    {
        //pthread_detach
#if 0
        if ((error = pthread_join(instance->handle, NULL)) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to suspend the calling thread (%d)\n",
                          __FUNCTION__,
                          error);
        }
#endif
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
bool pthreads_instance_start(struct pthreads_instance * const instance)
{
    bool retval = false;
    int error = 0;

    if (instance != NULL)
    {
        instance->shutdown = false;
        if ((error = pthread_create(&instance->handle,
                                    &instance->attributes,
                                    instance->function,
                                    instance->argument)) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to start thread (%d)\n",
                          __FUNCTION__,
                          error);
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
bool pthreads_instance_stop(struct pthreads_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        instance->shutdown = true;

        // Wait for thread to exit...
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
