/**
 * @file      arg_obj.c
 * @brief     Bottlerocket argument object implementation.
 * @author    Shane Barnes
 * @date      25 Oct 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "arg_obj.h"
#include "util_debug.h"
#include "util_inet.h"
#include "util_string.h"
#include "util_unit.h"

#define arg_noobjptr NULL
#define arg_optional true
#define arg_required false
#define val_optional true
#define val_required false

bool argobj_copyipaddr(const struct argobj * const arg,
                       const char * const src,
                       void * const dst)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((arg != NULL) &&
                         (arg->args != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)))
    {
        if ((arg->minval != NULL) && (false))
        {
            // Do nothing.
        }
        else if ((arg->maxval != NULL) && (false))
        {
            // Do nothing.
        }
        else
        {
            ret = utilinet_getaddrfromhost(src,
                                           arg->args->family,
                                           dst,
                                           INET6_ADDRSTRLEN);
        }
    }

    return ret;
}

bool argobj_copyuint16(const struct argobj * const arg,
                       const char * const src,
                       void * const dst)
{
    bool ret = false;
    uint32_t max, min, val;

    if (UTILDEBUG_VERIFY((arg != NULL) && (src != NULL) && (dst != NULL)))
    {
        if ((utilstring_parse(src, "%u", &val) == 1))
        {
            if ((arg->minval != NULL) &&
                ((utilstring_parse(arg->minval, "%u", &min) != 1) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((arg->maxval != NULL) &&
                     ((utilstring_parse(arg->maxval, "%u", &max) != 1) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint16_t*)dst = (uint16_t)val;
                ret = true;
            }
        }
    }

    return ret;
}

bool argobj_copyint32(const struct argobj * const arg,
                      const char * const src,
                      void * const dst)
{
    bool ret = false;
    int32_t max, min, val;

    if (UTILDEBUG_VERIFY((arg != NULL) && (src != NULL) && (dst != NULL)))
    {
        if ((utilstring_parse(src, "%d", &val) == 1))
        {
            if ((arg->minval != NULL) &&
                ((utilstring_parse(arg->minval, "%d", &min) != 1) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((arg->maxval != NULL) &&
                     ((utilstring_parse(arg->maxval, "%d", &max) != 1) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(int32_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}

bool argobj_copyuint32(const struct argobj * const arg,
                       const char * const src,
                       void * const dst)
{
    bool ret = false;
    uint32_t max, min, val;

    if (UTILDEBUG_VERIFY((arg != NULL) && (src != NULL) && (dst != NULL)))
    {
        if ((utilstring_parse(src, "%u", &val) == 1))
        {
            if ((arg->minval != NULL) &&
                ((utilstring_parse(arg->minval, "%u", &min) != 1) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((arg->maxval != NULL) &&
                     ((utilstring_parse(arg->maxval, "%u", &max) != 1) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint32_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}

bool argobj_copyrateunit(const struct argobj * const arg,
                         const char * const src,
                         void  * const dst)
{
    bool ret = false;
    int64_t max, min, val;

    if (UTILDEBUG_VERIFY((arg != NULL) && (src != NULL) && (dst != NULL)))
    {
        if ((val = utilunit_getbitrate(src)) >= 0)
        {
            if ((arg->minval != NULL) &&
                (((min = utilunit_getbitrate(arg->minval)) < 0) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((arg->maxval != NULL) &&
                     (((max = utilunit_getbitrate(arg->maxval)) < 0) ||
                      (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint64_t*)dst = (uint64_t)val;
                ret = true;
            }
        }
    }

    return ret;
}

bool argobj_copybyteunit(const struct argobj * const arg,
                         const char * const src,
                         void * const dst)
{
    bool ret = false;
    uint64_t max, min, val;

    if (UTILDEBUG_VERIFY((arg != NULL) && (src != NULL) && (dst != NULL)))
    {
        if ((val = utilunit_getbytes(src)) > 0)
        {
            if ((arg->minval != NULL) &&
                (((min = utilunit_getbytes(arg->minval)) == 0) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((arg->maxval != NULL) &&
                    (((max = utilunit_getbytes(arg->maxval)) == 0) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint64_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}

bool argobj_copytimeunit(const struct argobj * const arg,
                         const char * const src,
                         void * const dst)
{
    bool ret = false;
    uint64_t max, min, val;

    if (UTILDEBUG_VERIFY((arg != NULL) && (src != NULL) && (dst != NULL)) )
    {
        if ((val = utilunit_getsecs(src, UNIT_TIME_USEC)) > 0)
        {
            if ((arg->minval != NULL) &&
                (((min = utilunit_getsecs(arg->minval, UNIT_TIME_USEC)) == 0) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((arg->maxval != NULL) &&
                    (((max = utilunit_getsecs(arg->maxval, UNIT_TIME_USEC)) == 0) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint64_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}
