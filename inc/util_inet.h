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
 * @param[in]     hostname A human-readable nickname for an IP address (e.g.,
 *                         www.google.com).
 * @param[in,out] addr     A pointer to a string buffer to copy the IP address.
 * @param[in]     len      The maximum size of the string buffer in bytes.
 *
 * @return True if an IP address was found for the given hostname.
 */
bool utilinet_getaddrfromhost(const char * const hostname,
                              char * const addr,
                              const uint32_t len);

#endif // _UTIL_INET_H_
