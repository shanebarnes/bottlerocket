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
#include "util_string.h"

static char spinner [] = {'|', '/', '-', '\\'};

/**
 * @see See header file for interface comments.
 */
int32_t formobj_idle(struct formobj * const obj)
{
    int32_t retval = -1;

    if ((obj == NULL) ||
        (obj->sock == NULL) ||
        (obj->dstbuf == NULL) ||
        (obj->dstlen <= 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
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

/**
 * @see See header file for interface comments.
 */
char formobj_spin(struct formobj * const obj)
{
    char retval = '\0';

    if (obj == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        obj->spincount = (obj->spincount + 1) % sizeof(spinner);
        retval = spinner[obj->spincount];
    }

    return retval;
}
