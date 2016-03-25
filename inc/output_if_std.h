/**
 * @file      output_if_std.h
 * @brief     Standard output stream interface.
 * @author    Shane Barnes
 * @date      14 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _OUTPUT_IF_STD_H_
#define _OUTPUT_IF_STD_H_

#include "system_types.h"

/**
 * @see oio_send() for interface comments.
 */
int32_t output_if_std_send(void * const buf, const uint32_t len);

#endif // _OUTPUT_IF_STD_H_
