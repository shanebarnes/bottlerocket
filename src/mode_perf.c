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
#include "sock_tcp.h"
#include "thread_obj.h"
#include "util_date.h"
#include "util_string.h"

#include <signal.h>
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
    char    recvbuf[65536], sendbuf[131072];
    int32_t count = 0, timeoutms = 500;
    int32_t formbytes = 0, recvbytes = 0, sendbytes = 0;

    //??
    uint64_t timeusec = 0;

    memcpy(client.conf.ipaddr, opts->ipaddr, sizeof(client.conf.ipaddr));
    client.conf.ipport = opts->ipport;
    client.conf.timeoutms = 0;
    client.conf.type = SOCK_STREAM;
    client.conf.model = SOCKOBJ_MODEL_CLIENT;
    memcpy(server.conf.ipaddr, opts->ipaddr, sizeof(server.conf.ipaddr));
    server.conf.ipport = opts->ipport;
    server.conf.timeoutms = timeoutms;
    server.conf.type = SOCK_STREAM;
    server.conf.model = SOCKOBJ_MODEL_SERVER;

    if (thread == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        form.ops.form_head = formperf_head;
        form.ops.form_body = formperf_body;
        form.ops.form_foot = formperf_foot;
        form.srcbuf = recvbuf;
        form.srclen = sizeof(recvbuf);
        form.dstbuf = sendbuf;
        form.dstlen = sizeof(sendbuf);

        if (opts->arch == SOCKOBJ_MODEL_CLIENT)
        {
            exit = !sockmod_init(&client);
        }
        else
        {
            exit = !sockmod_init(&server);
        }

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
                    // @todo use rate limiting input interface to send data.
                    sendbytes = client.ops.sock_send(&client,
                                                     sendbuf,
                                                     sizeof(sendbuf));

                    ///?? Make sure time is only getting retrieved once per loop
                    timeusec = utildate_gettstime(DATE_CLOCK_MONOTONIC,
                                                  UNIT_TIME_USEC);
                    if ((timeusec - client.info.startusec) > opts->maxtimeusec)
                    {
                        exit = true;
                    }

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
                                                     sizeof(recvbuf));
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
    }

    // Signal main thread to shutdown.
    raise(SIGTERM);

    return NULL;
}

/**
 * @see See header file for interface comments.
 */
bool modeperf_create(struct args_obj * const args)
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
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool modeperf_start(void)
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
bool modeperf_stop(void)
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
