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

#if defined(__cplusplus)
    #define UTILMEM_CALLOC(type, size, count) ((type*)calloc(count, size))
#else
    #define UTILMEM_CALLOC(type, size, count) (calloc(count, size))
#endif

#define UTILMEM_FREE(ptr) (free(ptr))

#if defined(__cplusplus)
    #define UTILMEM_MALLOC(type, size, count) ((type*)malloc(size * count))
#else
    #define UTILMEM_MALLOC(type, size, count) (malloc(size * count))
#endif

#if defined(__cplusplus)
    #define UTILMEM_REALLOC(type, ptr, size) ((type*)realloc(ptr, size))
#else
    #define UTILMEM_REALLOC(type, ptr, size) (realloc(ptr, size))
#endif

#endif // _UTIL_MEM_H_
