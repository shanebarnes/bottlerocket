/**
 * @file   io_event_instance.h
 * @author Shane Barnes
 * @date   15 Mar 2016
 * @brief  I/O event notification instance interface.
 */

#ifndef _IO_EVENT_INSTANCE_H_
#define _IO_EVENT_INSTANCE_H_

#include "system_types.h"

enum io_event_poll
{
    IO_EVENT_POLL_IN  = 0x01,
    IO_EVENT_POLL_OUT = 0x02
};

enum io_event_ret
{
    IO_EVENT_RET_TIMEOUT  = 0x01,
    IO_EVENT_RET_ERROR    = 0x02,
    IO_EVENT_RET_INREADY  = 0x04,
    IO_EVENT_RET_OUTREADY = 0x08
};

struct internals;

struct io_event_instance
{
    int32_t          *fds;
    int32_t           size;
    uint32_t          timeoutms;
    uint32_t          pevents;
    uint32_t          revents;
    struct internals *internal;
};

struct io_event_ops
{
    /**
     * @brief Create an I/O event notification instance.
     *
     * @param[in,out] instance A pointer to an I/O event notification instance.
     *
     * @return True on success.
     */
    bool (*ieo_create)(struct io_event_instance * const instance);

    /**
     * @brief Destroy an I/O event notification instance.
     *
     * @param[in,out] instance A pointer to an I/O event notification instance.
     *
     * @return True on success.
     */
    bool (*ieo_destroy)(struct io_event_instance * const instance);

    /**
     * @brief Check an I/O event instance for events of interest. The instance
     *        will be polled until the I/O event instance timeout is reached or
     *        will block until an event occurs if the I/O event instance timeout
     *        is zero.
     *
     * @param[in,out] instance A pointer to an I/O event notification instance.
     *
     * @return True if the I/O event instance was polled.
     */
    bool (*ieo_poll)(struct io_event_instance * const instance);
};

#endif // _IO_EVENT_INSTANCE_H_
