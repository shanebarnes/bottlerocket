/**
 * @file      thread_pool.c
 * @brief     Thread pool implementation.
 * @author    Shane Barnes
 * @date      14 Sep 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "cv_obj.h"
#include "mutex_obj.h"
#include "thread_pool.h"
#include "util_debug.h"
#include "util_mem.h"
#include "vector.h"

#include <errno.h>
#include <string.h>

struct threadpool_priv
{
    struct cvobj    cv_task;
    struct cvobj    cv_wait;
    struct mutexobj mtx;
    struct vector   threads;
    struct vector   tasks;
    uint32_t        startup;
    uint32_t        running;
    uint32_t        busy;
    uint32_t        complete;
    uint32_t        wait;
    bool            shutdown;
};

struct threadpool_task
{
    void (*func)();
    void *arg;
    uint32_t id;
};

struct threadpool_thread
{
    struct threadobj handle;
    uint32_t         taskid;
};

/**
 * @brief Set the thread pool task id of the calling thread.
 *
 * @param[in,out] pool   A pointer to a thread pool to create.
 * @param[in]     taskid A task id to associated with the calling thread.
 *
 * @return True if a thread pool task id was associated with the calling thread.
 */
static bool threadpool_setid(struct threadpool * const pool,
                             const uint32_t taskid)
{
    bool ret = false;
    uint64_t threadid = threadobj_getcallerid();
    struct threadpool_thread *thread = NULL;
    uint32_t i;

    for (i = 0; i < vector_getsize(&pool->priv->threads); i++)
    {
        thread = vector_getval(&pool->priv->threads, i);

        if (threadid == threadobj_getthreadid(&thread->handle))
        {
            thread->taskid = taskid;
            ret = true;
            break;
        }
    }

    return ret;
}

/**
 * @brief Thread pool task thread that waits to be signaled when a new task is
 *        available to be executed.
 *
 * @param[in,out] arg A pointer to a thread pool.
 */
static void *threadpool_taskthread(void *arg)
{
    struct threadpool *pool = (struct threadpool*)arg;
    struct threadpool_task task, *temp = NULL;

    mutexobj_lock(&pool->priv->mtx);
    pool->priv->startup--;

    while (!pool->priv->shutdown)
    {
        task.func = NULL;

        temp = vector_gettail(&pool->priv->tasks);
        if (temp != NULL)
        {
            task.func = temp->func;
            task.arg = temp->arg;
            threadpool_setid(pool, temp->id);
            vector_deletetail(&pool->priv->tasks);

            pool->priv->busy++;
            mutexobj_unlock(&pool->priv->mtx);
            (*task.func)(task.arg);
            mutexobj_lock(&pool->priv->mtx);
            pool->priv->busy--;
            pool->priv->complete++;

            if ((pool->priv->wait > 0) &&
                (pool->priv->complete >= pool->priv->wait))
            {
                mutexobj_unlock(&pool->priv->mtx);
                cvobj_signalall(&pool->priv->cv_wait);
                mutexobj_lock(&pool->priv->mtx);
            }
        }

        if ((!pool->priv->shutdown) &&
            (vector_getsize(&pool->priv->tasks) == 0))
        {
            cvobj_wait(&pool->priv->cv_task, &pool->priv->mtx);
        }
    }

    pool->priv->running--;
    mutexobj_unlock(&pool->priv->mtx);

    return NULL;
}

