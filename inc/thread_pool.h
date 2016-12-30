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
#include "thread_obj.h"

struct threadpool_priv;

struct threadpool
{
    struct threadpool_priv *priv;
};

/**
 * @brief Create a thread pool.
 *
 * @param[in,out] pool A pointer to a thread pool to create.
 * @param[in]     size The number of worker threads to create.
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
 * @brief Execute a task (i.e., a function with arguments) concurrently in a
 *        thread pool worker thread.
 *
 * @param[in,out] pool A pointer to a thread pool.
 * @param[in]     func A pointer to a task function.
 * @param[in]     arg  A pointer to a task function argument.
 * @param[in]     id   A task identifier.
 *
 * @return True if a task was executed in a thread pool worker thread.
 */
bool threadpool_execute(struct threadpool * const pool,
                        void * const func,
                        void * const arg,
                        const uint32_t id);

/**
 * @brief Wait (block) until a specified number of tasks are completed by a
 *        thread pool.
 *
 * @param[in,out] pool       A pointer to a thread pool.
 * @param[in]     wait_count The number of tasks required to unblock the wait.
 *
 * @return True if the specified number of tasks were completed by a thread
 *         pool.
 */
bool threadpool_wait(struct threadpool * const pool, const uint32_t wait_count);

/**
 * @brief Wake a threadpool_wait call.
 *
 * @param[in,out] pool A pointer to a thread pool.
 *
 * @return True if a threadpool_wait call was unblocked.
 */
bool threadpool_wake(struct threadpool * const pool);

/**
 * @brief Get the thread pool task id of the calling thread.
 *
 * @param[in] pool A pointer to a thread pool.
 *
 * @return A thread pool task id of the calling thread.
 */
uint32_t threadpool_getid(struct threadpool * const pool);

/**
 * @brief Get the thread pool thread handle of the calling thread.
 *
 * @param[in] pool A pointer to a thread pool.
 *
 * @return A pointer to a thread handle (null pointer on error).
 */
struct threadobj* threadpool_getthread(struct threadpool * const pool);

/**
 * @brief Check if a thread pool is running.
 *
 * @param[in] pool A pointer to a thread pool.
 *
 * @return True if a thread pool is running.
 */
bool threadpool_isrunning(struct threadpool * const pool);

/**
 * @brief Get the number of tasks being executed by a thread pool.
 *
 * @param[in] pool A pointer to a thread pool.
 *
 * @return The number of tasks being executed by a thread pool.
 */
uint32_t threadpool_getexeccount(struct threadpool * const pool);

/**
 * @brief Get the number of tasks being executed or waiting to be executed by  a
 *        thread pool.
 *
 * @param[in] pool A pointer to a thread pool.
 *
 * @return The number of tasks being executed or waiting to be executed by a
 *         thread pool.
 */
uint32_t threadpool_gettaskcount(struct threadpool * const pool);

/**
 * @brief Get the number of running threads in a thread pool.
 *
 * @param[in] pool A pointer to a thread pool.
 *
 * @return The number of running threads in a thread pool.
 */
uint32_t threadpool_getthreadcount(struct threadpool * const pool);

/**
 * @brief Get the number of tasks waiting to be executed by a thread pool.
 *
 * @param[in] pool A pointer to a thread pool.
 *
 * @return The number of tasks waiting to be executed by a thread pool.
 */
uint32_t threadpool_getwaitcount(struct threadpool * const pool);

#endif // _THREAD_POOL_H_
