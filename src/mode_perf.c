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
#include "sock_tcp.h"
#include "thread_obj.h"
#include "util_string.h"

#include <string.h>

static struct threadobj thread;
static struct args_obj *opts = NULL;

/**
 * @see See header file for interface comments.
 */
static void *modeperf_thread(void * arg)
{
    struct threadobj *thread = (struct threadobj *)arg;
    struct sockobj server, socket;
    struct formobj form;
    char    recvbuf[65536], sendbuf[65536];
    int32_t count = 0, timeoutms = 500;
    int32_t recvbytes = 0, formbytes = 0;

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

        socktcp_create(&server);
        memcpy(server.ipaddr, opts->ipaddr, sizeof(server.ipaddr));
        server.ipport = opts->ipport;

        if ((server.ops.sock_open(&server) == false) ||
            (server.ops.sock_bind(&server) == false) ||
            (server.ops.sock_listen(&server, 1) == false))
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to listen on %s:%u\n",
                          __FUNCTION__,
                          server.ipaddr,
                          server.ipport);
        }
        else
        {
            server.event.timeoutms = timeoutms;
        }

        while (threadobj_isrunning(thread) == true)
        {
            if (count <= 0)
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
            else
            {
                recvbytes = socket.ops.sock_recv(&socket,
                                                 recvbuf,
                                                 sizeof(recvbuf) - 1);

                // @todo use rate limiting input interface to send data.

                if (recvbytes > 0)
                {
                    form.srclen = recvbytes;
                }
                else if (recvbytes < 0)
                {
                    socket.ops.sock_close(&socket);
                    count--;
                    formbytes = form.ops.form_foot(&form);
                    output_if_std_send(form.dstbuf, formbytes);
                }

                formbytes = form.ops.form_body(&form);
                output_if_std_send(form.dstbuf, formbytes);
            }
        }
    }

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
