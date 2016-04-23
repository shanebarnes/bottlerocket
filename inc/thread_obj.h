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

#include <pthread.h>

struct internals;

struct threadobj
{
    char              name[64];
    pthread_t         handle;
    pthread_attr_t    attributes;
    void             *function;
    void             *argument;
    struct internals *internal;
};

/**
 * @brief Create a thread object.
 *
 * @param[in,out] obj A pointer to a thread object to create.
 *
 * @return True if a thread object was created.
 */
bool threadobj_create(struct threadobj * const obj);

/**
 * @brief Destroy a thread object.
 *
 * @param[in,out] obj A pointer to a thread object to destroy.
 *
 * @return True if a thread object was destroyed.
 */
bool threadobj_destroy(struct threadobj * const obj);

/**
 * @brief Start a thread object.
 *
 * @param[in,out] obj A pointer to a thread object to start.
 *
 * @return True if a thread obj was started.
 */
bool threadobj_start(struct threadobj * const obj);

/**
 * @brief Check if a thread object is running.
 *
 * @param[in] obj A pointer to a thread object.
 *
 * @return True if a thread object is running.
 */
bool threadobj_isrunning(struct threadobj * const obj);

/**
 * @brief Stop a thread object.
 *
 * @param[in,out] obj A pointer to a thread object to stop.
 *
 * @return True if a thread object was stopped.
 */
bool threadobj_stop(struct threadobj * const obj);

#endif // _THREAD_OBJ_H_
