/**
 * @file      sock_con.h
 * @brief     Socket connection manager interface.
 * @author    Shane Barnes
 * @date      23 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_CON_H_
#define _SOCK_CON_H_

#include "sock_obj.h"
#include "system_types.h"

#include <sys/socket.h>

struct sockcon_priv;

struct sockcon
{
    struct sockobj      *sock;
    struct sockcon_priv *priv;
};

/**
 * @brief Create a socket connection manager.
 *
 * @param[in,out] obj A pointer to a socket connection manager to create.
 *
 * @return True if a socket connection manager was created.
 */
bool sockcon_create(struct sockcon * const con);

/**
 * @brief Destroy a socket connection manager.
 *
 * @param[in,out] obj A pointer to a socket connection manager to destroy.
 *
 * @return True if a socket connection manager was destroyed.
 */
bool sockcon_destroy(struct sockcon * const con);

/**
 * @brief Listen for connections on a socket object.
 *
 * @param[in,out] con     A pointer to a socket connection manager.
 * @param[in,out] sock    A pointer to a socket object.
 * @param[in]     backlog The maximum length of the pending connection queue.
 *                        Defaults to SOMAXCONN if length specified is less than
 *                        or equal to zero.
 *
 * @return True if socket connection manager started listening for connections.
 */
bool sockcon_listen(struct sockcon * const con,
                    struct sockobj * const sock,
                    const int32_t backlog);

/**
 * @brief Accept a connection from a socket connection manager.
 *
 * @param[in,out] con  A pointer to a socket connection manager.
 * @param[in,out] addr A pointer to an address structure to fill with the
 *                     address of the remote peer.
 * @param[in,out] len  The length of the address structure to fill. It will be
 *                     modified upon return with the actual length of the
 *                     address.
 *
 * @return A socket file descriptor (non-negative integer) on success.
 */
int32_t sockcon_accept(struct sockcon * const con,
                       struct sockaddr * const addr,
                       socklen_t * const len);

#endif // _SOCK_CON_H_
