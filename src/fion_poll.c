/**
 * @file      fion_poll.c
 * @brief     File I/O event notification poll implementation.
 * @author    Shane Barnes
 * @date      15 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "fion_poll.h"
#include "logger.h"
#include "util_debug.h"
#include "util_mem.h"

#include <errno.h>
#include <poll.h>
#include <sys/socket.h>

/**
 * @see See header file for interface comments.
 */
bool fionpoll_create(struct fionobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (vector_getsize(&obj->fds) == 0)) == true)
    {
        obj->ops.fion_create    = fionpoll_create;
        obj->ops.fion_destroy   = fionpoll_destroy;
        obj->ops.fion_insertfd  = fionpoll_insertfd;
        obj->ops.fion_deletefd  = fionpoll_deletefd;
        obj->ops.fion_setflags  = fionpoll_setflags;
        obj->ops.fion_poll      = fionpoll_poll;
        obj->ops.fion_getevents = fionpoll_getevents;
        obj->timeoutms          = 0;
        obj->pevents            = 0;
        obj->revents            = 0;

        if (vector_create(&obj->fds, 0, sizeof(struct pollfd)) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: vector allocation failed (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_destroy(struct fionobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(obj != NULL) == true)
    {
        vector_destroy(&obj->fds);

        obj->ops.fion_create    = NULL;
        obj->ops.fion_destroy   = NULL;
        obj->ops.fion_insertfd  = NULL;
        obj->ops.fion_deletefd  = NULL;
        obj->ops.fion_setflags  = NULL;
        obj->ops.fion_poll      = NULL;
        obj->ops.fion_getevents = NULL;

        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_insertfd(struct fionobj * const obj, const int32_t fd)
{
    bool ret = false, found = false;
    struct pollfd *pfd = NULL;
    struct pollfd val = {.fd = fd, .events = 0, .revents = 0};
    uint32_t i;

    if (UTILDEBUG_VERIFY(obj != NULL) == true)
    {
        for (i = 0; i < vector_getsize(&obj->fds); i++)
        {
            pfd = (struct pollfd *)vector_getval(&obj->fds, i);

            if (pfd->fd == fd)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: fd %d is already in the list\n",
                              __FUNCTION__,
                              fd);
                found = true;
                break;
            }
        }

        if (found == false)
        {
            if ((vector_inserttail(&obj->fds, &val) == true) &&
                (obj->ops.fion_setflags(obj) == true))
            {
                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_deletefd(struct fionobj * const obj, const int32_t fd)
{
    bool ret = false, found = false;
    struct pollfd *pfd = NULL;
    uint32_t i;

    if (UTILDEBUG_VERIFY(obj != NULL) == true)
    {
        for (i = 0; i < vector_getsize(&obj->fds); i++)
        {
            pfd = (struct pollfd *)vector_getval(&obj->fds, i);

            if (pfd->fd == fd)
            {
                found = true;
                break;
            }
        }

        if (found == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: fd %d is not in the list\n",
                          __FUNCTION__,
                          fd);
        }
        else
        {
            ret = vector_delete(&obj->fds, i);
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_setflags(struct fionobj * const obj)
{
    bool ret = false;
    struct pollfd *pfd = NULL;
    uint32_t i;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (vector_getsize(&obj->fds) > 0)) == true)
    {
        for (i = 0; i < vector_getsize(&obj->fds); i++)
        {
            pfd = (struct pollfd *)vector_getval(&obj->fds, i);
            pfd->events = POLLPRI | POLLERR | POLLHUP | POLLNVAL;
#if defined(__linux__)
            pfd->events |= POLLRDHUP;
#endif
            if (obj->pevents & FIONOBJ_PEVENT_IN)
            {
                pfd->events |= POLLIN;
            }

            if (obj->pevents & FIONOBJ_PEVENT_OUT)
            {
                pfd->events |= POLLOUT;
            }

            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_poll(struct fionobj * const obj)
{
    bool ret = false;
    int32_t err;
    uint32_t i;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (vector_getsize(&obj->fds) > 0)) == true)
    {
        obj->revents = 0;
        err = poll((struct pollfd *)vector_getval(&obj->fds, 0),
                   vector_getsize(&obj->fds),
                   obj->timeoutms);

        if (err == 0)
        {
            // No event on any file descriptor.
            obj->revents = FIONOBJ_REVENT_TIMEOUT;
            ret = true;
        }
        else if (err > 0)
        {
            for (i = 0; i < vector_getsize(&obj->fds); i++)
            {
                obj->revents |= fionpoll_getevents(obj, i);
            }

            ret = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: poll failed (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
uint32_t fionpoll_getevents(struct fionobj * const obj, const uint32_t pos)
{
    uint32_t ret = 0;
    struct pollfd *pfd = NULL;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (pos < vector_getsize(&obj->fds))) == true)
    {
        pfd = (struct pollfd *)vector_getval(&obj->fds, pos);

        // Check for error events.
        if ((pfd->revents & POLLERR) ||
#if defined(__linux__)
            (pfd->revents & POLLRDHUP) ||
#endif
            (pfd->revents & POLLHUP) ||
            (pfd->revents & POLLNVAL))
        {
            ret |= FIONOBJ_REVENT_ERROR;
        }

        // Check for input event.
        if (pfd->revents & POLLIN)
        {
            ret |= FIONOBJ_REVENT_INREADY;
        }

        // Check for output event.
        if (pfd->revents & POLLOUT)
        {
            ret |= FIONOBJ_REVENT_OUTREADY;
        }

        // Assume a timeout if no events received.
        if (ret == 0)
        {
            ret = FIONOBJ_REVENT_TIMEOUT;
        }
    }

    return ret;
}
