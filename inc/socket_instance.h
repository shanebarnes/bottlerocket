/**
 * @file   socket_instance.h
 * @author Shane Barnes
 * @date   03 Mar 2016
 * @brief  Network socket instance interface.
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "socket_api.h"
#include "system_types.h"

#include <netdb.h>
#include <netinet/in.h>

struct socket_addr_info
{
    struct sockaddr_in sockaddr;
    char               ipaddr[INET_ADDRSTRLEN];
    uint16_t           ipport;
    char               sockaddrstr[INET_ADDRSTRLEN + 6]; // form: <addr>:<port>
};

struct socket_instance
{
    struct socket_api        sockapi;
    int32_t                  socktype, // e.g.: SOCK_DGRAM, SOCK_STREAM
                             sockfd;
    struct socket_addr_info  addrself,
                             addrpeer;
    struct addrinfo          ainfo,
                            *alist;
    char                    *ipaddr; // User configuration
    uint16_t                 ipport; // User configuration
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
 * @see socket_api.h for interface comments.
 */
bool socket_instance_open(struct socket_instance * const instance);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_instance_close(struct socket_instance * const instance);

/**
 * @see socket_api.h for interface comments.
 */
bool socket_instance_bind(struct socket_instance * const instance);

#endif // _SOCKET_H_
