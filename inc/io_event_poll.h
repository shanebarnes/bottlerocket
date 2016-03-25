/**
 * @file      io_event_poll.h
 * @brief     Poll I/O event notification interface.
 * @author    Shane Barnes
 * @date      15 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _IO_EVENT_POLL_H_
#define _IO_EVENT_POLL_H_

#include "io_event_instance.h"
#include "system_types.h"

/**
 * @see ieo_create() for interface comments.
 */
bool io_event_poll_create(struct io_event_instance * const instance);

/**
 * @see ieo_destroy() for interface comments.
 */
bool io_event_poll_destroy(struct io_event_instance * const instance);

/**
 * @see ieo_setflags() for interface comments.
 */
bool io_event_poll_setflags(struct io_event_instance * const instance);

/**
 * @see ieo_poll() for interface comments.
 */
bool io_event_poll_poll(struct io_event_instance * const instance);

#endif // _IO_EVENT_POLL_H_
