/**
 * @file   util_sysctl.h
 * @author Shane Barnes
 * @date   11 Mar 2016
 * @brief  System control utility interface.
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

#endif // _UTIL_SYSCTL_H_
