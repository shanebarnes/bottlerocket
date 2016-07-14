/**
 * @file      util_mem.h
 * @brief     Memory allocation utility interface.
 * @author    Shane Barnes
 * @date      13 Jul 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _UTIL_MEM_H_
#define _UTIL_MEM_H_

#include <stdlib.h>

#define UTILMEM_FREE(ptr) (free(ptr))

#if defined(__cplusplus)
    #define UTILMEM_MALLOC(type, count) ((type*)malloc(sizeof(type) * count))
#else
    #define UTILMEM_MALLOC(type, count) (malloc(sizeof(type) * count))
#endif

#endif // _UTIL_MEM_H_
