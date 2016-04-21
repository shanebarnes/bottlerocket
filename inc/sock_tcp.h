/**
 * @file      sock_tcp.h
 * @brief     TCP socket interface.
 * @author    Shane Barnes
 * @date      01 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_TCP_H_
#define _SOCK_TCP_H_

#include "sock_obj.h"
#include "system_types.h"

/**
 * @see sock_create() for interface comments.
 */
bool socktcp_create(struct sockobj * const obj);

/**
 * @see sock_destroy() for interface comments.
 */
bool socktcp_destroy(struct sockobj * const obj);

/**
 * @see sock_listen() for interface comments.
 */
bool socktcp_listen(struct sockobj * const obj, const int32_t backlog);

/**
 * @see sock_accept() for interface comments.
 */
bool socktcp_accept(struct sockobj * const listener,
                    struct sockobj * const obj);

/**
 * @see sock_connect() for interface comments.
 */
bool socktcp_connect(struct sockobj * const obj);

/**
 * @see sock_recv() for interface comments.
 */
int32_t socktcp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len);

/**
 * @see sock_send() for interface comments.
 */
int32_t socktcp_send(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len);

#endif // _SOCK_TCP_H_
