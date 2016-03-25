/**
 * @file      sock_udp.h
 * @brief     UDP socket interface.
 * @author    Shane Barnes
 * @date      21 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_UDP_H_
#define _SOCK_UDP_H_

#include "sock_obj.h"
#include "system_types.h"

/**
 * @see soo_create() for interface comments.
 */
bool sockudp_create(struct sockobj * const obj);

/**
 * @see soo_destroy() for interface comments.
 */
bool sockudp_destroy(struct sockobj * const obj);

/**
 * @see soo_listen() for interface comments.
 */
bool sockudp_listen(struct sockobj * const obj, const int32_t backlog);

/**
 * @see soo_accept() for interface comments.
 */
bool sockudp_accept(struct sockobj * const listener,
                    struct sockobj * const obj);

/**
 * @see soo_connect() for interface comments.
 */
bool sockudp_connect(struct sockobj * const obj);

/**
 * @see soo_recv() for interface comments.
 */
int32_t sockudp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len);

/**
 * @see soo_send() for interface comments.
 */
int32_t sockudp_send(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len);

#endif // _SOCK_UDP_H_
