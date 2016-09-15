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
#include "util_debug.h"
#include "util_mem.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

struct threadobj_priv
{
    char            name[64];
    pthread_t       handle;
    pthread_attr_t  attributes;
    struct mutexobj mutex;
    bool            shutdown;
};

/**
 * @see See header file for interface comments.
 */
bool threadobj_create(struct threadobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->priv == NULL)) == true)
    {
        obj->priv = UTILMEM_CALLOC(struct threadobj_priv,
                                   sizeof(struct threadobj_priv),
                                   1);

        if (obj->priv == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate private memory (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else if (pthread_attr_init(&obj->priv->attributes) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create thread (%d)\n",
                          __FUNCTION__,
                          errno);
            threadobj_destroy(obj);
        }
        else if (mutexobj_create(&obj->priv->mutex) == false)
        {
            threadobj_destroy(obj);
        }
        else
        {
            obj->priv->shutdown = true;
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_destroy(struct threadobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->priv) != NULL) == true)
    {
        //pthread_detach
#if 0
        if (pthread_join(obj->priv->handle, NULL) == 0)
        {
            ret = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to suspend the calling thread (%d)\n",
                          __FUNCTION__,
                          errno);
        }
#endif

        threadobj_stop(obj);
        mutexobj_destroy(&obj->priv->mutex);
        pthread_attr_destroy(&obj->priv->attributes);
        UTILMEM_FREE(obj->priv);
        obj->priv = NULL;
        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_start(struct threadobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->priv != NULL)) == true)
    {
        mutexobj_lock(&obj->priv->mutex);
        obj->priv->shutdown = false;
        mutexobj_unlock(&obj->priv->mutex);

        if (pthread_create(&obj->priv->handle,
                           &obj->priv->attributes,
                           obj->function,
                           obj->argument) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to start thread (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_stop(struct threadobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->priv != NULL)) == true)
    {
        mutexobj_lock(&obj->priv->mutex);
        obj->priv->shutdown = true;
        mutexobj_unlock(&obj->priv->mutex);

        // Wait for thread to exit.
        while (pthread_kill(obj->priv->handle, 0) == 0)
        {
            usleep(10 * 1000);
        }

        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_isrunning(struct threadobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->priv != NULL)) == true)
    {
        mutexobj_lock(&obj->priv->mutex);
        ret = !obj->priv->shutdown;
        mutexobj_unlock(&obj->priv->mutex);
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool threadobj_join(struct threadobj * const obj)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY((obj != NULL) && (obj->priv != NULL)) == true)
    {
        err = pthread_join(obj->priv->handle, NULL);

        if (err != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to suspend the calling thread (%d)\n",
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
