/**
 * @file      fion_poll.h
 * @brief     File I/O event notification poll interface.
 * @author    Shane Barnes
 * @date      15 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _FION_POLL_H_
#define _FION_POLL_H_

#include "fion_obj.h"
#include "system_types.h"

/**
 * @see foo_create() for interface comments.
 */
bool fionpoll_create(struct fionobj * const obj);

/**
 * @see foo_destroy() for interface comments.
 */
bool fionpoll_destroy(struct fionobj * const obj);

/**
 * @see foo_insertfd() for interface comments.
 */
bool fionpoll_insertfd(struct fionobj * const obj, const int32_t fd);

/**
 * @see foo_deletefd() for interface comments.
 */
bool fionpoll_deletefd(struct fionobj * const obj, const int32_t fd);

/**
 * @see foo_setflags() for interface comments.
 */
bool fionpoll_setflags(struct fionobj * const obj);

/**
 * @see foo_poll() for interface comments.
 */
bool fionpoll_poll(struct fionobj * const obj);

#endif // _FION_POLL_H_
