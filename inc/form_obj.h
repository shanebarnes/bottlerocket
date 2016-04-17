/**
 * @file      form_obj.h
 * @brief     Presentation layer format interface.
 * @author    Shane Barnes
 * @date      16 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _FORM_OBJ_H_
#define _FORM_OBJ_H_

#include "sock_obj.h"
#include "system_types.h"

struct formobj;

struct formobj_ops
{
    /**
     * @brief Create and format a data header and store it in the format object
     *        destination buffer.
     *
     * @param[in,out] obj A pointer to a format object.
     *
     * @return The number of formatted bytes in the format object destination
     *         buffer (-1 on error).
     */
    int32_t (*form_head)(struct formobj * const obj);

    /**
     * @brief Create and format a data body and store it in the format object
     *        destination buffer.
     *
     * @param[in,out] obj A pointer to a format object.
     *
     * @return The number of formatted bytes in the format object destination
     *         buffer (-1 on error).
     */
    int32_t (*form_body)(struct formobj * const obj);

    /**
     * @brief Create and format a data footer and store it in the format object
     *        destination buffer.
     *
     * @param[in,out] obj A pointer to a format object.
     *
     * @return The number of formatted bytes in the format object destination
     *         buffer (-1 on error).
     */
    int32_t (*form_foot)(struct formobj * const obj);
};

struct formobj
{
    struct formobj_ops  ops;
    struct sockobj     *sock;
    void               *srcbuf,
                       *dstbuf;
    int32_t             srclen,
                        dstlen;
};

#endif // _FORM_OBJ_H_
