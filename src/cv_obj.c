/**
 * @file      cv_obj.c
 * @brief     Condition variable object implementation.
 * @author    Shane Barnes
 * @date      09 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "cv_obj.h"
#include "logger.h"
#include "util_date.h"
#include "util_debug.h"
#include "util_mem.h"
#include "util_unit.h"

#include <errno.h>
#include <pthread.h>

struct cvobj_priv
{
    pthread_cond_t handle;
};

bool cvobj_create(struct cvobj * const cv)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY((cv != NULL) && (cv->priv == NULL)))
    {
        cv->priv = UTILMEM_CALLOC(struct cvobj_priv,
                                  sizeof(struct cvobj_priv),
                                  1);

        if (cv->priv == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate private memory (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else if ((err = pthread_cond_init(&cv->priv->handle, NULL)) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create condition variable (%d)\n",
                          __FUNCTION__,
                          err);
            cvobj_destroy(cv);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool cvobj_destroy(struct cvobj * const cv)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((cv != NULL) && (cv->priv != NULL)))
    {
        pthread_cond_destroy(&cv->priv->handle);
        UTILMEM_FREE(cv->priv);
        cv->priv = NULL;
        ret = true;
    }

    return ret;
}

bool cvobj_signalall(struct cvobj * const cv)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY((cv != NULL) && (cv->priv != NULL)))
    {
        if ((err = pthread_cond_broadcast(&cv->priv->handle)) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to broadcast condition variable(%d)\n",
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

bool cvobj_signalone(struct cvobj * const cv)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY((cv != NULL) && (cv->priv != NULL)))
    {
        if ((err = pthread_cond_signal(&cv->priv->handle)) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to signal condition variable(%d)\n",
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

bool cvobj_wait(struct cvobj * const cv, struct mutexobj * const mtx)
{
    bool ret = false;
    int32_t err = 0;

    if (UTILDEBUG_VERIFY((cv != NULL) && (cv->priv != NULL) && (mtx != NULL)))
    {
        if ((err = pthread_cond_wait(&cv->priv->handle, &mtx->obj)) != 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to block on condition variable (%d)\n",
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

bool cvobj_timedwait(struct cvobj * const cv,
                     struct mutexobj * const mtx,
                     const uint32_t timeoutus)
{
    bool ret = false;
    int32_t err = 0;
    uint64_t tsus = 0;
    struct timespec ts;

    if (UTILDEBUG_VERIFY((cv != NULL) && (cv->priv != NULL) && (mtx != NULL)))
    {
        tsus = utildate_gettstime(DATE_CLOCK_REALTIME, UNIT_TIME_USEC);
        tsus += timeoutus;
        ts.tv_sec = tsus / UNIT_TIME_USEC;
        ts.tv_nsec = (tsus - ts.tv_sec * UNIT_TIME_USEC) * 1000;

        err = pthread_cond_timedwait(&cv->priv->handle, &mtx->obj, &ts);

        if ((err != 0) && (err != ETIMEDOUT))
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to block on condition variable (%d)\n",
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
