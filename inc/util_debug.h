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

#include "system_types.h"

/**
 * @brief Verify an expression.
 *
 * @param[in] expr An expression to verify.
 *
 * @return True if an expression was verified.
 */
#define UTILDEBUG_VERIFY(expr) utildebug_verify((!(expr) ? 0 : 1), #expr, __FUNCTION__, __LINE__)

/**
 * @brief Print the function return addresses of the current thread's call
 *        stack.
 *
 * @return Void.
 */
void utildebug_backtrace(void);

/**
 * @brief Verify a conditional expression. If expression verification failed,
 *        then print an error message without causing program termination. This
 *        function was intended to be called by the UTILDEBUG_VERIFY macro.
 *
 * @param[in] eval True if an expression was verified.
 * @param[in] expr A character string representation of a expression.
 * @param[in] func A character string representation of expression function.
 * @param[in] line Line number of expression function.
 *
 * @return True if an expression was verified.
 */
bool utildebug_verify(const bool eval,
                      const char * const expr,
                      const char * const func,
                      const uint32_t line);

#endif // _UTIL_DEBUG_H_
