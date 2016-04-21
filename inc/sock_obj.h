/**
 * @file      sock_obj.h
 * @brief     Socket object interface.
 * @author    Shane Barnes
 * @date      03 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_OBJ_H_
#define _SOCK_OBJ_H_

#include "fion_obj.h"
#include "system_types.h"

#include <netdb.h>
#include <netinet/in.h>

struct sockobj;

struct sockobj_ops
{
    /**
     * @brief Create a socket object.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True if a socket object was created.
     */
    bool (*sock_create)(struct sockobj * const obj);

    /**
     * brief Destroy a socket object.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True if a socket object was destroyed.
     */
    bool (*sock_destroy)(struct sockobj * const obj);

    /**
     * @brief Open a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_open)(struct sockobj * const obj);

    /**
     * @brief Close a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_close)(struct sockobj * const obj);

    /**
     * @brief Assign an address to a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_bind)(struct sockobj * const obj);

    /**
     * @brief Listen for connections on an open socket.
     *
     * @param[in,out] obj     A pointer to a socket object.
     * @param[in]     backlog The maximum length of the pending connection
     *                        queue for the socket.
     *
     * @return True on success.
     */
    bool (*sock_listen)(struct sockobj * const obj,
                        const int32_t backlog);

    /**
     * @brief Accept a connection on a listener socket.
     *
     * @param[in,out] listener  A pointer to a listener socket object.
     * @param[in,out] obj       A pointer to a socket object to initialize
     *                          with a new socket if a connection was accepted.
     *
     * @return True if a connection was accepted on a listener socket.
     */
    bool (*sock_accept)(struct sockobj * const listener,
                        struct sockobj * const obj);

    /**
     * @brief Initiate a connection on a socket.
     *
     * @param[in,out] obj  A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_connect)(struct sockobj * const obj);

    /**
     * @brief Receive data from a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     * @param[in,out] buf A pointer to a buffer to store data received from the
     *                    socket.
     * @param[in]     len The maximum size of the receive buffer in bytes.
     *
     * @return The number of bytes received from the socket (-1 on error).
     */
    int32_t (*sock_recv)(struct sockobj * const obj,
                         void * const buf,
                         const uint32_t len);

    /**
     * @brief Send data to a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     * @param[in]     buf A pointer to a buffer containing data to be sent to
     *                    the socket.
     * @param[in]     len The maximum number of bytes in the send buffer.
     *
     * @return The number of bytes sent to the socket (-1 on error).
     */
    int32_t (*sock_send)(struct sockobj * const obj,
                         void * const buf,
                         const uint32_t len);
};

enum sockobj_state
{
    SOCKOBJ_STATE_NULL    = 0x00,
    SOCKOBJ_STATE_CLOSE   = 0x01,
    SOCKOBJ_STATE_OPEN    = 0x02,
    SOCKOBJ_STATE_BIND    = 0x04,
    SOCKOBJ_STATE_LISTEN  = 0x08,
    SOCKOBJ_STATE_CONNECT = 0x10,
};

struct sockobj_addr
{
    struct sockaddr_in sockaddr;
    char               ipaddr[INET6_ADDRSTRLEN];
    uint16_t           ipport;
    char               sockaddrstr[INET6_ADDRSTRLEN + 6]; // form: <addr>:<port>
};

struct sockobj_info
{
    uint64_t startusec,
             stopusec,
             recvbytes,
             sendbytes;
};

struct sockobj
{
    struct sockobj_info  info;
    struct sockobj_ops   ops;
    struct fionobj       event;
    int32_t              socktype, // e.g.: SOCK_DGRAM, SOCK_STREAM
                         sockfd;
    struct sockobj_addr  addrself,
                         addrpeer;
    struct addrinfo      ainfo,
                        *alist;
    char                 ipaddr[INET6_ADDRSTRLEN];
    uint16_t             ipport;
    enum sockobj_state   state;
};

/**
 * @brief Get the peer (remote) socket address.
 *
 * @param[in,out] obj A pointer to a socket object.
 *
 * @return True if the peer socket address was obtained.
 */
bool sockobj_getaddrpeer(struct sockobj * const obj);

/**
 * @brief Get the self (local) socket address.
 *
 * @param[in,out] obj A pointer to a socket object.
 *
 * @return True if the self socket address was obtained.
 */
bool sockobj_getaddrself(struct sockobj * const obj);

/**
 * @see sock_create() for interface comments.
 */
bool sockobj_create(struct sockobj * const obj);

/**
 * @see sock_destroy() for interface comments.
 */
bool sockobj_destroy(struct sockobj * const obj);

/**
 * @see sock_open() for interface comments.
 */
bool sockobj_open(struct sockobj * const obj);

/**
 * @see sock_close() for interface comments.
 */
bool sockobj_close(struct sockobj * const obj);

/**
 * @see sock_bind() for interface comments.
 */
bool sockobj_bind(struct sockobj * const obj);

#endif // _SOCK_OBJ_H_
