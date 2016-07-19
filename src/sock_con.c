/**
 * @file      sock_con.c
 * @brief     Socket container implementation.
 * @author    Shane Barnes
 * @date      23 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "fion_poll.h"
#include "logger.h"
#include "sock_con.h"

/**
 * @see See header file for interface comments.
 */
bool sockcon_create(struct sockcon * const con)
{
    bool retval = false;

    if ((con == NULL) ||
        (vector_getsize(&con->fion.fds) != 0) ||
        (vector_getsize(&con->sock) != 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((fionpoll_create(&con->fion) == true) &&
            (vector_create(&con->sock, 0) == true))
        {
            retval = true;
        }

        //con->event.fds       = -1;
        //con->event.size      = 0;
        //con->event.timeoutms = 0;
        //con->event.pevents   = IO_EVENT_POLL_IN;
        //con->event.internal  = NULL;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockcon_destroy(struct sockcon * const con)
{
    bool retval = false;

    if (con == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((vector_destroy(&con->sock) == true) &&
            (fionpoll_destroy(&con->fion) == true))
        {
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
struct sockobj *sockcon_get(struct sockcon * const con, const uint32_t index)
{
    struct sockobj *retval = NULL;

    if (con == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = vector_getval(&con->sock, index);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockcon_insert(struct sockcon * const con, struct sockobj * const sock)
{
    bool retval = false;

    if ((con == NULL) || (sock == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (con->fion.ops.fion_insertfd(&con->fion, sock->fd) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to insert socket %d\n",
                          __FUNCTION__,
                          sock->fd);
        }
        else if (vector_inserttail(&con->sock, sock) == false)
        {
            con->fion.ops.fion_deletefd(&con->fion, sock->fd);
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to insert socket %d\n",
                          __FUNCTION__,
                          sock->fd);
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
bool sockcon_delete(struct sockcon * const con, const uint32_t index)
{
    bool retval = false;
    struct sockobj *sock = NULL;

    if (con == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        sock = sockcon_get(con, index);

        if (sock == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to find socket at index %u\n",
                          __FUNCTION__,
                          index);
        }
        else
        {
            if (con->fion.ops.fion_deletefd(&con->fion, sock->fd) == false)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to destroy socket %d fd\n",
                              __FUNCTION__,
                              sock->fd);
            }
            else if (vector_delete(&con->sock, index) == false)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to destroy socket at index %u\n",
                              __FUNCTION__,
                              index);
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
bool sockcon_poll(struct sockcon * const con)
{
    bool retval = false;

    if (con == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = con->fion.ops.fion_poll(&con->fion);
    }

    return retval;
}
