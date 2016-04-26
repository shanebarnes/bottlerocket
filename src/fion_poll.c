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

/**
 * @see See header file for interface comments.
 */
bool fionpoll_create(struct fionobj * const obj)
{
    bool retval = false;

    if ((obj == NULL) || (obj->fds.vsize != 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->ops.fion_create   = fionpoll_create;
        obj->ops.fion_destroy  = fionpoll_destroy;
        obj->ops.fion_insertfd = fionpoll_insertfd;
        obj->ops.fion_deletefd = fionpoll_deletefd;
        obj->ops.fion_setflags = fionpoll_setflags;
        obj->ops.fion_poll     = fionpoll_poll;

        obj->timeoutms = 0;
        obj->pevents   = 0;

        if (vector_create(&obj->fds, 0) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: vector allocation failed (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            retval = true;
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

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        while (obj->fds.vsize > 0)
        {
            if (vector_get(&obj->fds, 0) != NULL)
            {
                free(vector_get(&obj->fds, 0));
                vector_delete(&obj->fds, 0);
            }
        }

        obj->ops.fion_create   = NULL;
        obj->ops.fion_destroy  = NULL;
        obj->ops.fion_insertfd = NULL;
        obj->ops.fion_deletefd = NULL;
        obj->ops.fion_setflags = NULL;
        obj->ops.fion_poll     = NULL;

        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_insertfd(struct fionobj * const obj, const int32_t fd)
{
    bool retval = false, found = false;
    struct pollfd *pfd = NULL;
    uint32_t i;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 0; i < obj->fds.vsize; i++)
        {
            pfd = (struct pollfd *)obj->fds.array[i];

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
            pfd = malloc(sizeof(struct pollfd));
            pfd->fd = fd;
            retval = vector_inserttail(&obj->fds, pfd);
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool fionpoll_deletefd(struct fionobj * const obj, const int32_t fd)
{
    bool retval = false, found = false;
    struct pollfd *pfd = NULL;
    uint32_t i;

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 0; i < obj->fds.vsize; i++)
        {
            pfd = (struct pollfd *)obj->fds.array[i];

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
            free(pfd);
            retval = vector_delete(&obj->fds, i);
        }
    }

    return retval;
}


/**
 * @see See header file for interface comments.
 */
bool fionpoll_setflags(struct fionobj * const obj)
{
    bool retval = false;
    struct pollfd *pfd = NULL;
    uint32_t i;
    int32_t flags;

    if ((obj == NULL) || (obj->fds.vsize == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 0; i < obj->fds.vsize; i++)
        {
            pfd = (struct pollfd *)obj->fds.array[i];
            pfd->events = POLLPRI |
#if defined(LINUX)
                          POLLRDHUP |
#endif
                          POLLERR |
                          POLLHUP |
                          POLLNVAL;

            if (obj->pevents & FIONOBJ_PEVENT_IN)
            {
                pfd->events |= POLLIN;
            }

            if (obj->pevents & FIONOBJ_PEVENT_OUT)
            {
                pfd->events |= POLLOUT;
            }

            flags = fcntl(pfd->fd, F_GETFL, 0);

            if (fcntl(pfd->fd, F_SETFL, flags | O_NONBLOCK) != 0)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: socket %d non-blocking option failed (%d)\n",
                              __FUNCTION__,
                              pfd->fd,
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
    struct pollfd *pfd = NULL;
    uint32_t i;
    int32_t error;

    if ((obj == NULL) || (obj->fds.vsize == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->revents = 0;

        error = poll((struct pollfd *)obj->fds.array[0], obj->fds.vsize, obj->timeoutms);

        if (error == 0)
        {
            // No event on any file descriptor.
            obj->revents = FIONOBJ_REVENT_TIMEOUT;
            retval = true;
        }
        else if (error > 0)
        {
            for (i = 0; i < obj->fds.vsize; i++)
            {
                pfd = (struct pollfd *)obj->fds.array[i];

                // Check for error events.
                if ((pfd->revents & POLLERR) ||
#if defined(LINUX)
                    (pfd->revents & POLLRDHUP) ||
#endif
                    (pfd->revents & POLLHUP) ||
                    (pfd->revents & POLLNVAL))
                {
                    obj->revents |= FIONOBJ_REVENT_ERROR;
                }

                // Check for input event.
                if (pfd->revents & POLLIN)
                {
                    obj->revents |= FIONOBJ_REVENT_INREADY;
                }

                // Check for output event.
                if (pfd->revents & POLLOUT)
                {
                    obj->revents |= FIONOBJ_REVENT_OUTREADY;
                }
//#if defined(__CYGWIN__)
//                // @bug Hack to detect a remote peer connection closure.
//                if (pfd->revents & POLLIN)
//                {
//                    uint8_t buf[1];
//                    if (recv(pfd->fd,
//                             buf,
//                             sizeof(buf),
//                             MSG_PEEK | MSG_DONTWAIT) != sizeof(buf))
//                    {
//                        obj->revents |= FIONOBJ_REVENT_ERROR;
//                    }
//                }
//#endif
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
