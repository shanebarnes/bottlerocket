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
#include "util_date.h"
#include "util_ioctl.h"
#include "util_string.h"
#include "util_unit.h"

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

        obj->timeoutusec = obj->sock->info.startusec;

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
    int32_t  retval   = -1;
    uint64_t diffusec = 0, timeusec = 0;
    struct util_date_diff diff;
    char strrecvbytes[16], strsendbytes[16];
    char strrecvrate[16], strsendrate[16];

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
        timeusec = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);

        if (timeusec >= obj->timeoutusec)
        {
            diffusec = utildate_gettsdiff(obj->sock->info.startusec,
                                          timeusec,
                                          UNIT_TIME_USEC,
                                          &diff);

            utilunit_getdecformat(10, 3, obj->sock->info.recvbytes, strrecvbytes, sizeof(strrecvbytes));
            utilunit_getdecformat(10, 3, obj->sock->info.sendbytes, strsendbytes, sizeof(strsendbytes));
            utilunit_getdecformat(10, 3, 0, strrecvrate, sizeof(strrecvrate));
            utilunit_getdecformat(10, 3, 0, strsendrate, sizeof(strsendrate));

            retval = utilstring_concat(obj->dstbuf,
                                       obj->dstlen,
                                       "[%4u] "
                                       "%21s > %-21s "
                                       "%3u%% "
                                       "[%.*s%.*s] "
                                       "%9sbps / "
                                       "%9sbps | "
                                       "%9sB / "
                                       "%9sB | "
                                       "%02u:%02u:%02u:%02u.%03u\n",
                                       1,
                                       obj->sock->addrself.sockaddrstr,
                                       obj->sock->addrpeer.sockaddrstr,
                                       0,
                                       5,
                                       "==========",
                                       10 - 5,
                                       "          ",
                                       strrecvrate,
                                       strsendrate,
                                       strrecvbytes,
                                       strsendbytes,
                                       diff.day + (diff.week * 7),
                                       diff.hour,
                                       diff.min,
                                       diff.sec,
                                       diff.msec);

            obj->timeoutusec += 1 * UNIT_TIME_USEC;

            // Correct timeout in the event that the new timeout has already
            // expired.
            if (obj->timeoutusec <= timeusec)
            {
                obj->timeoutusec = timeusec + 1 * UNIT_TIME_USEC;
            }
        }
        else
        {
            retval = 0;
        }
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
        obj->timeoutusec = 0;
        retval = formperf_body(obj);
    }

    return retval;
}
