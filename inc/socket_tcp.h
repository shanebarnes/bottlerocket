/**
 * @file   socket_tcp.h
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  TCP socket interface.
 */

#ifndef _SOCKET_TCP_H_
#define _SOCKET_TCP_H_

#include "socket_api.h"
#include "system_types.h"

/**
 * @brief Initialize a TCP socket API structure.
 *
 * @param[in,out] api A pointer to the TCP socket APi structure.
 *
 * @return True on success.
 */
bool socket_tcp_init(struct socket_api * const api);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_tcp_listen(const int32_t sockfd, const int32_t backlog);

/**
 * @see socket_api.h for interface comments.
 */
int32_t socket_tcp_accept(const int32_t sockfd, const int32_t timeoutms);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_tcp_connect(const int32_t sockfd, const int32_t timeoutms);

/**
 * @see socket_api.h for interface comments.
 */
int32_t socket_tcp_recv(const int32_t sockfd,
                        void * const buf,
                        const uint32_t len);

/**
 * @see socket_api.h for interface comments.
 */
int32_t socket_tcp_send(const int32_t sockfd,
                        void * const buf,
                        const uint32_t len);

#endif // _SOCKET_TCP_H_
