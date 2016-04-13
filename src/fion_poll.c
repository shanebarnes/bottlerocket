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

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>

struct internals
{
    struct pollfd *pfds;
};

/**
 * @see See header file for interface comments.
 */
bool fionpoll_create(struct fionobj * const obj)
{
    bool retval = false;

    if ((obj == NULL) || (obj->internal != NULL) || (obj->size != 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->ops.foo_create   = fionpoll_create;
        obj->ops.foo_destroy  = fionpoll_destroy;
        obj->ops.foo_setflags = fionpoll_setflags;
        obj->ops.foo_poll     = fionpoll_poll;

        obj->fds       = 0;
        obj->size      = 1;
        obj->timeoutms = 0;
        obj->pevents   = 0;
        obj->internal  = malloc(sizeof(struct internals));

        if (obj->internal == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: memory allocation failed (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            obj->internal->pfds = malloc(obj->size * sizeof(struct pollfd));

            if (obj->internal->pfds == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: memory allocation failed (%d)\n",
                              __FUNCTION__,
                              errno);
            }
            else
            {
                retval = true;
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_destroy(struct fionobj * const obj)
{
    bool retval = false;

    if ((obj == NULL) || (obj->internal == NULL) || (obj->size <= 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (obj->internal->pfds != NULL)
        {
            free(obj->internal->pfds);
            obj->internal->pfds = NULL;
        }

        free(obj->internal);
        obj->internal = NULL;

        obj->ops.foo_create   = NULL;
        obj->ops.foo_destroy  = NULL;
        obj->ops.foo_setflags = NULL;
        obj->ops.foo_poll     = NULL;

        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_setflags(struct fionobj * const obj)
{
    bool retval = false;
    int32_t i, flags;

    if ((obj == NULL) || (obj->internal == NULL) || (obj->size <= 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 0; i < obj->size; i++)
        {
            obj->internal->pfds[i].fd = obj->fds[i];
            obj->internal->pfds[i].events = POLLPRI |
#if defined(LINUX)
                                            POLLRDHUP |
#endif
                                            POLLERR |
                                            POLLHUP |
                                            POLLNVAL;

            if (obj->pevents & FIONOBJ_PEVENT_IN)
            {
                obj->internal->pfds[i].events |= POLLIN;
            }

            if (obj->pevents & FIONOBJ_PEVENT_OUT)
            {
                obj->internal->pfds[i].events |= POLLOUT;
            }

            flags = fcntl(obj->fds[i], F_GETFL, 0);

            if (fcntl(obj->fds[i], F_SETFL, flags | O_NONBLOCK) != 0)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d non-blocking option failed (%d)\n",
                              __FUNCTION__,
                              obj->fds[i],
                              errno);
            }
        }

        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_poll(struct fionobj * const obj)
{
    bool retval = false;
    int32_t i, error;

    if ((obj == NULL) ||
        (obj->internal == NULL) ||
        (obj->internal->pfds == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->revents = 0;

        error = poll(obj->internal->pfds, obj->size, obj->timeoutms);

        if (error == 0)
        {
            // No event on any file descriptor.
            obj->revents = FIONOBJ_REVENT_TIMEOUT;
            retval = true;
        }
        else if (error > 0)
        {
            for (i = 0; i < obj->size; i++)
            {
                // Check for error events.
                if ((obj->internal->pfds[i].revents & POLLERR) ||
#if defined(LINUX)
                    (obj->internal->pfds[i].revents & POLLRDHUP) ||
#endif
                    (obj->internal->pfds[i].revents & POLLHUP) ||
                    (obj->internal->pfds[i].revents & POLLNVAL))
                {
                    obj->revents |= FIONOBJ_REVENT_ERROR;
                }

                // Check for input event.
                if (obj->internal->pfds[i].revents & POLLIN)
                {
                    obj->revents |= FIONOBJ_REVENT_INREADY;
                }

                // Check for output event.
                if (obj->internal->pfds[i].revents & POLLOUT)
                {
                    obj->revents |= FIONOBJ_REVENT_OUTREADY;
                }
#if defined(__CYGWIN__)
                // @bug Hack to detect a remote peer connection closure.
                if (obj->internal->pfds[i].revents & POLLIN)
                {
                    uint8_t buf[1];
                    if (recv(obj->fds[i],
                             buf,
                             sizeof(buf),
                             MSG_PEEK | MSG_DONTWAIT) != sizeof(buf))
                    {
                        obj->revents |= FIONOBJ_REVENT_ERROR;
                    }
                }
#endif
            }

            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: poll failed (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}
