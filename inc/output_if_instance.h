/**
 * @file   output_if_instance.h
 * @author Shane Barnes
 * @date   13 Mar 2016
 * @brief  Output interface instance interface.
 */

#ifndef _OUTPUT_IF_INSTANCE_H_
#define _OUTPUT_IF_INSTANCE_H_

#include "system_types.h"

struct output_if_ops
{
    // @todo: JSON formatted buffers for the data plane.

    /**
     * @brief Send a status message to an output interface.
     *
     * @param[in] buf A pointer to a buffer containing a status message to send
     *                to an output interface.
     * @param[in] len The maximum number of bytes in the send buffer.
     *
     * @return The number of bytes sent to an output interface (-1 on error).
     */
    int32_t (*oio_send)(void * const buf, const uint32_t len);
};

#endif // _OUTPUT_IF_INSTANCE_H_
