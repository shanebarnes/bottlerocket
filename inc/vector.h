/**
 * @file      vector.h
 * @brief     Vector interface.
 * @author    Shane Barnes
 * @date      09 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "system_types.h"

struct vector
{
    void     **array;
    uint32_t   asize;
    uint32_t   vsize;
};

/**
 * @brief Create a vector.
 *
 * @param[in,out] vector A pointer to a vector.
 * @param[in]     size   The initial/default size of the vector.
 *
 * @return True if a vector was created.
 */
bool vector_create(struct vector * const vector, const uint32_t size);

/**
 * @brief Destroy a vector.
 *
 * @param[in,out] vector A pointer to a vector.
 *
 * @return True if a vector was destroyed.
 */
bool vector_destroy(struct vector * const vector);

/**
 * @brief Resize a vector.
 *
 * @param[in,out] vector A pointer to a vector.
 * @param[in]     size   The new size of the vector.
 *
 * @return True if a vector was resized.
 */
bool vector_resize(struct vector * const vector, const uint32_t size);

/**
 * @brief Get a value from a position in a vector.
 *
 * @param[in] vector A pointer to a vector.
 * @param[in] index  A position in a vector.
 *
 * @return A pointer to a value (NULL on error).
 */
void *vector_get(struct vector * const vector, const uint32_t index);

/**
 * @brief Insert a value at a position in a vector.
 *
 * @param[in,out] vector A pointer to a vector.
 * @param[in]     index  A position in the vector.
 * @param[in]     val    A value to insert at the end of a vector.
 *
 * @return True if a value was inserted at a position in a vector.
 */
bool vector_insert(struct vector * const vector,
                   const uint32_t index,
                   void * const val);

/**
 * @brief Insert a value at the end of a vector.
 *
 * @param[in,out] vector A pointer to a vector.
 * @param[in]     val    A value to insert at the end of a vector.
 *
 * @return True if a value was inserted at the end of a vector.
 */
bool vector_inserttail(struct vector * const vector, void * const val);

/**
 * @brief Delete a value at position in a vector.
 *
 * @param[in,out] vector A pointer to a vector.
 * @param[in]     index  A position in a vector.
 *
 * @return True if a value was deleted at a position in a vector.
 */
bool vector_delete(struct vector * const vector, const uint32_t index);

/**
 * @brief Delete a value at the end of a vector.
 *
 * @param[in,out] vector A pointer to a vector.
 *
 * @return True if a value was deleted from the end of a vector.
 */
bool vector_deletetail(struct vector * const vector);

#endif // _VECTOR_H_
