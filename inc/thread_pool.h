/**
 * @file      thread_pool.h
 * @brief     Thread pool interface.
 * @author    Shane Barnes
 * @date      14 Sep 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "system_types.h"

struct threadpool_priv;

struct threadpool
{
    struct threadpool_priv *priv;
};

/**
 * @brief Create a thread pool.
 *
 * @param[in,out] pool A pointer to a thread pool to create.
 * @param[in]     size The number of worker threads to create..
 *
 * @return True if a thread pool was created.
 */
bool threadpool_create(struct threadpool * pool, const uint32_t size);

/**
 * @brief Destroy a thread pool.
 *
 * @param[in,out] pool A pointer to a thread pool to destroy.
 *
 * @return True if a thread pool was destroyed.
 */
bool threadpool_destroy(struct threadpool * const pool);

/**
 * @brief Start a thread pool.
 *
 * @param[in,out] pool A pointer to a thread pool to start.
 *
 * @return True if a thread pool was started.
 */
bool threadpool_start(struct threadpool * const pool);

/**
 * @brief Stop a thread pool.
 *
 * @param[in,out] pool A pointer to a thread pool to stop.
 *
 * @return True if a thread pool was stopped.
 */
bool threadpool_stop(struct threadpool * const pool);

/**
 * @brief Check if a thread pool is running.
 *
 * @param[in] obj A pointer to a thread pool.
 *
 * @return True if a thread pool is running.
 */
bool threadpool_isrunning(struct threadpool * const pool);

/**
 * @brief Execute a task (i.e., a function with arguments) concurrently in a
 *        thread pool worker thread.
 *
 * @param[in,out] pool A pointer to a thread pool.
 * @param[in]     func A pointer to a task function.
 * @param[in]     arg  A pointer to a task function argument.
 *
 * @return True if a task was executed in a thread pool worker thread.
 */
bool threadpool_execute(struct threadpool * const pool,
                        void * const func,
                        void * const arg);

#endif // _THREAD_POOL_H_
