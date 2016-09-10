/**
 * @file      mutex_obj.h
 * @brief     Mutex object interface.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _MUTEX_OBJ_H_
#define _MUTEX_OBJ_H_

#include "system_types.h"

#include <pthread.h>

struct mutexobj
{
    pthread_mutex_t obj;
};

/**
 * @brief Create a mutex object.
 *
 * @param[in,out] mtx A pointer to a mutex object to create.
 *
 * @return True if a mutex object was created.
 */
bool mutexobj_create(struct mutexobj * const mtx);

/**
 * @brief Destroy a mutex object.
 *
 * @param[in,out] mtx A pointer to a mutex object to destroy.
 *
 * @return True if a mutex object was destroyed.
 */
bool mutexobj_destroy(struct mutexobj * const mtx);

/**
 * @brief Block until a mutex object is locked.
 *
 * @param[in,out] mtx A pointer to a mutex object to lock.
 *
 * @return True if a mutex object was locked.
 */
bool mutexobj_lock(struct mutexobj * const mtx);

/**
 * @brief Attempt to lock a mutex object without blocking.
 *
 * @param[in,out] mtx A pointer to a mutex object to lock.
 *
 * @return True if a mutex object was locked.
 */
bool mutexobj_trylock(struct mutexobj * const mtx);

/**
 * @brief Unlock a mutex object.
 *
 * @param[in,out] mtx A pointer to a mutex object to unlock.
 *
 * @return True if a mutex object was unlocked.
 */
bool mutexobj_unlock(struct mutexobj * const mtx);

#endif // _MUTEX_OBJ_H_
