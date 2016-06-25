/**
 * @file      util_sysctl.h
 * @brief     System control utility interface.
 * @author    Shane Barnes
 * @date      11 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_SYSCTL_H_
#define _UTIL_SYSCTL_H_

#include "system_types.h"

/**
 * @brief Get the number of available CPUs.
 *
 * @return The number of available CPUs (0 on error).
 */
uint32_t utilsysctl_getcpusavail(void);

/**
 * @brief Get the maximum UDP datagram size.
 *
 * @return The maximum UDP datagram size in bytes (-1 on error).
 */
int32_t utilsysctl_getmaxudpsize(void);

/**
 * @brief Get the maximum socket buffer size.
 *
 * @return The maximum socket buffer size in bytes (-1 on error).
 */
int32_t utilsysctl_getmaxsockbufsize(void);

#endif // _UTIL_SYSCTL_H_
