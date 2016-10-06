/**
 * @file      util_math.c
 * @brief     Math utility implementation.
 * @author    Shane Barnes
 * @date      17 Sep 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "util_math.h"

const uint32_t utilmath_tab64[64] =
{
    63,  0, 58,  1, 59, 47, 53,  2, 60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12, 44, 24, 15,  8, 23,  7,  6,  5
};

uint32_t utilmath_log2(const uint64_t val)
{
    uint32_t ret = 0;
    uint64_t tmp = val;

    tmp |= tmp >> 1;
    tmp |= tmp >> 2;
    tmp |= tmp >> 4;
    tmp |= tmp >> 8;
    tmp |= tmp >> 16;
    tmp |= tmp >> 32;

    ret = utilmath_tab64[((uint64_t)((tmp - (tmp >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];

    return ret;
}
