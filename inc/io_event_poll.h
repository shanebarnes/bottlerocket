/**
 * @file   io_event_poll.h
 * @author Shane Barnes
 * @date   15 Mar 2016
 * @brief  Poll I/O event notification interface.
 */

#ifndef _IO_EVENT_POLL_H_
#define _IO_EVENT_POLL_H_

#include "io_event_api.h"
#include "system_types.h"

/**
 * @see io_event_api.h for interface comments.
 */
bool io_event_poll_create(struct io_event_instance * const instance);

/**
 * @see io_event_api.h for interface comments.
 */
bool io_event_poll_destroy(struct io_event_instance * const instance);

/**
 * @see io_event_api.h for interface comments.
 */
bool io_event_poll_poll(struct io_event_instance * const instance);

#endif // _IO_EVENT_POLL_H_
