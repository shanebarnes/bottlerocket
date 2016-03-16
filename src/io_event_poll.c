/**
 * @file   io_event_poll.c
 * @author Shane Barnes
 * @date   15 Mar 2016
 * @brief  Poll I/O event notification implementation.
 */

#include "io_event_poll.h"
#include "logger.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>

struct internals
{
    struct pollfd *pfds;
};

/**
 * @see See header file for interace comments.
 */
bool io_event_poll_create(struct io_event_instance * const instance)
{
    bool retval = false;
    int32_t i;

    if ((instance != NULL) &&
        (instance->internal == NULL) &&
        (instance->size > 0))
    {
        instance->internal = malloc(sizeof(struct internals));
        instance->internal->pfds = malloc(instance->size * sizeof(struct pollfd));

        if (instance->internal->pfds != NULL)
        {
            for (i = 0; i < instance->size; i++)
            {
                instance->internal->pfds[i].fd = instance->fds[i];
                instance->internal->pfds[i].events = POLLPRI |
#if defined(LINUX)
                                                     POLLRDHUP |
#endif
                                                     POLLERR |
                                                     POLLHUP |
                                                     POLLNVAL;

                if (instance->pevents & IO_EVENT_POLL_IN)
                {
                    instance->internal->pfds[i].events |= POLLIN;
                }

                if (instance->pevents & IO_EVENT_POLL_OUT)
                {
                    instance->internal->pfds[i].events |= POLLOUT;
                }

                if (fcntl(instance->fds[i], F_SETFL, O_NONBLOCK) != 0)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: setting non-blocking flag failed (%d)\n",
                                  __FUNCTION__,
                                  errno);
                }
            }

            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: memory allocation failed (%d)\n",
                          __FUNCTION__,
                          errno);
        }
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool io_event_poll_destroy(struct io_event_instance * const instance)
{
    bool retval = false;

    if ((instance != NULL) &&
        (instance->internal != NULL) &&
        (instance->size > 0))
    {
        if (instance->internal->pfds != NULL)
        {
            free(instance->internal->pfds);
            instance->internal->pfds = NULL;
        }

        free(instance->internal);
        instance->internal = NULL;
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool io_event_poll_poll(struct io_event_instance * const instance)
{
    bool retval = false;
    int32_t i, error;

    if ((instance != NULL) &&
        (instance->internal != NULL) &&
        (instance->internal->pfds != NULL))
    {
        instance->revents = 0;

        error = poll(instance->internal->pfds,
                     instance->size,
                     instance->timeoutms);

        if (error == 0)
        {
            // No event on any file descriptor.
            instance->revents = IO_EVENT_RET_TIMEOUT;
            retval = true;
        }
        else if (error > 0)
        {
            for (i = 0; i < instance->size; i++)
            {
                // Check for error events.
                if ((instance->internal->pfds[i].revents & POLLERR) ||
#if defined(LINUX)
                    (instance->internal->pfds[i].revents & POLLRDHUP) ||
#endif
                    (instance->internal->pfds[i].revents & POLLHUP) ||
                    (instance->internal->pfds[i].revents & POLLNVAL))
                {
                    instance->revents |= IO_EVENT_RET_ERROR;
                }

                // Check for out event.
                if (instance->internal->pfds[i].revents & POLLOUT)
                {
                    instance->revents |= IO_EVENT_RET_OUTREADY;
                }
#if defined(__CYGWIN__)
                // @bug Hack to detect a remote peer connection closure.
                if (instance->internal->pfds[i].revents & POLLIN)
                {
                    uint8_t buf[1];
                    if (recv(instance->fds[i],
                             buf,
                             sizeof(buf),
                             MSG_PEEK | MSG_DONTWAIT) != sizeof(buf))
                    {
                        instance->revents |= IO_EVENT_RET_ERROR;
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
