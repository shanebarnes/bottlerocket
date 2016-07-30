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
    char recvwin[16], sendwin[16];
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

        utilunit_getdecformat(10,
                              3,
                              obj->sock->info.recv.winsize,
                              recvwin,
                              sizeof(recvwin));

        utilunit_getdecformat(10,
                              3,
                              obj->sock->info.send.winsize,
                              sendwin,
                              sizeof(sendwin));

        retval = utilstring_concat(obj->dstbuf,
                                   obj->dstlen,
                                   "rwin: %sB, swin: %sB\n"
                                   "%6s %21s   %-21s %17s %27s %25s %17s\n",
                                   recvwin,
                                   sendwin,
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
    uint64_t diffusec = 0;
    uint32_t progress = 0;
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
        if ((obj->tsus >= obj->timeoutusec) &&
            (obj->tsus > obj->sock->info.startusec))
        {
            diffusec = utildate_gettsdiff(obj->sock->info.startusec,
                                          obj->tsus,
                                          UNIT_TIME_USEC,
                                          &diff);

            if (obj->sock->conf.timelimitusec > 0)
            {
                progress = (uint32_t)(diffusec * 100 /
                                      obj->sock->conf.timelimitusec);
            }
            else if (obj->sock->conf.datalimitbyte > 0)
            {
                progress = (uint32_t)(obj->sock->info.send.totalbytes * 100 /
                                      obj->sock->conf.datalimitbyte);
            }

            utilunit_getdecformat(10,
                                  3,
                                  obj->sock->info.recv.totalbytes,
                                  strrecvbytes,
                                  sizeof(strrecvbytes));
            utilunit_getdecformat(10,
                                  3,
                                  obj->sock->info.send.totalbytes,
                                  strsendbytes,
                                  sizeof(strsendbytes));
            utilunit_getdecformat(10,
                                  3,
                                  obj->sock->info.recv.totalbytes * 8 * UNIT_TIME_USEC / diffusec,
                                  strrecvrate,
                                  sizeof(strrecvrate));
            utilunit_getdecformat(10,
                                  3,
                                  obj->sock->info.send.totalbytes * 8 * UNIT_TIME_USEC / diffusec,
                                  strsendrate,
                                  sizeof(strsendrate));

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
                                       "%02u:%02u:%02u:%02u.%03u\r",
                                       obj->sock->id,
                                       obj->sock->addrself.sockaddrstr,
                                       obj->sock->addrpeer.sockaddrstr,
                                       progress,
                                       progress / 10,
                                       "==========",
                                       10 - progress / 10,
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
            if (obj->timeoutusec <= obj->tsus)
            {
                obj->timeoutusec = obj->tsus + 1 * UNIT_TIME_USEC;
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

        if ((retval > 0) && (retval < obj->dstlen))
        {
            *(char*)(obj->dstbuf + retval)     = '\n';
            *(char*)(obj->dstbuf + retval + 1) = '\0';
            retval++;
        }
    }

    return retval;
}
