/**
 * @file      mutex_instance.c
 * @brief     Mutex instance implementation.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "mutex_instance.h"

#include <errno.h>

/**
 * @see See header file for interface comments.
 */
bool mutex_instance_create(struct mutex_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_mutex_init(&instance->mutex, NULL) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to create mutex (%d)\n",
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
bool mutex_instance_destroy(struct mutex_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_mutex_destroy(&instance->mutex) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to destroy lock (%d)\n",
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
bool mutex_instance_lock(struct mutex_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_mutex_lock(&instance->mutex) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to lock mutex (%d)\n",
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
bool mutex_instance_trylock(struct mutex_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_mutex_trylock(&instance->mutex) == 0)
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
bool mutex_instance_unlock(struct mutex_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_mutex_unlock(&instance->mutex) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to unlock mutex (%d)\n",
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
