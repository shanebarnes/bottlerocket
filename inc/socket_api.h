/**
 * @file   socket_api.h
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  Socket API interface.
 */

#ifndef _SOCKET_API_H_
#define _SOCKET_API_H_

#include "socket_instance.h"
#include "system_types.h"

struct socket_instance; // Forward declaration

struct socket_api
{
    /**
     * @brief Open a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True on success.
     */
    bool (*open)(struct socket_instance * const instance);

    /**
     * @brief Close a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True on success.
     */
    bool (*close)(struct socket_instance * const instance);

    /**
     * @brief Assign an address to a socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     *
     * @return True on success.
     */
    bool (*bind)(struct socket_instance * const instance);

    /**
     * @brief Listen for connections on an open socket.
     *
     * @param[in,out] instance A pointer to a socket instance.
     * @param[in]     backlog  The maximum length of the pending connection
     *                         queue for the socket.
     *
     * @return True on success.
     */
    bool (*listen)(struct socket_instance * const instance,
                   const int32_t backlog);

    /**
     * @brief Accept a connection on a listener socket.
     *
     * @param[in,out] listener  A pointer to a listener socket instance.
     * @param[in,out] instance  A pointer to a socket instance to initialize
     *                          with a new socket if a connection was accepted.
     * @param[in]     timeoutms The accept timeout in milliseconds.
     *
     * @return True if a connection was accepted on a listener socket.
     */
    bool (*accept)(struct socket_instance * const listener,
                   struct socket_instance * const instance,
                   const int32_t timeoutms);

    /**
     * @brief Initiate a connection on a socket.
     *
     * @param[in,out] instance  A pointer to a socket instance.
     * @param[in]     timeoutms The connect timeout in milliseconds.
     *
     * @return True on success.
     */
    bool (*connect)(struct socket_instance * const instance,
                    const int32_t timeoutms);

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
    int32_t (*recv)(struct socket_instance * const instance,
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
    int32_t (*send)(struct socket_instance * const instance,
                    void * const buf,
                    const uint32_t len);
};

#endif // _SOCKET_API_H_
