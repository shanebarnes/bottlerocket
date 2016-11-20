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
#include "sock_tcp.h"
#include "util_cpu.h"
#include "util_date.h"
#include "util_debug.h"
#include "util_ioctl.h"
#include "util_string.h"
#include "util_unit.h"

bool formperf_create(struct formobj * const obj, const int32_t bufsize)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(formobj_create(obj, bufsize)))
    {
        obj->ops.form_create  = formperf_create;
        obj->ops.form_destroy = formobj_destroy;
        obj->ops.form_head    = formperf_head;
        obj->ops.form_body    = formperf_body;
        obj->ops.form_foot    = formperf_foot;
        obj->spincount        = 0;

        ret = true;
    }

    return ret;
}

int32_t formperf_head(struct formobj * const obj)
{
    int32_t retval = -1;
    char recvwin[16], sendwin[16];
    //uint16_t cols    = 0,
    //         rows    = 0;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->sock != NULL) &&
                         (obj->dstbuf != NULL) &&
                         (obj->dstlen > 0)))
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
                                   "%9s %21s   %-21s %17s %27s %25s %11s %17s %3s\n",
                                   recvwin,
                                   sendwin,
                                   "Con ID",
                                   "Client",         // or self?
                                   "Server",         // or peer?
                                   "Progress",
                                   "Goodput",        // or "Bit Rate"?
                                   obj->sock->conf.model == SOCKOBJ_MODEL_CLIENT ?
                                       "Bytes Sent" : "Bytes Received",
                                   obj->sock->conf.type == SOCK_STREAM ?
                                       "Segments" : "Datagrams",
                                   "Elapsed Time",
                                   "CPU");
    }

    return retval;
}

int32_t formperf_body(struct formobj * const obj)
{
    int32_t  retval = -1;
    uint64_t diffusec = 0, packets = 0;
    uint32_t progress = 0;
    struct util_date_diff diff;
    char *client = NULL, *server = NULL;
    char strrecvbytes[16], strsendbytes[16];
    char strrecvrate[16], strsendrate[16];
    char strppi[16]; // packets per interval
    struct socktcp_info info;
    struct utilcpu_info cpu;

    // @todo redirect to udp- or tcp-specific function.
    // udp packets per second, jitter, etc.
    // tcp packets successful reads per second, jitter, etc.
    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->sock != NULL) &&
                         (obj->dstbuf != NULL) &&
                         (obj->dstlen > 0)))
    {
        if ((obj->tsus >= obj->timeoutusec) &&
            (obj->tsus > obj->sock->info.startusec))
        {
            diffusec = utildate_gettsdiff(obj->sock->info.startusec,
                                          obj->tsus,
                                          UNIT_TIME_USEC,
                                          &diff);

            if (obj->sock->conf.type == SOCK_DGRAM)
            {
                packets = (obj->sock->conf.model == SOCKOBJ_MODEL_CLIENT ?
                              obj->sock->info.send.passedcalls :
                              obj->sock->info.recv.passedcalls);
            }
            else
            {
                if ((obj->sock->state & SOCKOBJ_STATE_OPEN) &&
                    (socktcp_getinfo(obj->sock->fd, &info) == true))
                {
                    packets = (obj->sock->conf.model == SOCKOBJ_MODEL_CLIENT ?
                                  info.txpackets :
                                  info.rxpackets);
                }
            }

            utilunit_getdecformat(10,
                                  3,
                                  packets,
                                  strppi,
                                  sizeof(strppi));

            // This could be done in sock_obj during stats collection but that
            // would happen after every socket call.
            //if ((obj->sock->conf.type == SOCK_STREAM) &&
            //    (obj->sock->state & SOCKOBJ_STATE_OPEN))
            //{
            //    if (socktcp_getinfo(obj->sock->fd, &info) == true)
            //    {
            //        sendrttms = info.rttcur;
            //    }
            //}
            //else if (obj->sock->conf.type == SOCK_DGRAM)
            //{
            //}

            if (obj->sock->conf.model == SOCKOBJ_MODEL_CLIENT)
            {
                client = obj->sock->addrself.sockaddrstr;
                server = obj->sock->addrpeer.sockaddrstr;
            }
            else
            {
                client = obj->sock->addrpeer.sockaddrstr;
                server = obj->sock->addrself.sockaddrstr;
            }

            if (obj->sock->conf.timelimitusec > 0)
            {
                progress = (uint32_t)(diffusec * 100 /
                                      obj->sock->conf.timelimitusec);
            }
            else if (obj->sock->conf.datalimitbyte > 0)
            {
                if (obj->sock->conf.model == SOCKOBJ_MODEL_CLIENT)
                {
                    progress = (uint32_t)(obj->sock->info.send.totalbytes * 100 /
                                          obj->sock->conf.datalimitbyte);
                }
                else
                {
                    progress = (uint32_t)(obj->sock->info.recv.totalbytes * 100 /
                                          obj->sock->conf.datalimitbyte);
                }
            }
            else
            {
                progress = (uint32_t)(diff.sec % 20);

                if (progress > 10)
                {
                    progress = 20 - progress;
                }

                progress *= 10;
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
#if defined(__linux__)
            cpu.realtime.tv_sec = (uint32_t)(diffusec / UNIT_TIME_USEC);
            cpu.realtime.tv_usec = (uint32_t)(diffusec - cpu.realtime.tv_sec * UNIT_TIME_USEC);
#endif
            utilcpu_getinfo(&cpu);

            retval = utilstring_concat(obj->dstbuf,
                                       obj->dstlen,
                                       "[%2u:%-4u] "
                                       "%21s > %-21s "
                                       "%3u%% "
                                       "[%.*s%.*s] "
                                       "%9sbps / "
                                       "%9sbps | "
                                       "%9sB / "
                                       "%9sB | "
                                       "%9s | "
                                       "%02u:%02u:%02u:%02u.%03u "
                                       "%3u\r",
                                       obj->sock->tid,
                                       obj->sock->sid,
                                       client,
                                       server,
                                       progress,
                                       progress / 10,
                                       "==========",
                                       10 - progress / 10,
                                       "          ",
                                       strrecvrate,
                                       strsendrate,
                                       strrecvbytes,
                                       strsendbytes,
                                       strppi,
                                       diff.day + (diff.week * 7),
                                       diff.hour,
                                       diff.min,
                                       diff.sec,
                                       diff.msec,
                                       cpu.usage);

            obj->timeoutusec += obj->intervalusec;

            // Correct timeout in the event that the new timeout has already
            // expired.
            if (obj->timeoutusec <= obj->tsus)
            {
                obj->timeoutusec = obj->tsus + obj->intervalusec;
            }
        }
        else
        {
            retval = 0;
        }
    }

    return retval;
}

int32_t formperf_foot(struct formobj * const obj)
{
    int32_t retval = -1;

    if (UTILDEBUG_VERIFY((obj != NULL) &&
                         (obj->sock != NULL) &&
                         (obj->dstbuf != NULL) &&
                         (obj->dstlen > 0)))
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
