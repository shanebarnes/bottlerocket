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
bool socket_open(const int32_t sockfd)
{
    bool retval = false;

    if (sockfd)
    {

    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_close(const int32_t sockfd)
{
    int32_t retval = false;

    if (close(sockfd) == 0)
    {
        retval = true;
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: socket %d could not be closed\n",
                      __FUNCTION__,
                      sockfd);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool socket_bind(const int32_t sockfd)
{
    bool retval = false;

    if (sockfd)
    {

    }

    return retval;
}
