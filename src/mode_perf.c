/**
 * @file      mode_perf.c
 * @brief     Performance mode implementation.
 * @author    Shane Barnes
 * @date      28 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "form_perf.h"
#include "logger.h"
#include "mode_perf.h"
#include "output_if_std.h"
#include "sock_mod.h"
#include "thread_obj.h"
#include "token_bucket.h"
#include "util_date.h"
#include "util_string.h"

#include <string.h>

static struct threadobj thread;
static struct args_obj *opts = NULL;

/**
 * @see See header file for interface comments.
 */
static void *modeperf_thread(void * arg)
{
    bool exit = false;
    struct threadobj *thread = (struct threadobj *)arg;
    struct sockobj client, server, socket;
    struct formobj form;
    struct tokenbucket tb;
    char *recvbuf = NULL, *sendbuf = NULL;
    int32_t count = 0, timeoutms = 500;
    int32_t formbytes = 0, recvbytes = 0, sendbytes = 0;

    //??
    uint64_t timeusec = 0;
    uint64_t buflen = 0;

    memcpy(client.conf.ipaddr, opts->ipaddr, sizeof(client.conf.ipaddr));
    client.conf.ipport = opts->ipport;
    client.conf.timeoutms = 0;
    client.conf.timelimitusec = opts->timelimitusec;
    client.conf.datalimitbyte = opts->datalimitbyte;
    client.conf.family = opts->family;
    client.conf.type = opts->type;
    client.conf.model = SOCKOBJ_MODEL_CLIENT;
    memcpy(server.conf.ipaddr, opts->ipaddr, sizeof(server.conf.ipaddr));
    server.conf.ipport = opts->ipport;
    server.conf.timeoutms = timeoutms;
    server.conf.family = opts->family;
    server.conf.type = opts->type;
    server.conf.model = SOCKOBJ_MODEL_SERVER;

    if (thread == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if ((recvbuf = malloc(opts->buflen)) == NULL)
    {

    }
    else if ((sendbuf = malloc(opts->buflen)) == NULL)
    {
        free(recvbuf);
        recvbuf = NULL;
    }
    else
    {
        form.ops.form_head = formperf_head;
        form.ops.form_body = formperf_body;
        form.ops.form_foot = formperf_foot;
        form.srcbuf = recvbuf;
        form.srclen = opts->buflen;
        form.dstbuf = sendbuf;
        form.dstlen = opts->buflen;

        if (opts->arch == SOCKOBJ_MODEL_CLIENT)
        {
            exit = !sockmod_init(&client);
        }
        else
        {
            exit = !sockmod_init(&server);
        }

        tokenbucket_init(&tb, opts->ratelimitbps);//(opts->ratelimitbps - 1) / 8 + 1); // Round up (x + (y-1)) / y or (x-1)/y + 1

        while ((exit == false) && (threadobj_isrunning(thread) == true))
        {
            if (count <= 0)
            {
                if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                {
                    form.sock = &client;
                    count++;
                    //formbytes = formobj_idle(&form);
                    //output_if_std_send(form.dstbuf, formbytes);
                    //formbytes = utilstring_concat(form.dstbuf,
                    //                              form.dstlen,
                    //                              "%c",
                    //                              '\r');
                    //output_if_std_send(form.dstbuf, formbytes);
                }
                else
                {
                    if (server.ops.sock_accept(&server, &socket) == true)
                    {
                        logger_printf(LOGGER_LEVEL_DEBUG,
                                      "%s: server accepted connection on %s\n",
                                      __FUNCTION__,
                                      server.addrself.sockaddrstr);
                        socket.event.timeoutms = timeoutms;
                        form.sock = &socket;
                        formbytes = form.ops.form_head(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        count++;
                    }
                    else
                    {
                        form.sock = &server;
                        formbytes = formobj_idle(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        formbytes = utilstring_concat(form.dstbuf,
                                                      form.dstlen,
                                                      "%c",
                                                      '\r');
                        output_if_std_send(form.dstbuf, formbytes);
                    }

                    // @todo Exit if server socket has fatal error.
                }
            }
            else
            {
                if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                {
                    if ((client.state & SOCKOBJ_STATE_CONNECT) == 0)
                    {
                        client.ops.sock_connect(&client);
                    }

                    if (opts->datalimitbyte > 0)
                    {
                        if (client.info.send.totalbytes < opts->datalimitbyte)
                        {
                            buflen = opts->datalimitbyte - client.info.send.totalbytes;

                            if (buflen > opts->buflen)
                            {
                                buflen = opts->buflen;
                            }

                            buflen = tokenbucket_remove(&tb, buflen * 8) / 8;
                        }
                        else
                        {
                            buflen = 0;
                        }
                    }
                    else if (opts->timelimitusec > 0)
                    {
                        buflen = tokenbucket_remove(&tb, opts->buflen * 8) / 8;
                    }

                    // @todo use rate limiting input interface to send data.
                    if (buflen > 0)
                    {
                        sendbytes = client.ops.sock_send(&client,
                                                         sendbuf,
                                                         buflen);
                    }
                    else
                    {
                        sendbytes = 0;
                    }

                    ///?? Make sure time is only getting retrieved once per loop
                    timeusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                  UNIT_TIME_USEC);

                    if ((opts->timelimitusec > 0) &&
                        ((timeusec - client.info.startusec) >= opts->timelimitusec))
                    {
                        formbytes = form.ops.form_foot(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        client.ops.sock_close(&client);
                        exit = true;

                        logger_printf(LOGGER_LEVEL_INFO,
                                      "%s: passed %" PRIu64
                                      ", failed %" PRIu64
                                      ", avg:%u max:%u min:%u\n",
                                      __FUNCTION__,
                                      client.info.send.passedcalls,
                                      client.info.send.failedcalls,
                                      client.info.send.avgbuflen,
                                      client.info.send.maxbuflen,
                                      client.info.send.minbuflen);
                    }
                    else if ((opts->datalimitbyte > 0) &&
                             (client.info.send.totalbytes >= opts->datalimitbyte))
                    {
                        formbytes = form.ops.form_foot(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        client.ops.sock_close(&client);
                        exit = true;
                    }

                    tokenbucket_return(&tb, sendbytes < 0 ?
                                            0 :
                                            buflen - (uint32_t)sendbytes);

                    if (sendbytes < 0)
                    {
                        formbytes = form.ops.form_foot(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        client.ops.sock_close(&client);
                        exit = true;
                    }
                    else
                    {
                        formbytes = form.ops.form_body(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                    }
                }
                else
                {
                    recvbytes = socket.ops.sock_recv(&socket,
                                                     recvbuf,
                                                     opts->buflen);
                    if (recvbytes < 0)
                    {
                        formbytes = form.ops.form_foot(&form);
                        output_if_std_send(form.dstbuf, formbytes);

                        socket.ops.sock_close(&socket);
                        count--;
                    }
                    else
                    {
                        formbytes = form.ops.form_body(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                    }
                }
            }
        }

        free(recvbuf);
        free(sendbuf);
    }

    return NULL;
}

/**
 * @see See header file for interface comments.
 */
static bool modeperf_init(struct args_obj * const args)
{
    bool retval = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        opts = args;
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
static bool modeperf_start(void)
{
    bool retval = false;

    thread.function = modeperf_thread;
    thread.argument = &thread;

    if (opts == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (threadobj_create(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to create chat thread\n",
                      __FUNCTION__);
    }
    else if (threadobj_start(&thread) == false)
    {
        threadobj_destroy(&thread);
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to start chat thread\n",
                      __FUNCTION__);
    }
    else
    {
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
static bool modeperf_stop(void)
{
    bool retval = false;

    if (threadobj_stop(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to stop chat thread\n",
                      __FUNCTION__);
    }
    else if (threadobj_destroy(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to destroy chat thread\n",
                      __FUNCTION__);
    }
    else
    {
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool modeperf_run(struct args_obj * const args)
{
    bool retval = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (modeperf_init(args) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to initialize mode\n",
                          __FUNCTION__);

        }
        else if (modeperf_start() == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to start mode\n",
                          __FUNCTION__);

        }
        else if (threadobj_join(&thread) == false)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to suspend caller\n",
                          __FUNCTION__);
        }
        else
        {
            modeperf_stop();
            retval = true;
        }
    }

    return retval;
}
