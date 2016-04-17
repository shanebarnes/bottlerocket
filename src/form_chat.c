/**
 * @file      form_chat.c
 * @brief     Chat mode presentation layer format implementation.
 * @author    Shane Barnes
 * @date      16 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "form_chat.h"
#include "logger.h"
#include "util_ioctl.h"
#include "util_string.h"

/**
 * @see See header file for interface comments.
 */
int32_t formchat_head(struct formobj * const obj)
{
    int32_t  retval  = -1;
    uint16_t cols    = 0,
             rows    = 0;
    int32_t  lmargin = 0,
             rmargin = 0;

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
        utilioctl_gettermsize(&rows, &cols);
        rmargin = cols;
        lmargin = cols / 2;

        // Add tag to each message including an audible beep.
        // @todo Make audible beep optional.
        retval = utilstring_concat(obj->dstbuf,
                                   obj->dstlen,
                                   "%*s[%s (%4d bytes)]\a\n",
                                   lmargin,
                                   "",
                                   obj->sock->addrpeer.sockaddrstr,
                                   obj->srclen);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t formchat_body(struct formobj * const obj)
{
    int32_t  retval   = -1;
    uint16_t cols     = 0,
             rows     = 0;
    int32_t  lmargin  = 0,
             rmargin  = 0;
    int32_t  srcbytes = 0,
             tmpbytes = 0;

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
        retval = 0;
        utilioctl_gettermsize(&rows, &cols);
        rmargin = cols;
        lmargin = cols / 2;

        while (srcbytes < obj->srclen)
        {
            if ((obj->srclen - srcbytes) < (rmargin - lmargin))
            {
                rmargin = lmargin + (obj->srclen - srcbytes);
            }

            // Set the left margin and create a substring that is left justified.
            tmpbytes = utilstring_concat(obj->dstbuf + retval,
                                         obj->dstlen - retval,
                                         "%*s%-*.*s\n",
                                         lmargin,
                                         "",
                                         rmargin - lmargin,
                                         rmargin - lmargin,
                                         obj->srcbuf + srcbytes);

            if (tmpbytes > 0)
            {
                srcbytes += (rmargin - lmargin);
                retval   += tmpbytes;
            }
            else
            {
                break; // Prevent spinning.
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t formchat_foot(struct formobj * const obj)
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
