/**
 * @file   socket.c
 * @author Shane Barnes
 * @date   03 Mar 2016
 * @brief  Network socket implementation.
 */

#include "logger.h"
#include "socket.h"

#include <unistd.h>

/**
 * @see See header file for interface comments.
 */
bool socket_open(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {

    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_close(struct socket_instance * const instance)
{
    int32_t retval = false;

    if (instance != NULL)
    {
        if (close(instance->sockfd) == 0)
        {
            retval = true;
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: socket %d could not be closed\n",
                          __FUNCTION__,
                          instance->sockfd);
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_bind(struct socket_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {

    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: instance does not exist\n",
                      __FUNCTION__);
    }

    return retval;
}
