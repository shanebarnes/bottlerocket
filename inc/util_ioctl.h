/**
 * @file      util_ioctl.h
 * @brief     I/O control utility interface.
 * @author    Shane Barnes
 * @date      24 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_IOCTL_H_
#define _UTIL_IOCTL_H_

#include "system_types.h"

#include <netinet/in.h>

/**
 * @brief Get the number of bytes that are available to be read.
 *
 * @param[in] fd A file descriptor.
 *
 * @return The number of bytes available to be read (-1 on error).
 */
int32_t utilioctl_getbytesavail(const int32_t fd);

/**
 * @brief Get the network interface MTU in bytes for a network interface
 *        address.
 *
 * @param [in] addr The socket address.
 *
 * @return The network interface MTU in bytes (-1 on error).
 */
int32_t utilioctl_getifmtubyaddr(const struct sockaddr_in addr);

/**
 * @brief Get the network interface MTU in bytes for a network interface name.
 *
 * @param[in] name The network interface name.
 *
 * @return The network interface MTU in bytes (-1 on error).
 */
int32_t utilioctl_getifmtubyname(char * const name);

/**
 * @brief Get the maximum network interface MTU in bytes.
 *
 * @return The maximum network interface MTU in bytes (-1 on error).
 */
int32_t utilioctl_getifmaxmtu(void);

/**
 * @brief Get the minimum network interface MTU in bytes.
 *
 * @return The minimum network interface MTU in bytes (-1 on error).
 */
int32_t utilioctl_getifminmtu(void);

/**
 * @brief Get the terminal window size in terms of column size and row size.
 *
 * @param[in,out] rows A pointer to a row size variable to set (0 on error).
 * @param[in,out] cols A pointer to a column size variable to set (0 on error).
 *
 * @return True if the terminal window size dimensions were retrieved.
 */
bool utilioctl_gettermsize(uint16_t * const rows, uint16_t * const cols);

#endif // _UTIL_IOCTL_H_
