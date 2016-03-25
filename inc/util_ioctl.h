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

/**
 * @brief Get the number of bytes that are available to be read.
 *
 * @param[in] fd A file descriptor.
 *
 * @return The number of bytes available to be read (-1 on error).
 */
int32_t utilioctl_getbytesavail(const int32_t fd);

#endif // _UTIL_IOCTL_H_
