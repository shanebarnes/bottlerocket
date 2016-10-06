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
    char             name[64];
    uint32_t         id;
    pthread_t        handle;
    pthread_attr_t   attr;
    void            *func;
    void            *arg;
    struct mutexobj  mutex;
    bool             shutdown;
};

bool threadobj_create(struct threadobj * const thread)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv == NULL)))
    {
        thread->priv = UTILMEM_CALLOC(struct threadobj_priv,
                                      sizeof(struct threadobj_priv),
                                      1);

        if (thread->priv == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate private memory (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else if (pthread_attr_init(&thread->priv->attr) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create thread (%d)\n",
                          __FUNCTION__,
                          errno);
            threadobj_destroy(thread);
        }
        else if (pthread_attr_setstacksize(&thread->priv->attr, 64 * 1024) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to set thread stack size (%d)\n",
                          __FUNCTION__,
                          errno);
            threadobj_destroy(thread);
        }
        else if (!mutexobj_create(&thread->priv->mutex))
        {
            threadobj_destroy(thread);
        }
        else
        {
            thread->priv->shutdown = true;
            ret = true;
        }
    }

    return ret;
}

bool threadobj_destroy(struct threadobj * const thread)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv != NULL)))
    {
        threadobj_stop(thread);
        mutexobj_destroy(&thread->priv->mutex);
        pthread_attr_destroy(&thread->priv->attr);
        UTILMEM_FREE(thread->priv);
        thread->priv = NULL;
        ret = true;
    }

    return ret;
}

bool threadobj_init(struct threadobj * const thread, void *func, void *arg)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv != NULL)))
    {
        thread->priv->func = func;
        thread->priv->arg  = arg;
        ret = true;
    }

    return ret;
}

bool threadobj_start(struct threadobj * const thread)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv != NULL)))
    {
        mutexobj_lock(&thread->priv->mutex);
        thread->priv->shutdown = false;
        mutexobj_unlock(&thread->priv->mutex);

        if (pthread_create(&thread->priv->handle,
                           &thread->priv->attr,
                           thread->priv->func,
                           thread->priv->arg) != 0)
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

bool threadobj_stop(struct threadobj * const thread)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv != NULL)))
    {
        mutexobj_lock(&thread->priv->mutex);
        thread->priv->shutdown = true;
        mutexobj_unlock(&thread->priv->mutex);

        while (pthread_kill(thread->priv->handle, 0) == 0)
        {
            threadobj_sleepusec(10 * 1000);
        }

        ret = true;
    }

    return ret;
}

uint32_t threadobj_getthreadid(struct threadobj * const thread)
{
    uint32_t ret = 0;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv != NULL)))
    {
        ret = (uint32_t)thread->priv->handle;
    }

    return ret;
}

uint32_t threadobj_getcallerid(void)
{
    return (uint32_t)pthread_self();
}

bool threadobj_sleepusec(const int32_t interval)
{
    bool ret = true;

    if (usleep(interval) != 0)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to suspend thread execution (%d)\n",
                      __FUNCTION__,
                      errno);
        ret = false;
    }

    return ret;
}

bool threadobj_isrunning(struct threadobj * const thread)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv != NULL)))
    {
        mutexobj_lock(&thread->priv->mutex);
        ret = !thread->priv->shutdown;
        mutexobj_unlock(&thread->priv->mutex);
    }

    return ret;
}

bool threadobj_join(struct threadobj * const thread)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY((thread != NULL) && (thread->priv != NULL)))
    {
        err = pthread_join(thread->priv->handle, NULL);

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
