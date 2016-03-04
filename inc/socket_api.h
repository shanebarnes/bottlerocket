/**
 * @file   socket_api.h
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  Socket API interface.
 */

#ifndef _SOCKET_API_H_
#define _SOCKET_API_H_

#include "system_types.h"

struct socket_api
{
    /**
     * @brief Open a socket.
     *
     * @param[in] sockfd An unused socket file descriptor.
     *
     * @return True on success.
     */
    bool (*socket_api_open)(const int32_t sockfd);

    /**
     * @brief Close a socket.
     *
     * @param[in] sockfd An open socket file descriptor.
     *
     * @return True on success.
     */
    bool (*socket_api_close)(const int32_t sockfd);

    /**
     * @brief Assign an address to a socket.
     *
     * @param[in] sockfd An open socket file descriptor.
     *
     * @return True on success.
     */
    bool (*socket_api_bind)(const int32_t sockfd);

    /**
     * @brief Listen for connections on an open socket.
     *
     * @param[in] sockfd  An open socket file descriptor.
     * @param[in] backlog The maximum length of the pending connection queue
     *                    for the socket.
     *
     * @return True on success.
     */
    bool (*socket_api_listen)(const int32_t sockfd, const int32_t backlog);

    /**
     * @brief Accept a connection on a listener socket.
     *
     * @param[in] sockfd    An open listener socket file descriptor.
     * @param[in] timeoutms The accept timeout in milliseconds.
     *
     * @return A non-negative integer that is the accepted socket file
     *         descriptor (-1 on error).
     */
    int32_t (*socket_api_accept)(const int32_t sockfd, const int32_t timeoutms);

    /**
     * @brief Initiate a connection on a socket.
     *
     * @param[in] sockfd    An open socket file descriptor.
     * @param[in] timeoutms The connect timeout in milliseconds.
     *
     * @return True on success.
     */
    bool (*socket_api_connect)(const int32_t sockfd, const int32_t timeoutms);

    /**
     * @brief Receive data from a socket.
     *
     * @param[in]     sockfd An open socket file descriptor.
     * @param[in,out] buf    A pointer to a buffer to store data received from
     *                       the socket.
     * @param[in]     len    The maximum size of the receive buffer in bytes.
     *
     * @return The number of bytes received from the socket (-1 on error).
     */
    int32_t (*socket_api_recv)(const int32_t sockfd,
                               void * const buf,
                               const uint32_t len);

    /**
     * @brief Send data to a socket.
     *
     * @param[in] sockfd An open socket file descriptor.
     * @param[in] buf    A pointer to a buffer containing data to be sent to
     *                   the socket.
     * @param[in] len    The maximum number of bytes in the send buffer.
     *
     * @return The number of bytes sent to the socket (-1 on error).
     */
    int32_t (*socket_api_send)(const int32_t sockfd,
                               void * const buf,
                               const uint32_t len);
};

#endif // _SOCKET_API_H_
