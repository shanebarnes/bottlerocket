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
 * @see sio_create() for interface comments.
 */
bool socket_tcp_create(struct socket_instance * const instance);

/**
 * @see sio_destroy() for interface comments.
 */
bool socket_tcp_destroy(struct socket_instance * const instance);

/**
 * @see sio_listen() for interface comments.
 */
bool socket_tcp_listen(struct socket_instance * const instance,
                       const int32_t backlog);

/**
 * @see sio_accept() for interface comments.
 */
bool socket_tcp_accept(struct socket_instance * const listener,
                       struct socket_instance * const instance);

/**
 * @see sio_connect() for interface comments.
 */
bool socket_tcp_connect(struct socket_instance * const instance);

/**
 * @see sio_recv() for interface comments.
 */
int32_t socket_tcp_recv(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);

/**
 * @see sio_send() for interface comments.
 */
int32_t socket_tcp_send(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);

#endif // _SOCKET_TCP_H_
