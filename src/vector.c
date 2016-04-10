/**
 * @file      vector.c
 * @brief     Vector implementation.
 * @author    Shane Barnes
 * @date      09 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "vector.h"
#include "logger.h"

#include <errno.h>

/**
 * @see See header file for interface comments.
 */
bool vector_create(struct vector * const vector, const uint32_t size)
{
    bool retval = false;

    if ((vector == NULL) ||
        (vector->array != NULL) ||
        (vector->asize > 0) ||
        (vector->vsize > 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = vector_resize(vector, size);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool vector_destroy(struct vector * const vector)
{
    bool retval = false;

    if ((vector == NULL) ||
        (vector->array == NULL) ||
        (vector->asize < vector->vsize))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        free(vector->array);
        vector->array = NULL;
        vector->asize = vector->vsize = 0;

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

    if (vector == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (vector->vsize != size)
    {
        if (((size > vector->vsize) && (size > vector->asize)) ||
            ((size < vector->vsize) && (size < vector->asize / 2)))
        {
            // Round the array size up to the nearest power of 2 using a power
            // of two ceiling operation (note: start with a size of 1 for a
            // floor operation).
            vector->asize = 2;
            while (nsize >>= 1)
            {
                vector->asize <<= 1;
            }

            vector->array = realloc(vector->array,
                                    sizeof(void *) * vector->asize);

            if (vector == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to resize vector (%d)\n",
                              __FUNCTION__,
                              errno);

                vector->asize = vector->vsize = 0;
            }
            else
            {
                vector->vsize = size;
                retval = true;
            }
        }
        else
        {
            vector->vsize = size;
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
void *vector_get(struct vector * const vector, const uint32_t index)
{
    void *retval = NULL;

    if ((vector == NULL) || (index >= vector->vsize))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = vector->array[index];
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

    if ((vector == NULL) || (index >= vector->vsize))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector_resize(vector, vector->vsize + 1) == true)
        {
            if (vector->vsize > 1)
            {
                for (i = vector->vsize - 2; i >= index; i--)
                {
                    vector->array[i+1] = vector->array[i];
                }
            }

            vector->array[index] = val;
            vector->vsize++;
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

    if (vector == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector_resize(vector, vector->vsize + 1) == true)
        {
            vector->array[vector->vsize - 1] = val;
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

    if ((vector == NULL) || (index >= vector->vsize))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (vector->vsize > 1)
        {
            for (i = index; i < vector->vsize - 1; i++)
            {
                vector->array[i] = vector->array[i+1];
            }
        }

        retval = vector_resize(vector, vector->vsize - 1);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
 bool vector_deletetail(struct vector * const vector)
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
        retval = vector_resize(vector, vector->vsize - 1);
    }

    return retval;
}
