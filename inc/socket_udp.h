/**
 * @file      socket_udp.h
 * @brief     UDP socket interface.
 * @author    Shane Barnes
 * @date      21 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCKET_UDP_H_
#define _SOCKET_UDP_H_

#include "socket_instance.h"
#include "system_types.h"

/**
 * @see sio_create() for interface comments.
 */
bool socket_udp_create(struct socket_instance * const instance);

/**
 * @see sio_destroy() for interface comments.
 */
bool socket_udp_destroy(struct socket_instance * const instance);

/**
 * @see sio_listen() for interface comments.
 */
bool socket_udp_listen(struct socket_instance * const instance,
                       const int32_t backlog);

/**
 * @see sio_accept() for interface comments.
 */
bool socket_udp_accept(struct socket_instance * const listener,
                       struct socket_instance * const instance);

/**
 * @see sio_connect() for interface comments.
 */
bool socket_udp_connect(struct socket_instance * const instance);

/**
 * @see sio_recv() for interface comments.
 */
int32_t socket_udp_recv(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);

/**
 * @see sio_send() for interface comments.
 */
int32_t socket_udp_send(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);

#endif // _SOCKET_UDP_H_
