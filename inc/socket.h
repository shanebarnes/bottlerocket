/**
 * @file   socket.h
 * @author Shane Barnes
 * @date   03 Mar 2016
 * @brief  Network socket interface.
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "system_types.h"

struct socket_instance
{
    int32_t sockfd,
            listenfd;
};

/**
 * @see socket_api.h for interface comments.
 */
bool socket_open(struct socket_instance * const instance);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_close(struct socket_instance * const instance);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_bind(struct socket_instance * const instance);

#endif // _SOCKET_H_
