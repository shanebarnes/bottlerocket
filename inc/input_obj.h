/**
 * @file      input_obj.h
 * @brief     Input object interface.
 * @author    Shane Barnes
 * @date      24 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _INPUT_OBJECT_H_
#define _INPUT_OBJECT_H_

#include "system_types.h"

struct inputobj_ops
{
    /**
     * @brief Receive a status message from an input object.
     *
     * @param[in,out] buf       A pointer to a buffer to fill with a status
     *                          message received from an input object.
     * @param[in]     len       The maximum number of bytes in the receive
     *                          buffer.
     * @param[in]     timeoutms The receive timeout in milliseconds (-1 to block
     *                          indefinitely while waiting for a status message
     *                          from an input object.
     *
     * @return The number of bytes received from an input object (-1 on error).
     */
    int32_t (*inputobj_recv)(void * const buf,
                             const uint32_t len,
                             const int32_t timeoutms);
};

#endif // _INPUT_OBJ_H_
