/**
 * @file   socket_tcp.h
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  TCP socket interface.
 */

#ifndef _SOCKET_TCP_H_
#define _SOCKET_TCP_H_

#include "socket_instance.h"
#include "system_types.h"

/**
 * @brief Initialize a TCP socket API structure.
 *
 * @param[in,out] api A pointer to a socket instance.
 *
 * @return True on success.
 */
bool socket_tcp_init(struct socket_instance * const instance);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_tcp_listen(struct socket_instance * const instance,
                       const int32_t backlog);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_tcp_accept(struct socket_instance * const listener,
                       struct socket_instance * const instance,
                       const int32_t timeoutms);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_tcp_connect(struct socket_instance * const instance,
                        const int32_t timeoutms);

/**
 * @see socket_api.h for interface comments.
 */
int32_t socket_tcp_recv(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);

/**
 * @see socket_api.h for interface comments.
 */
int32_t socket_tcp_send(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);

#endif // _SOCKET_TCP_H_
