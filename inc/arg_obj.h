/**
 * @file      arg_obj.h
 * @brief     Bottlerocket argument object interface.
 * @author    Shane Barnes
 * @date      25 Oct 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _ARG_OBJ_H_
#define _ARG_OBJ_H_

#include "args.h"
#include "system_types.h"

struct args_obj;

struct argobj
{
    const bool       status;
    const char      *lname;  // Attribute long name (e.g., --argument)
    const char       sname;  // Attribute short name (e.g., -a)
    const char      *desc;   // Description
    const char      *dval;   // Default value
    const char      *minval; // Minimum value
    const char      *maxval; // Maximum value
    const bool       oval;   // Optional value
    const bool       oarg;   // Optional attribute
    const uint64_t   cflags; // Conflict flags (i.e., incompatible options)
    struct args_obj *args;   // Pointer to current argument values.
                             // Option copy function pointer.
    bool            (*copy)(const struct argobj * const arg,
                            const char * const src,
                            void * const dst);
    void            *dest;   // Copy destination.
};

/**
 * @brief Copy an IP address value from a source memory area to a destination
 *        memory area if it satisfies the restrictions contained in the
 *        argument object.
 *
 * @param[in]     arg A pointer to an argument object.
 * @param[in]     src A pointer to an IP address.
 * @param[in,out] dst A pointer to a destination buffer.
 *
 * @return True if an IP address value was copied to a destination memory area.
 */
bool argobj_copyipaddr(const struct argobj * const arg,
                       const char * const src,
                       void * const dst);

/**
 * @brief Copy a 16-bit unsigned integer value  from a source memory area to a
 *        destination memory area if it satisfies the restrictions contained in
 *        the argument object.
 *
 * @param[in]     arg A pointer to an argument object.
 * @param[in]     src A pointer to a 16-bit unsigned integer value.
 * @param[in,out] dst A pointer to a destination buffer.
 *
 * @return True if a 16-bit unsigned integer value was copied to a destination
 *         memory area.
 */
bool argobj_copyuint16(const struct argobj * const arg,
                       const char * const src,
                       void * const dst);

/**
 * @brief Copy a 32-bit signed integer value  from a source memory area to a
 *        destination memory area if it satisfies the restrictions contained in
 *        the argument object.
 *
 * @param[in]     arg A pointer to an argument object.
 * @param[in]     src A pointer to a 32-bit signed integer value.
 * @param[in,out] dst A pointer to a destination buffer.
 *
 * @return True if a 32-bit signed integer value was copied to a destination
 *         memory area.
 */
bool argobj_copyint32(const struct argobj * const arg,
                      const char * const src,
                      void * const dst);

/**
 * @brief Copy a 32-bit unsigned integer value  from a source memory area to a
 *        destination memory area if it satisfies the restrictions contained in
 *        the argument object.
 *
 * @param[in]     arg A pointer to an argument object.
 * @param[in]     src A pointer to a 32-bit unsigned integer value.
 * @param[in,out] dst A pointer to a destination buffer.
 *
 * @return True if a 32-bit unsigned integer value was copied to a destination
 *         memory area.
 */
bool argobj_copyuint32(const struct argobj * const arg,
                       const char * const src,
                       void * const dst);

/**
 * @brief Copy a rate unit value from a source memory area to a destination
 *        memory area if it satisfies the restrictions contained in the
 *        argument object.
 *
 * @param[in]     arg A pointer to an argument object.
 * @param[in]     src A pointer to an IP address.
 * @param[in,out] dst A pointer to a destination buffer.
 *
 * @return True if a rate unit value was copied to a destination memory area.
 */
bool argobj_copyrateunit(const struct argobj * const arg,
                         const char * const src,
                         void  * const dst);

/**
 * @brief Copy a byte unit value from a source memory area to a destination
 *        memory area if it satisfies the restrictions contained in the
 *        argument object.
 *
 * @param[in]     arg A pointer to an argument object.
 * @param[in]     src A pointer to an IP address.
 * @param[in,out] dst A pointer to a destination buffer.
 *
 * @return True if a byte unit value was copied to a destination memory area.
 */
bool argobj_copybyteunit(const struct argobj * const arg,
                         const char * const src,
                         void * const dst);

/**
 * @brief Copy a time unit value from a source memory area to a destination
 *        memory area if it satisfies the restrictions contained in the
 *        argument object.
 *
 * @param[in]     arg A pointer to an argument object.
 * @param[in]     src A pointer to an IP address.
 * @param[in,out] dst A pointer to a destination buffer.
 *
 * @return True if a time unit value was copied to a destination memory area.
 */
bool argobj_copytimeunit(const struct argobj * const arg,
                         const char * const src,
                         void * const dst);

#endif // _ARG_OBJ_H_
