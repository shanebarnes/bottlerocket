/**
 * @file      fion_obj.h
 * @brief     File I/O event notification object interface.
 * @author    Shane Barnes
 * @date      15 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _FION_OBJ_H_
#define _FION_OBJ_H_

#include "system_types.h"

enum fionobj_pevent
{
    FIONOBJ_PEVENT_IN  = 0x01,
    FIONOBJ_PEVENT_OUT = 0x02
};

enum fionobj_revent
{
    FIONOBJ_REVENT_TIMEOUT  = 0x01,
    FIONOBJ_REVENT_ERROR    = 0x02,
    FIONOBJ_REVENT_INREADY  = 0x04,
    FIONOBJ_REVENT_OUTREADY = 0x08
};

struct internals;
struct fionobj;

struct fionobj_ops
{
    /**
     * @brief Create a file I/O event notification object.
     *
     * @param[in,out] obj A pointer to a file I/O event notification object.
     *
     * @return True on success.
     */
    bool (*foo_create)(struct fionobj * const obj);

    /**
     * @brief Destroy a file I/O event notification object.
     *
     * @param[in,out] obj A pointer to a file I/O event notification object.
     *
     * @return True on success.
     */
    bool (*foo_destroy)(struct fionobj * const obj);

    /**
     * @brief Set the file I/O event flags to handle.
     *
     * @param[in,out] obj A pointer to a file I/O event notification object.
     *
     * @return True on sucess.
     */
    bool (*foo_setflags)(struct fionobj * const obj);

    /**
     * @brief Check a file I/O event object for events of interest. The object
     *        will be polled until the file I/O event object timeout is reached
     *        or will block until an event occurs if the file I/O event object
     *        timeout is zero.
     *
     * @param[in,out] obj A pointer to a file I/O event notification object.
     *
     * @return True if the file I/O event object was polled.
     */
    bool (*foo_poll)(struct fionobj * const obj);
};

struct fionobj
{
    struct fionobj_ops  ops;
    int32_t            *fds;
    int32_t             size;
    int32_t             timeoutms;
    uint32_t            pevents;
    uint32_t            revents;
    struct internals   *internal;
};

#endif // _FION_OBJ_H_