bool threadpool_create(struct threadpool * pool, const uint32_t size)
{
    bool ret = false;
    uint32_t i = 0;
    struct threadpool_thread *thread = NULL;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv == NULL) && (size > 0)))
    {
        pool->priv = UTILMEM_CALLOC(struct threadpool_priv,
                                    sizeof(struct threadpool_priv),
                                    1);

        if (pool->priv == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate private memory (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else if (!vector_create(&pool->priv->threads,
                                size,
                                sizeof(struct threadpool_thread)))
        {
            threadpool_destroy(pool);
        }
        else if (!vector_create(&pool->priv->tasks,
                                0,
                                sizeof(struct threadpool_task)))
        {
            threadpool_destroy(pool);
        }
        else if (!cvobj_create(&pool->priv->cv_task))
        {
            threadpool_destroy(pool);
        }
        else if (!cvobj_create(&pool->priv->cv_wait))
        {
            threadpool_destroy(pool);
        }
        else if (!mutexobj_create(&pool->priv->mtx))
        {
            threadpool_destroy(pool);
        }
        else
        {
            for (i = 0; i < vector_getsize(&pool->priv->threads); i++)
            {
                thread = vector_getval(&pool->priv->threads, i);
                memset(thread, 0, sizeof(*thread));

                if (!threadobj_create(&thread->handle))
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: failed to create thread #%u\n",
                                  __FUNCTION__,
                                  i);
                }
                else if (!threadobj_init(&thread->handle,
                                         threadpool_taskthread,
                                         pool))
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: failed to initialize thread #%u\n",
                                  __FUNCTION__,
                                  i);
                }
            }

            if (i != vector_getsize(&pool->priv->threads))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to create thread pool\n",
                              __FUNCTION__);
                threadpool_destroy(pool);
            }
            else
            {
                pool->priv->shutdown = true;
                ret = true;
            }
        }
    }

    return ret;
}

bool threadpool_destroy(struct threadpool * const pool)
{
    bool ret = false;
    uint32_t i = 0;
    struct threadpool_thread *thread = NULL;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        threadpool_stop(pool);
        mutexobj_destroy(&pool->priv->mtx);
        cvobj_destroy(&pool->priv->cv_wait);
        cvobj_destroy(&pool->priv->cv_task);
        vector_destroy(&pool->priv->tasks);

        for (i = 0; i < vector_getsize(&pool->priv->threads); i++)
        {
            thread = vector_getval(&pool->priv->threads, i);

            if (!threadobj_destroy(&thread->handle))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to destroy thread #%u\n",
                              __FUNCTION__,
                              i);
            }
        }

        vector_destroy(&pool->priv->threads);
        UTILMEM_FREE(pool->priv);
        pool->priv = NULL;
        ret = true;
    }

    return ret;
}

bool threadpool_start(struct threadpool * const pool)
{
    bool ret = false;
    uint32_t i = 0;
    struct threadpool_thread *thread = NULL;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        pool->priv->shutdown = false;
        pool->priv->startup = 0;
        pool->priv->running = 0;
        pool->priv->busy = 0;
        pool->priv->complete = 0;
        pool->priv->wait = 0;
        mutexobj_unlock(&pool->priv->mtx);

        for (i = 0; i < vector_getsize(&pool->priv->threads); i++)
        {
            mutexobj_lock(&pool->priv->mtx);
            pool->priv->startup++;
            pool->priv->running++;
            mutexobj_unlock(&pool->priv->mtx);

            thread = vector_getval(&pool->priv->threads, i);
            if (!threadobj_start(&thread->handle))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to start thread #%u\n",
                              __FUNCTION__,
                              i);
                mutexobj_lock(&pool->priv->mtx);
                pool->priv->startup--;
                pool->priv->running--;
                mutexobj_unlock(&pool->priv->mtx);
            }
            else
            {
                ret = true;
            }
        }
    }

    return ret;
}

bool threadpool_stop(struct threadpool * const pool)
{
    bool ret = false;
    uint32_t i = 0;
    struct threadpool_thread *thread = NULL;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        pool->priv->shutdown = true;
        mutexobj_unlock(&pool->priv->mtx);
        cvobj_signalall(&pool->priv->cv_task);

        for (i = 0; i < vector_getsize(&pool->priv->threads); i++)
        {
            thread = vector_getval(&pool->priv->threads, i);

            if (!threadobj_stop(&thread->handle))
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to stop thread #%u\n",
                              __FUNCTION__,
                              i);
            }
            else
            {
                ret = true;
            }
        }

        while (threadpool_getthreadcount(pool) > 0)
        {
            threadobj_sleepusec(1000);
        }

        cvobj_signalall(&pool->priv->cv_wait);
    }

    return ret;
}

