/**
 * @file   threads_instance.h
 * @author Shane Barnes
 * @date   10 Mar 2016
 * @brief  thread instance interface.
 */

#ifndef _THREADS_INSTANCE_H_
#define _THREADS_INSTANCE_H_

#include "system_types.h"

#include <pthread.h>

struct internals;

struct thread_instance
{
    char              name[64];
    pthread_t         handle;
    pthread_attr_t    attributes;
    void             *function;
    void             *argument;
    struct internals *internal;
};

/**
 * @brief Create a thread instance.
 *
 * @param[in,out] instance A pointer to a thread instance to create.
 *
 * @return True if the thread instance was created.
 */
bool thread_instance_create(struct thread_instance * const instance);

/**
 * @brief Destroy a thread instance.
 *
 * @param[in,out] instance A pointer to a thread instance to destroy.
 *
 * @return True if the thread instance was destroyed.
 */
bool thread_instance_destroy(struct thread_instance * const instance);

/**
 * @brief Start a thread instance.
 *
 * @param[in,out] instance A pointer to a thread instance to start.
 *
 * @return True if the thread instance was started.
 */
bool thread_instance_start(struct thread_instance * const instance);

/**
 * @brief Check if the thread instance is running.
 *
 * @param[in] instance A pointer to a thread instance.
 *
 * @return True if the thread instance is running.
 */
bool thread_instance_isrunning(struct thread_instance * const instance);

/**
 * @brief Stop a thread instance.
 *
 * @param[in,out] instance A pointer to a thread instance to stop.
 *
 * @return True if the thread instance was stopped.
 */
bool thread_instance_stop(struct thread_instance * const instance);

#endif // _THREAD_INSTANCE_H_
