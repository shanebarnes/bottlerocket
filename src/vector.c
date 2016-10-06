/**
 * @file      vector.c
 * @brief     Vector implementation.
 * @author    Shane Barnes
 * @date      09 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "util_debug.h"
#include "util_mem.h"
#include "vector.h"

#include <errno.h>
#include <string.h>

struct vector_priv
{
    uint8_t  *array;
    uint32_t  count; // allocated member count (used/unused member count)
    uint32_t  msize; // member size (bytes)
    uint32_t  vsize; // vector size (used member count)
};

bool vector_create(struct vector * const vector,
                   const uint32_t count,
                   const uint32_t size)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((vector != NULL) &&
                         (size > 0) &&
                         (vector->priv == NULL)))
    {
        vector->priv = UTILMEM_CALLOC(struct privs,
                                      sizeof(struct vector_priv),
                                      1);

        if (vector->priv == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate privs (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            vector->priv->array = UTILMEM_MALLOC(uint8_t, size, 1);

            if (vector->priv->array != NULL)
            {
                vector->priv->count = vector->priv->vsize = 1;
                vector->priv->msize = size;
                ret = vector_resize(vector, count);
            }
        }
    }

    return ret;
}

bool vector_destroy(struct vector * const vector)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(vector != NULL))
    {
        if (vector->priv != NULL)
        {
            if (vector->priv->array != NULL)
            {
                UTILMEM_FREE(vector->priv->array);
                vector->priv->array = NULL;
            }

            UTILMEM_FREE(vector->priv);
            vector->priv = NULL;

            ret = true;
        }
    }

    return ret;
}

bool vector_resize(struct vector * const vector, const uint32_t size)
{
    bool ret = false;
    uint32_t nsize = size;

    if (!UTILDEBUG_VERIFY((vector != NULL) && (vector->priv != NULL)))
    {
        // Do nothing.
    }
    else if (vector->priv->vsize != size)
    {
        if (((size > vector->priv->vsize) &&
             (size > vector->priv->count)) ||
            ((size < vector->priv->vsize) &&
             (size < vector->priv->count / 2)))
        {
            // Round the array size up to the nearest power of 2 using a power
            // of two ceiling operation (note: start with a size of 1 for a
            // floor operation).
            vector->priv->count = 2;
            while (nsize >>= 1)
            {
                vector->priv->count <<= 1;
            }

            vector->priv->array = UTILMEM_REALLOC(uint8_t,
                                                  vector->priv->array,
                                                  vector->priv->msize *
                                                  vector->priv->count);

            if (vector->priv->array == NULL)
            {
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: failed to resize vector (%d)\n",
                              __FUNCTION__,
                              errno);

                vector->priv->count = vector->priv->vsize = 0;
            }
            else
            {
                vector->priv->vsize = size;
                ret = true;
            }
        }
        else
        {
            vector->priv->vsize = size;
            ret = true;
        }
    }
    else
    {
        ret = true;
    }

    return ret;
}

void *vector_gettail(struct vector * const vector)
{
    void *ret = NULL;
    uint32_t size = vector_getsize(vector);

    if (size > 0)
    {
        ret = &vector->priv->array[(size - 1) * vector->priv->msize];
    }

    return ret;
}

void *vector_getval(struct vector * const vector, const uint32_t index)
{
    void *ret = NULL;

    if (UTILDEBUG_VERIFY((vector != NULL) &&
                         (vector->priv != NULL) &&
                         (index < vector->priv->vsize)))
    {
        ret = &vector->priv->array[index * vector->priv->msize];
    }

    return ret;
}

uint32_t vector_getsize(struct vector * const vector)
{
    uint32_t ret = 0;

    if (UTILDEBUG_VERIFY(vector != NULL))
    {
        if (vector->priv != NULL)
        {
            ret = vector->priv->vsize;
        }
    }

    return ret;
}

bool vector_insert(struct vector * const vector,
                   const uint32_t index,
                   void * const val)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((vector != NULL) &&
                         (vector->priv != NULL) &&
                         (index < vector->priv->vsize)))
    {
        if (vector_resize(vector, vector->priv->vsize + 1))
        {
            if (vector->priv->vsize > 1)
            {
                memmove(&vector->priv->array[(index+1)*vector->priv->msize],
                        &vector->priv->array[index*vector->priv->msize],
                        (vector->priv->vsize - index - 1) *
                         vector->priv->msize);
            }

            // There is no guarantee that memory will not overlap.
            memmove(&vector->priv->array[index*vector->priv->msize],
                    val,
                    vector->priv->msize);
            vector->priv->vsize++;
            ret = true;
        }
    }

    return ret;
}

bool vector_inserttail(struct vector * const vector, void * const val)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((vector != NULL) &&
                         (vector->priv != NULL) &&
                         (val != NULL)))
    {
        if (vector_resize(vector, vector->priv->vsize + 1))
        {
            // There is no guarantee that memory will not overlap.
            memmove(&vector->priv->array[(vector->priv->vsize - 1) * vector->priv->msize],
                    val,
                    vector->priv->msize);
            ret = true;
        }
    }

    return ret;
}

bool vector_delete(struct vector * const vector, const uint32_t index)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((vector != NULL) &&
                         (vector->priv != NULL) &&
                         (index < vector->priv->vsize)))
    {
        if (index < (vector->priv->vsize - 1))
        {
            memmove(&vector->priv->array[index*vector->priv->msize],
                    &vector->priv->array[(index+1)*vector->priv->msize],
                    (vector->priv->vsize - index - 1) *
                     vector->priv->msize);
        }

        ret = vector_resize(vector, vector->priv->vsize - 1);
    }

    return ret;
}

bool vector_deletetail(struct vector * const vector)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((vector != NULL) && (vector->priv != NULL)))
    {
        if (vector->priv->vsize > 0)
        {
            ret = vector_resize(vector, vector->priv->vsize - 1);
        }
    }

    return ret;
}
