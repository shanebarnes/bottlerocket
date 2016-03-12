/**
 * @file   rwlock_instance.c
 * @author Shane Barnes
 * @date   11 Mar 2016
 * @brief  Read/write lock instance implementation.
 */

#include "logger.h"
#include "rwlock_instance.h"

#include <errno.h>

/**
 * @see See header file for interface comments.
 */
bool rwlock_instance_create(struct rwlock_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_rwlock_init(&instance->lock, NULL) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to create lock (%d)\n",
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
bool rwlock_instance_destroy(struct rwlock_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_rwlock_destroy(&instance->lock) == 0)
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
bool rwlock_instance_rdlock(struct rwlock_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_rwlock_rdlock(&instance->lock) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to acquire read lock (%d)\n",
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
bool rwlock_instance_tryrdlock(struct rwlock_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_rwlock_tryrdlock(&instance->lock) == 0)
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
bool rwlock_instance_wrlock(struct rwlock_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_rwlock_wrlock(&instance->lock) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to acquire write lock (%d)\n",
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
bool rwlock_instance_trywrlock(struct rwlock_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_rwlock_trywrlock(&instance->lock) == 0)
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
bool rwlock_instance_unlock(struct rwlock_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if (pthread_rwlock_unlock(&instance->lock) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: Failed to release a read/write lock (%d)\n",
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