bool threadpool_execute(struct threadpool * const pool,
                        void * const func,
                        void * const arg,
                        const uint32_t id)
{
    bool ret = false;
    struct threadpool_task task = {.func = func, .arg = arg, .id = id};

    if (UTILDEBUG_VERIFY((pool != NULL) &&
                         (pool->priv != NULL) &&
                         (func != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        while (pool->priv->startup > 0)
        {
            mutexobj_unlock(&pool->priv->mtx);
            threadobj_sleepusec(1000);
            mutexobj_lock(&pool->priv->mtx);

        }
        mutexobj_unlock(&pool->priv->mtx);

        // @todo Replace vector with circular queue.
        mutexobj_lock(&pool->priv->mtx);
        ret = vector_inserttail(&pool->priv->tasks, &task);
        mutexobj_unlock(&pool->priv->mtx);

        cvobj_signalone(&pool->priv->cv_task);
    }

    return ret;
}

bool threadpool_wait(struct threadpool * const pool, const uint32_t wait_count)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((pool != NULL) &&
                         (pool->priv != NULL) &&
                         (wait_count > 0)))
    {
        mutexobj_lock(&pool->priv->mtx);

        if (pool->priv->wait > 0)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: thread pool failed to wait (already waiting)\n",
                          __FUNCTION__);
        }
        else
        {
            pool->priv->wait = wait_count;
            cvobj_wait(&pool->priv->cv_wait, &pool->priv->mtx);

            if (pool->priv->complete >= wait_count)
            {
                ret = true;
            }

            pool->priv->wait = 0;
        }

        mutexobj_unlock(&pool->priv->mtx);
    }

    return ret;
}

bool threadpool_wake(struct threadpool * const pool)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        ret = cvobj_signalall(&pool->priv->cv_wait);
    }

    return ret;
}

uint32_t threadpool_getid(struct threadpool * const pool)
{
    uint32_t ret = 0;
    uint64_t threadid = threadobj_getcallerid();
    struct threadpool_thread *thread = NULL;
    uint32_t i;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        for (i = 0; i < vector_getsize(&pool->priv->threads); i++)
        {
            thread = vector_getval(&pool->priv->threads, i);

            if (threadid == threadobj_getthreadid(&thread->handle))
            {
                ret = thread->taskid;
                break;
            }
        }
    }

    return ret;
}

struct threadobj *threadpool_getthread(struct threadpool * const pool)
{
    struct threadobj *ret = NULL;
    uint64_t threadid = threadobj_getcallerid();
    struct threadpool_thread *thread = NULL;
    uint32_t i;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        for (i = 0; i < vector_getsize(&pool->priv->threads); i++)
        {
            thread = vector_getval(&pool->priv->threads, i);

            if (threadid == threadobj_getthreadid(&thread->handle))
            {
                ret = &thread->handle;
                break;
            }
        }
    }

    return ret;
}

bool threadpool_isrunning(struct threadpool * const pool)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        ret = !pool->priv->shutdown;
        mutexobj_unlock(&pool->priv->mtx);
    }

    return ret;
}

uint32_t threadpool_getexeccount(struct threadpool * const pool)
{
    uint32_t ret = 0;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        ret = pool->priv->busy;
        mutexobj_unlock(&pool->priv->mtx);
    }

    return ret;
}

uint32_t threadpool_gettaskcount(struct threadpool * const pool)
{
    uint32_t ret = 0;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        ret = pool->priv->busy + vector_getsize(&pool->priv->tasks);
        mutexobj_unlock(&pool->priv->mtx);
    }

    return ret;
}

uint32_t threadpool_getthreadcount(struct threadpool * const pool)
{
    uint32_t ret = 0;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        ret = pool->priv->running;
        mutexobj_unlock(&pool->priv->mtx);
    }

    return ret;
}

uint32_t threadpool_getwaitcount(struct threadpool * const pool)
{
    uint32_t ret = 0;

    if (UTILDEBUG_VERIFY((pool != NULL) && (pool->priv != NULL)))
    {
        mutexobj_lock(&pool->priv->mtx);
        ret = vector_getsize(&pool->priv->tasks);
        mutexobj_unlock(&pool->priv->mtx);
    }

    return ret;
}
