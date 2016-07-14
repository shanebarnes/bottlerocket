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

struct internals
{
    void     **array;
    uint32_t   asize;
    uint32_t   vsize;
};

/**
 * @see See header file for interface comments.
 */
bool vector_create(struct vector * const vector, const uint32_t size)
{
    bool retval = false;

    if ((vector == NULL) || (vector->internal != NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        vector->internal = calloc(1, sizeof(struct internals));

        if (vector->internal == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate internals (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            vector->internal->array = UTILMEM_MALLOC(void*, 1);

            if (vector->internal->array != NULL)
            {
                vector->internal->asize = vector->internal->vsize = 1;
                retval = vector_resize(vector, size);
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
        }

        retval = true;
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
             (size > vector->internal->asize)) ||
            ((size < vector->internal->vsize) &&
             (size < vector->internal->asize / 2)))
        {
            // Round the array size up to the nearest power of 2 using a power
            // of two ceiling operation (note: start with a size of 1 for a
            // floor operation).
            vector->internal->asize = 2;
            while (nsize >>= 1)
            {
                vector->internal->asize <<= 1;
            }

            vector->internal->array = realloc(vector->internal->array,
                                              sizeof(void *) * vector->internal->asize);

            if (vector->internal->array == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to resize vector (%d)\n",
                              __FUNCTION__,
                              errno);

                vector->internal->asize = vector->internal->vsize = 0;
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
        retval = vector->internal->array[index];
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
    uint32_t i = 0;

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
                for (i = vector->internal->vsize - 2; i >= index; i--)
                {
                    vector->internal->array[i+1] = vector->internal->array[i];
                }
            }

            vector->internal->array[index] = val;
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

    if ((vector == NULL) || (vector->internal == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector_resize(vector, vector->internal->vsize + 1) == true)
        {
            vector->internal->array[vector->internal->vsize - 1] = val;
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
    uint32_t i = 0;

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
        if (vector->internal->vsize > 1)
        {
            for (i = index; i < vector->internal->vsize - 1; i++)
            {
                vector->internal->array[i] = vector->internal->array[i+1];
            }
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
        retval = vector_resize(vector, vector->internal->vsize - 1);
    }

    return retval;
}
