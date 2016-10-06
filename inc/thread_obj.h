/**
 * @file      thread_obj.h
 * @brief     Thread object interface.
 * @author    Shane Barnes
 * @date      10 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _THREAD_OBJ_H_
#define _THREAD_OBJ_H_

#include "system_types.h"

struct threadobj_priv;

struct threadobj
{
    struct threadobj_priv *priv;
};

/**
 * @brief Create a thread object.
 *
 * @param[in,out] thread A pointer to a thread object to create.
 *
 * @return True if a thread object was created.
 */
bool threadobj_create(struct threadobj * const thread);

/**
 * @brief Destroy a thread object.
 *
 * @param[in,out] thread A pointer to a thread object to destroy.
 *
 * @return True if a thread object was destroyed.
 */
bool threadobj_destroy(struct threadobj * const thread);

/**
 * @brief Initialize a thread object with a start routine and argument.
 *
 * @param[in,out] thread A pointer to a thread object to initialize.
 * @param[in]     func   A pointer to a start routine.
 * @param[in]     arg    A pointer to an argument.
 *
 * @return True if a thread object was initialized.
 */
bool threadobj_init(struct threadobj * const thread, void *func, void *arg);

/**
 * @brief Start a thread object.
 *
 * @param[in,out] thread A pointer to a thread object to start.
 *
 * @return True if a thread obj was started.
 */
bool threadobj_start(struct threadobj * const thread);

/**
 * @brief Stop a thread object.
 *
 * @param[in,out] thread A pointer to a thread object to stop.
 *
 * @return True if a thread object was stopped.
 */
bool threadobj_stop(struct threadobj * const thread);

/**
 * @brief Get a thread object's thread id.
 *
 * @param[in] thread A pointer to a thread object.
 *
 * @return A thread object's thread id (0 on error).
 */
uint64_t threadobj_getthreadid(struct threadobj * const thread);

/**
 * @brief Get a calling thread's id.
 *
 * @return A calling thread's id.
 */
uint64_t threadobj_getcallerid(void);

/**
 * @brief Suspend thread execution for a specified amounf of time in
 *        microseconds.
 *
 * @param[in] interval Amount of time in microseconds to suspend thread
 *                     execution.
 *
 * @return True if thread execution was suspended for a specified amount of time
 *         in microseconds.
 */
bool threadobj_sleepusec(const int32_t interval);

/**
 * @brief Check if a thread object is running.
 *
 * @param[in] thread A pointer to a thread object.
 *
 * @return True if a thread object is running.
 */
bool threadobj_isrunning(struct threadobj * const thread);

/**
 * @brief Suspend thread object caller.
 *
 * @param[in] thread A pointer to a thread object.
 *
 * @return True if thread object caller was suspended.
 */
bool threadobj_join(struct threadobj * const thread);

#endif // _THREAD_OBJ_H_
