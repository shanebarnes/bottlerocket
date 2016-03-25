/**
 * @file      input_if_std.h
 * @brief     Standard input stream interface.
 * @author    Shane Barnes
 * @date      24 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _INPUT_IF_STD_H_
#define _INPUT_IF_STD_H_

#include "system_types.h"

/**
 * @see iio_recv() for interface comments.
 */
int32_t input_if_std_recv(void * const buf,
                          const uint32_t len,
                          const int32_t timeoutms);

#endif // _INPUT_IF_STD_H_
