/**
 * @file      rwlock_obj.h
 * @brief     Read/write lock object interface.
 * @author    Shane Barnes
 * @date      11 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _RWLOCK_OBJ_H_
#define _RWLOCK_OBJ_H_

#include "system_types.h"

#include <pthread.h>

struct rwlockobj
{
    pthread_rwlock_t lock;
};

/**
 * @brief Create a read/write lock object.
 *
 * @param[in,out] obj A pointer to a read/write object to create.
 *
 * @return True if a read/write lock object was created.
 */
bool rwlockobj_create(struct rwlockobj * const obj);

/**
 * @brief Destroy a read/write lock object.
 *
 * @param[in,out] obj A pointer to a read/write lock object to destroy.
 *
 * @return True if a read/write lock object was destroyed.
 */
bool rwlockobj_destroy(struct rwlockobj * const obj);

/**
 * @brief Block until a read lock is acquired on a read/write lock object.
 *
 * @param[in,out] obj A pointer to a read/write lock object on which to acquire
 *                    a read lock.
 *
 * @return True if a read lock was acquired on the read/write lock object.
 */
bool rwlockobj_rdlock(struct rwlockobj * const obj);

/**
 * @brief Do not block until a read lock is acquired on a read/write lock object
 *        unless the lock can be acquired immediately.
 *
 * @param[in,out] obj A pointer to a read/write lock object on which to attempt
 *                    to acquire a read lock.
 *
 * @return True if a read lock was acquired on the read/write lock object.
 */
bool rwlockobj_tryrdlock(struct rwlockobj * const obj);

/**
 * @brief Block until a write lock is acquired on a read/write lock object.
 *
 * @param[in,out] obj A pointer to a read/write lock object on which to acquire
 *                    a write lock.
 *
 * @return True if a write lock was acquired on the read/write lock object.
 */
bool rwlockobj_wrlock(struct rwlockobj * const obj);

/**
 * @brief Do not block until a write lock is acquired on a read/write lock
 *        object unless the lock can be acquired immediately.
 *
 * @param[in,out] obj A pointer to a read/write lock object on which to attempt
 *                    to acquire a write lock.
 *
 * @return True if a write lock was acquired on the read/write lock object.
 */
bool rwlockobj_trywrlock(struct rwlockobj * const obj);

/**
 * @brief Release a read/write lock.
 *
 * @param[in,out] obj A pointer to the read/write lock object holding a
 *                    read/write lock.
 *
 * @return True if a read/write lock was released from a read/write lock object.
 */
bool rwlockobj_unlock(struct rwlockobj * const obj);

#endif // _RWLOCK_OBJ_H_
