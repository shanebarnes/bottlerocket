/**
 * @file   socket_instance.h
 * @author Shane Barnes
 * @date   03 Mar 2016
 * @brief  Network socket instance interface.
 */

#ifndef _SOCKET_INSTANCE_H_
#define _SOCKET_INSTANCE_H_

#include "io_event_instance.h"
#include "system_types.h"

#include <netdb.h>
#include <netinet/in.h>

struct socket_instance;

struct socket_instance_ops
{
    /**
     * @brief Create a socket instance.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True if a socket instance was created.
     */
    bool (*sio_create)(struct socket_instance * const instance);

    /**
     * brief Destroy a socket instance.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True if a socket instance was destroyed.
     */
    bool (*sio_destroy)(struct socket_instance * const instance);

    /**
     * @brief Open a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True on success.
     */
    bool (*sio_open)(struct socket_instance * const instance);

    /**
     * @brief Close a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True on success.
     */
    bool (*sio_close)(struct socket_instance * const instance);

    /**
     * @brief Assign an address to a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True on success.
     */
    bool (*sio_bind)(struct socket_instance * const instance);

    /**
     * @brief Listen for connections on an open socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     * @param[in]     backlog  The maximum length of the pending connection
     *                         queue for the socket.
     *
     * @return True on success.
     */
    bool (*sio_listen)(struct socket_instance * const instance,
                       const int32_t backlog);

    /**
     * @brief Accept a connection on a listener socket.
     *
     * @param[in,out] listener  A pointer to a listener socket instance.
     * @param[in,out] instance  A pointer to a socket instance to initialize
     *                          with a new socket if a connection was accepted.
     *
     * @return True if a connection was accepted on a listener socket.
     */
    bool (*sio_accept)(struct socket_instance * const listener,
                       struct socket_instance * const instance);

    /**
     * @brief Initiate a connection on a socket.
     *
     * @param[in,out] instance  A pointer to a socket instance.
     *
     * @return True on success.
     */
    bool (*sio_connect)(struct socket_instance * const instance);

    /**
     * @brief Receive data from a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     * @param[in,out] buf      A pointer to a buffer to store data received from
     *                         the socket.
     * @param[in]     len      The maximum size of the receive buffer in bytes.
     *
     * @return The number of bytes received from the socket (-1 on error).
     */
    int32_t (*sio_recv)(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);

    /**
     * @brief Send data to a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     * @param[in]     buf      A pointer to a buffer containing data to be sent
     *                         to the socket.
     * @param[in]     len      The maximum number of bytes in the send buffer.
     *
     * @return The number of bytes sent to the socket (-1 on error).
     */
    int32_t (*sio_send)(struct socket_instance * const instance,
                        void * const buf,
                        const uint32_t len);
};

struct socket_addr_info
{
    struct sockaddr_in sockaddr;
    char               ipaddr[INET_ADDRSTRLEN];
    uint16_t           ipport;
    char               sockaddrstr[INET_ADDRSTRLEN + 6]; // form: <addr>:<port>
};

struct socket_instance
{
    struct socket_instance_ops  ops;
    struct io_event_instance    event;
    int32_t                     socktype, // e.g.: SOCK_DGRAM, SOCK_STREAM
                                sockfd;
    struct socket_addr_info     addrself,
                                addrpeer;
    struct addrinfo             ainfo,
                               *alist;
    char                       *ipaddr; // User configuration
    uint16_t                    ipport; // User configuration
};

/**
 * @brief Get the peer (remote) socket address.
 *
 * @param[in,out] instance A pointer to a socket instance.
 *
 * @return True if the peer socket address was obtained.
 */
bool socket_instance_getaddrpeer(struct socket_instance * const instance);

/**
 * @brief Get the self (local) socket address.
 *
 * @param[in,out] instance A pointer to a socket instance.
 *
 * @return True if the self socket address was obtained.
 */
bool socket_instance_getaddrself(struct socket_instance * const instance);

/**
 * @see sio_create() for interface comments.
 */
bool socket_instance_create(struct socket_instance * const instance);

/**
 * @see sio_destroy() for interface comments.
 */
bool socket_instance_destroy(struct socket_instance * const instance);

/**
 * @see sio_open() for interface comments.
 */
bool socket_instance_open(struct socket_instance * const instance);

/**
 * @see sio_close() for interface comments.
 */
bool socket_instance_close(struct socket_instance * const instance);

/**
 * @see sio_bind() for interface comments.
 */
bool socket_instance_bind(struct socket_instance * const instance);

#endif // _SOCKET_INSTANCE_H_
