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
#include "util_date.h"
#include "util_ioctl.h"
#include "util_string.h"
#include "util_unit.h"

#include <string.h>

static const char * formdiv = "----------------------------------------"
                              "----------------------------------------"
                              "----------------------------------------"
                              "----------------------------------------"
                              "----------------------------------------";

/**
 * @see See header file for interface comments.
 */
int32_t formchat_head(struct formobj * const obj)
{
    int32_t  retval = -1;
    uint16_t cols   = 0,
             rows   = 0,
             fill   = 0;
    uint64_t sec    = 0,
             nsec   = 0;
    char strdate[32];

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

        utildate_gettvtime(DATE_CLOCK_REALTIME, &sec, &nsec);
        utildate_gettsformat(sec,
                             UNIT_TIME_SEC,
                             "%Y-%m-%dT%H:%M:%S",
                             strdate,
                             sizeof(strdate));

        fill = 31 + strlen(obj->sock->addrpeer.sockaddrstr) + strlen(strdate);

        if (fill > cols)
        {
            fill = cols;
        }

        retval = utilstring_concat(obj->dstbuf,
                                   obj->dstlen,
                                   "chat session %s opened at %s.%06u%.*s\a\n",
                                   obj->sock->addrpeer.sockaddrstr,
                                   strdate,
                                   (uint32_t)(nsec / 1000),
                                   cols - fill,
                                   formdiv);
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

        tmpbytes = utilstring_concat(obj->dstbuf,
                                     obj->dstlen,
                                     "%*s[%s (%4d bytes)]\a\n",
                                     lmargin,
                                     "",
                                     obj->sock->addrpeer.sockaddrstr,
                                     obj->srclen - 1);

        if (tmpbytes > 0)
        {
            retval += tmpbytes;
        }

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
    int32_t  retval = -1;
    uint16_t cols   = 0,
             rows   = 0,
             fill   = 0;
    uint64_t sec    = 0,
             nsec   = 0;
    uint64_t durationusec = 0,
             recvratebps  = 0,
             sendratebps  = 0;
    char strdate[32],
         strrecvbytes[16],
         strsendbytes[16],
         strrecvrate[16],
         strsendrate[16];

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

        durationusec = obj->sock->info.stopusec - obj->sock->info.startusec;
        recvratebps  = obj->sock->info.recvbytes * 8 * UNIT_TIME_USEC / durationusec;
        sendratebps  = obj->sock->info.sendbytes * 8 * UNIT_TIME_USEC / durationusec;

        utildate_gettvtime(DATE_CLOCK_REALTIME, &sec, &nsec);
        utildate_gettsformat(sec,
                             UNIT_TIME_SEC,
                             "%Y-%m-%dT%H:%M:%S",
                             strdate,
                             sizeof(strdate));

        utilunit_getdecformat(10,
                              3,
                              obj->sock->info.recvbytes,
                              strrecvbytes,
                              sizeof(strrecvbytes));
        utilunit_getdecformat(10,
                              3,
                              obj->sock->info.sendbytes,
                              strsendbytes,
                              sizeof(strsendbytes));
        utilunit_getdecformat(10,
                              3,
                              recvratebps,
                              strrecvrate,
                              sizeof(strrecvrate));
        utilunit_getdecformat(10,
                              3,
                              sendratebps,
                              strsendrate,
                              sizeof(strsendrate));

        fill = 31 + strlen(obj->sock->addrpeer.sockaddrstr) + strlen(strdate);

        if (fill > cols)
        {
            fill = cols;
        }

        retval = utilstring_concat(obj->dstbuf,
                                   obj->dstlen,
                                   "send stats: %sB / %sbps "
                                   "recv stats: %sB / %sbps\n"
                                   "chat session %s closed at %s.%06u%.*s\a\n",
                                   strsendbytes,
                                   strsendrate,
                                   strrecvbytes,
                                   strrecvrate,
                                   obj->sock->addrpeer.sockaddrstr,
                                   strdate,
                                   (uint32_t)(nsec / 1000),
                                   cols - fill,
                                   formdiv);
    }

    return retval;
}
