/**
 * @file      form_perf.c
 * @brief     Performance mode presentation layer format implementation.
 * @author    Shane Barnes
 * @date      18 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "form_perf.h"
#include "logger.h"
#include "util_ioctl.h"
#include "util_string.h"

/**
 * @see See header file for interface comments.
 */
int32_t formperf_head(struct formobj * const obj)
{
    int32_t  retval  = -1;
    //uint16_t cols    = 0,
    //         rows    = 0;

    if ((obj == NULL) ||
        (obj->sock == NULL) ||
        (obj->srcbuf == NULL) ||
        (obj->srclen <= 0) ||
        (obj->dstbuf == NULL) ||
        (obj->dstlen <= 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        // @todo Format should be responsive based on the number of columns.
        //utilioctl_gettermsize(&rows, &cols);

        retval = utilstring_concat(obj->dstbuf,
                                   obj->dstlen,
                                   "%6s %21s   %-21s %17s %27s %25s %17s\n",
                                   "Con ID",
                                   "Client",         // or self?
                                   "Server",         // or peer?
                                   "Progress",
                                   "Goodput",        // or "Bit Rate"?
                                   "Bytes Received", // or "Bytes Sent"?
                                   "Elapsed Time");
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t formperf_body(struct formobj * const obj)
{
    int32_t  retval = -1;

    if ((obj == NULL) ||
        (obj->sock == NULL) ||
        (obj->srcbuf == NULL) ||
        (obj->srclen <= 0) ||
        (obj->dstbuf == NULL) ||
        (obj->dstlen <= 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {

    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t formperf_foot(struct formobj * const obj)
{
    int32_t retval = -1;

    if ((obj == NULL) ||
        (obj->sock == NULL) ||
        (obj->srcbuf == NULL) ||
        (obj->srclen <= 0) ||
        (obj->dstbuf == NULL) ||
        (obj->dstlen <= 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {

    }

    return retval;
}
