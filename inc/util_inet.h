/**
 * @file      util_inet.h
 * @brief     IP network utility interface.
 * @author    Shane Barnes
 * @date      01 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_INET_H_
#define _UTIL_INET_H_

#include "system_types.h"

#include <sys/socket.h>

/**
 * @brief Check if an IP address is a valid IPv4 address. The check determines
 *        if the address is expressed in valid dot-decimal notation (e.g.,
 *        127.0.0.1).
 *
 * @param[in] addr A string-representation of an IP address.
 *
 * @return True if an IP address is an IPv4 address.
 */
bool utilinet_isipv4(const char * const addr);

/**
 * @brief Check if an IP address is a valid IPv6 address. The check determines
 *        if the address is expressed in valid IPv6 notation (e.g.,
 *        0:0:0:0:0:0:0:1).
 *
 * @param[in] addr A string-representation of an IP address.
 *
 * @return True if an IP address is an IPv6 address.
 */
bool utilinet_isipv6(const char * const addr);

/**
 * @brief Get an IP address from a hostname (e.g., reverse DNS lookup).
 *
 * @param[in]     host   A human-readable nickname for an IP address (e.g.,
 *                       www.google.com).
 * @param[in]     family The address family of the IP address (e.g., AF_INET).
 * @param[in,out] addr   A pointer to a string buffer to copy the IP address.
 * @param[in]     len    The maximum size of the string buffer in bytes.
 *
 * @return True if an IP address was found for the given hostname.
 */
bool utilinet_getaddrfromhost(const char * const host,
                              const int32_t family,
                              char * const addr,
                              const uint32_t len);

/**
 * @brief Get a pointer to a socket address given an address family (e.g.,
 *        AF_INET).
 *
 * @param[in] addr A pointer to a socket address storage structure.
 *
 * @return A pointer to a socket address (e.g., sockaddr_in*) or NULL on error.
 */
void *utilinet_getaddrfromstorage(const struct sockaddr_storage * const addr);

/**
 * @brief Get a pointer to a socket address port number field given an address
 *        family (e.g., AF_INET).
 *
 * @param[in] addr A pointer to a socket address storage structure.
 *
 * @return A pointer to a socket address port number field (e.g., &sin_port) or
 *         NULL on error.
 */
uint16_t *utilinet_getportfromstorage(const struct sockaddr_storage * const addr);

#endif // _UTIL_INET_H_
