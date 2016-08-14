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
 * @see fion_create() for interface comments.
 */
bool fionpoll_create(struct fionobj * const obj);

/**
 * @see fion_destroy() for interface comments.
 */
bool fionpoll_destroy(struct fionobj * const obj);

/**
 * @see fion_insertfd() for interface comments.
 */
bool fionpoll_insertfd(struct fionobj * const obj, const int32_t fd);

/**
 * @see fion_deletefd() for interface comments.
 */
bool fionpoll_deletefd(struct fionobj * const obj, const int32_t fd);

/**
 * @see fion_setflags() for interface comments.
 */
bool fionpoll_setflags(struct fionobj * const obj);

/**
 * @see fion_poll() for interface comments.
 */
bool fionpoll_poll(struct fionobj * const obj);

/**
 * @see fion_getevents() for interface comments.
 */
uint32_t fionpoll_getevents(struct fionobj * const obj, const uint32_t pos);

#endif // _FION_POLL_H_
