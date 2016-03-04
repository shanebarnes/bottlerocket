/**
 * @file   socket.h
 * @author Shane Barnes
 * @date   03 Mar 2016
 * @brief  Network socket interface.
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "system_types.h"

/**
 * @see socket_api.h for interface comments.
 */
bool socket_open(const int32_t sockfd);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_close(const int32_t sockfd);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_bind(const int32_t sockfd);

#endif // _SOCKET_H_
