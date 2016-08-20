/**
 * @file      vector.c
 * @brief     Vector implementation.
 * @author    Shane Barnes
 * @date      09 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_mem.h"
#include "vector.h"

#include <errno.h>
#include <string.h>

struct internals
{
    uint8_t  *array;
    uint32_t  count; // allocated member count (used/unused member count)
    uint32_t  msize; // member size (bytes)
    uint32_t  vsize; // vector size (used member count)
};

/**
 * @see See header file for interface comments.
 */
bool vector_create(struct vector * const vector,
                   const uint32_t count,
                   const uint32_t size)
{
    bool retval = false;

    if ((vector == NULL) || (size == 0) || (vector->internal != NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        vector->internal = UTILMEM_CALLOC(struct internals,
                                          sizeof(struct internals),
                                          1);

        if (vector->internal == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate internals (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            vector->internal->array = UTILMEM_MALLOC(uint8_t, size, 1);

            if (vector->internal->array != NULL)
            {
                vector->internal->count = vector->internal->vsize = 1;
                vector->internal->msize = size;
                retval = vector_resize(vector, count);
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool vector_destroy(struct vector * const vector)
{
    bool retval = false;

    if (vector == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector->internal != NULL)
        {
            if (vector->internal->array != NULL)
            {
                UTILMEM_FREE(vector->internal->array);
                vector->internal->array = NULL;
            }

            UTILMEM_FREE(vector->internal);
            vector->internal = NULL;

            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool vector_resize(struct vector * const vector, const uint32_t size)
{
    bool retval = false;
    uint32_t nsize = size;

    if ((vector == NULL) || (vector->internal == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (vector->internal->vsize != size)
    {
        if (((size > vector->internal->vsize) &&
             (size > vector->internal->count)) ||
            ((size < vector->internal->vsize) &&
             (size < vector->internal->count / 2)))
        {
            // Round the array size up to the nearest power of 2 using a power
            // of two ceiling operation (note: start with a size of 1 for a
            // floor operation).
            vector->internal->count = 2;
            while (nsize >>= 1)
            {
                vector->internal->count <<= 1;
            }

            vector->internal->array = UTILMEM_REALLOC(uint8_t,
                                                      vector->internal->array,
                                                      vector->internal->msize *
                                                      vector->internal->count);

            if (vector->internal->array == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to resize vector (%d)\n",
                              __FUNCTION__,
                              errno);

                vector->internal->count = vector->internal->vsize = 0;
            }
            else
            {
                vector->internal->vsize = size;
                retval = true;
            }
        }
        else
        {
            vector->internal->vsize = size;
            retval = true;
        }
    }
    else
    {
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
void *vector_getval(struct vector * const vector, const uint32_t index)
{
    void *retval = NULL;

    if ((vector == NULL) ||
        (vector->internal == NULL) ||
        (index >= vector->internal->vsize))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = &vector->internal->array[index * vector->internal->msize];
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint32_t vector_getsize(struct vector * const vector)
{
    uint32_t retval = 0;

    if (vector == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector->internal != NULL)
        {
            retval = vector->internal->vsize;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool vector_insert(struct vector * const vector,
                   const uint32_t index,
                   void * const val)
{
    bool retval = false;

    if ((vector == NULL) ||
        (vector->internal == NULL) ||
        (index >= vector->internal->vsize))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector_resize(vector, vector->internal->vsize + 1) == true)
        {
            if (vector->internal->vsize > 1)
            {
                memmove(&vector->internal->array[(index+1)*vector->internal->msize],
                        &vector->internal->array[index*vector->internal->msize],
                        (vector->internal->vsize - index - 1) *
                         vector->internal->msize);
            }

            // There is no guarantee that memory will not overlap.
            memmove(&vector->internal->array[index*vector->internal->msize],
                    val,
                    vector->internal->msize);
            vector->internal->vsize++;
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool vector_inserttail(struct vector * const vector, void * const val)
{
    bool retval = false;

    if ((vector == NULL) || (vector->internal == NULL) || (val == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector_resize(vector, vector->internal->vsize + 1) == true)
        {
            // There is no guarantee that memory will not overlap.
            memmove(&vector->internal->array[(vector->internal->vsize - 1) * vector->internal->msize],
                    val,
                    vector->internal->msize);
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool vector_delete(struct vector * const vector, const uint32_t index)
{
    bool retval = false;

    if ((vector == NULL) ||
        (vector->internal == NULL) ||
        (index >= vector->internal->vsize))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (index < (vector->internal->vsize - 1))
        {
            memmove(&vector->internal->array[index*vector->internal->msize],
                    &vector->internal->array[(index+1)*vector->internal->msize],
                    (vector->internal->vsize - index - 1) *
                     vector->internal->msize);
        }

        retval = vector_resize(vector, vector->internal->vsize - 1);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool vector_deletetail(struct vector * const vector)
{
    bool retval = false;

    if ((vector == NULL) || (vector->internal == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector->internal->vsize > 0)
        {
            retval = vector_resize(vector, vector->internal->vsize - 1);
        }
    }

    return retval;
}
