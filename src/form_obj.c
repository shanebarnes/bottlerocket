/**
 * @file      form_obj.c
 * @brief     Presentation layer format implementation.
 * @author    Shane Barnes
 * @date      29 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "form_obj.h"
#include "logger.h"
#include "util_debug.h"
#include "util_mem.h"
#include "util_string.h"

static char spinner [] = {'|', '/', '-', '\\'};

bool formobj_create(struct formobj * const obj, const int32_t bufsize)
{
    bool ret = false;

    if (!UTILDEBUG_VERIFY((obj != NULL) &&
                          (bufsize > 0) &&
                          (obj->srcbuf == NULL) &&
                          (obj->dstbuf == NULL)))
    {
        // Do nothing.
    }
    else if ((obj->srcbuf = UTILMEM_CALLOC(char,
                                           sizeof(char),
                                           bufsize)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: form source buffer allocation failed\n",
                      __FUNCTION__);
    }
    else if ((obj->dstbuf = UTILMEM_CALLOC(char,
                                           sizeof(char),
                                           bufsize)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: form destination buffer allocation failed\n",
                      __FUNCTION__);
        UTILMEM_FREE(obj->srcbuf);
        obj->srcbuf = NULL;
    }
    else
    {
        obj->srclen = obj->dstlen = bufsize;
        ret = true;
    }

    return ret;
}

bool formobj_destroy(struct formobj * const obj)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->srcbuf != NULL) &&
                         (obj->dstbuf != NULL)))
    {
        UTILMEM_FREE(obj->srcbuf);
        obj->srcbuf = NULL;
        obj->srclen = 0;

        UTILMEM_FREE(obj->dstbuf);
        obj->dstbuf = NULL;
        obj->dstlen = 0;

        obj->ops.form_create  = NULL;
        obj->ops.form_destroy = NULL;
        obj->ops.form_head    = NULL;
        obj->ops.form_body    = NULL;
        obj->ops.form_foot    = NULL;

        ret = true;
    }

    return ret;
}

int32_t formobj_idle(struct formobj * const obj)
{
    int32_t retval = -1;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->sock != NULL) &&
                         (obj->dstbuf != NULL) &&
                         (obj->dstlen > 0)))
    {
        retval = utilstring_concat(obj->dstbuf,
                                   obj->dstlen,
                                   "%s %s %s %c%*s",
                                   "Listening on",
                                   (obj->sock->conf.type == SOCK_STREAM ?
                                       "TCP" :
                                       "UDP"),
                                   obj->sock->addrself.sockaddrstr,
                                   formobj_spin(obj),
                                   0,
                                   " ");
    }

    return retval;
}

char formobj_spin(struct formobj * const obj)
{
    char retval = '\0';

    if (UTILDEBUG_VERIFY(obj != NULL))
    {
        obj->spincount = (obj->spincount + 1) % sizeof(spinner);
        retval = spinner[obj->spincount];
    }

    return retval;
}
