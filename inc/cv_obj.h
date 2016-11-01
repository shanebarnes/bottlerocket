/**
 * @file      cv_obj.h
 * @brief     Condition variable object interface.
 * @author    Shane Barnes
 * @date      09 Sep 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _CV_OBJ_H_
#define _CV_OBJ_H_

#include "mutex_obj.h"
#include "system_types.h"

struct cvobj_priv;

struct cvobj
{
    struct cvobj_priv *priv;
};

/**
 * @brief Create a condition variable object.
 *
 * @param[in,out] cv A pointer to a condition variable object to create.
 *
 * @return True if a condition variable object was created.
 */
bool cvobj_create(struct cvobj * const cv);

/**
 * @brief Destroy a condition variable object.
 *
 * @param[in,out] cv A pointer to a condition variable object to destroy.
 *
 * @return True if a condition variable object was destroyed.
 */
bool cvobj_destroy(struct cvobj * const cv);

/**
 * @brief Unblock all threads waiting on a condition variable.
 *
 * @param[in,out] cv A pointer to a condition variable.
 *
 * @return True if all threads waiting on a condition variable were unblocked.
 */
bool cvobj_signalall(struct cvobj * const cv);

/**
 * @brief Unblock at least one thread waiting on a condition variable.
 *
 * @param[in,out] cv A pointer to a condition variable.
 *
 * @return True if a thread waiting on a condition variable was unblocked.
 */
bool cvobj_signalone(struct cvobj * const cv);

/**
 * @brief Unlock a mutex and block on a condition variable object.
 *
 * @param[in,out] cv  A pointer to a condition variable object.
 * @param[in,out] mtx A pointer to a mutex object.
 *
 * @return True if a condition variable object waited on a condition.
 */
bool cvobj_wait(struct cvobj * const cv, struct mutexobj * const mtx);

/**
 * @brief Unlock a mutex and block on a condition variable object. The blocking
 *        wait will not last longer than the amount of time specified by the
 *        timeout.
 *
 * @param[in,out] cv        A pointer to a condition variable object.
 * @param[in,out] mtx       A pointer to a mutex object.
 * @param[in,out] timeoutus The maximum amount of time to wait in microseconds.
 *
 * @return True if a condition variable object waited on a condition.
 */
bool cvobj_timedwait(struct cvobj * const cv,
                     struct mutexobj * const mtx,
                     const uint32_t timeoutus);

#endif // _CV_OBJ_H_
