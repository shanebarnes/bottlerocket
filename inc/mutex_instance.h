/**
 * @file      mutex_instance.h
 * @brief     Mutex instance interface.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _MUTEX_INSTANCE_H_
#define _MUTEX_INSTANCE_H_

#include "system_types.h"

#include <pthread.h>

struct mutex_instance
{
    pthread_mutex_t mutex;
};

/**
 * @brief Create a mutex instance.
 *
 * @param[in,out] instance A pointer to a mutex instance to create.
 *
 * @return True if a mutex instance was created.
 */
bool mutex_instance_create(struct mutex_instance * const instance);

/**
 * @brief Destroy a mutex instance.
 *
 * @param[in,out] instance A pointer to a mutex instance to destroy.
 *
 * @return True if a mutex instance was destroyed.
 */
bool mutex_instance_destroy(struct mutex_instance * const instance);

/**
 * @brief Block until a mutex instance is locked.
 *
 * @param[in,out] instance A pointer to a mutex instance to lock.
 *
 * @return True if a mutex instance was locked.
 */
bool mutex_instance_lock(struct mutex_instance * const instance);

/**
 * @brief Attempt to lock a mutex instance without blocking.
 *
 * @param[in,out] instance A pointer to a mutex instance to lock.
 *
 * @return True if a mutex instance was locked.
 */
bool mutex_instance_trylock(struct mutex_instance * const instance);

/**
 * @brief Unlock a mutex instance.
 *
 * @param[in,out] instance A pointer to the mutex instance to unlock.
 *
 * @return True if a mutex instance was unlocked.
 */
bool mutex_instance_unlock(struct mutex_instance * const instance);

#endif // _MUTEX_INSTANCE_H_
