/**
 * @file      util_math.h
 * @brief     Math utility interface.
 * @author    Shane Barnes
 * @date      17 Sep 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_MATH_H_
#define _UTIL_MATH_H_

#include "system_types.h"

/**
 * @brief Compute the result of the logarithm of a value to base 2.
 *
 * @param[in] val A 64-bit value.
 *
 * @return The logarithm of a value to base 2.
 */
uint32_t utilmath_log2(const uint64_t val);

#endif // _UTIL_MATH_H_
