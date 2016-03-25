/**
 * @file      rwlock_instance.h
 * @brief     Read/write lock instance interface.
 * @author    Shane Barnes
 * @date      11 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _RWLOCK_INSTANCE_H_
#define _RWLOCK_INSTANCE_H_

#include "system_types.h"

#include <pthread.h>

struct rwlock_instance
{
    pthread_rwlock_t lock;
};

/**
 * @brief Create a read/write lock instance.
 *
 * @param[in,out] instance A pointer to a read/write instance to create.
 *
 * @return True if the read/write lock instance was created.
 */
bool rwlock_instance_create(struct rwlock_instance * const instance);

/**
 * @brief Destroy a read/write lock instance.
 *
 * @param[in,out] instance A pointer to a read/write lock instance to destroy.
 *
 * @return True if the read/write lock instance was destroyed.
 */
bool rwlock_instance_destroy(struct rwlock_instance * const instance);

/**
 * @brief Block until a read lock is acquired on a read/write lock instance.
 *
 * @param[in,out] instance A pointer to a read/write lock instance on which to
 *                         acquire a read lock.
 *
 * @return True if a read lock was acquired on the read/write lock instance.
 */
bool rwlock_instance_rdlock(struct rwlock_instance * const instance);

/**
 * @brief Do not block until a read lock is acquired on a read/write lock
 *        instance unless the lock can be acquired immediately.
 *
 * @param[in,out] instance A pointer to a read/write lock instance on which to
 *                         attempt to acquire a read lock.
 *
 * @return True if a read lock was acquired on the read/write lock instance.
 */
bool rwlock_instance_tryrdlock(struct rwlock_instance * const instance);

/**
 * @brief Block until a write lock is acquired on a read/write lock instance.
 *
 * @param[in,out] instance A pointer to a read/write lock instance on which to
 *                         acquire a write lock.
 *
 * @return True if a write lock was acquired on the read/write lock instance.
 */
bool rwlock_instance_wrlock(struct rwlock_instance * const instance);

/**
 * @brief Do not block until a write lock is acquired on a read/write lock
 *        instance unless the lock can be acquired immediately.
 *
 * @param[in,out] instance A pointer to a read/write lock instance on which to
 *                         attempt to acquire a write lock.
 *
 * @return True if a write lock was acquired on the read/write lock instance.
 */
bool rwlock_instance_trywrlock(struct rwlock_instance * const instance);

/**
 * @brief Release a read/write lock.
 *
 * @param[in,out] instance A pointer to the read/write lock instance holding a
 *                         a read/write lock.
 *
 * @return True if a read/write lock was released from a read/write lock
 *         instance.
 */
bool rwlock_instance_unlock(struct rwlock_instance * const instance);

#endif // _RWLOCK_INSTANCE_H_
