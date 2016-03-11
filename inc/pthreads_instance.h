/**
 * @file   pthreads_instance.h
 * @author Shane Barnes
 * @date   10 Mar 2016
 * @brief  Pthreads instance interface.
 */

#ifndef _PTHREADS_INSTANCE_H_
#define _PTHREADS_INSTANCE_H_

#include "system_types.h"

#include <pthread.h>

struct pthreads_instance
{
    pthread_t       handle;
    pthread_attr_t  attributes;
    void           *function;
    void           *argument;
    volatile bool   shutdown;
};

/**
 * @brief Create a pthreads instance.
 *
 * @param[in,out] instance A pointer to a pthreads instance to create.
 *
 * @return True if the pthreads instance was created.
 */
bool pthreads_instance_create(struct pthreads_instance * const instance);

/**
 * @brief Destroy a pthreads instance.
 *
 * @param[in,out] instance A pointer to a pthreads instance to destroy.
 *
 * @return True if the pthreads instance was destroyed.
 */
bool pthreads_instance_destroy(struct pthreads_instance * const instance);

/**
 * @brief Start a pthreads instance.
 *
 * @param[in] instance A pointer to a pthreads instance to start.
 *
 * @return True if the pthreads instance was started.
 */
bool pthreads_instance_start(struct pthreads_instance * const instance);

/**
 * @brief Stop a pthreads instance.
 *
 * @param[in] instance A pointer to a pthreads instance to stop.
 *
 * @return True if the pthreads instance was stopped.
 */
bool pthreads_instance_stop(struct pthreads_instance * const instance);

#endif // _PTHREADS_INSTANCE_H_
