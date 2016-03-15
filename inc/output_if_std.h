/**
 * @file   output_if_std.h
 * @author Shane Barnes
 * @date   14 Mar 2016
 * @brief  Standard output stream interface.
 */

#ifndef _OUTPUT_IF_STD_H_
#define _OUTPUT_IF_STD_H_

#include "system_types.h"

/**
 * @see output_if_api.h for interface comments.
 */
int32_t output_if_std_send(void * const buf, const uint32_t len);

#endif // _OUTPUT_IF_STD_H_
