/**
 * @file      util_debug.h
 * @brief     Debug utility interface.
 * @author    Shane Barnes
 * @date      21 Jul 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_DEBUG_H_
#define _UTIL_DEBUG_H_

/**
 * @brief Print the function return addresses of the current thread's call
 *        stack.
 *
 * @return Void.
 */
void utildebug_backtrace(void);

#endif // _UTIL_DEBUG_H_
