/**
 * @file      mode_chat.c
 * @brief     Chat mode implementation.
 * @author    Shane Barnes
 * @date      02 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "form_chat.h"
#include "input_if_std.h"
#include "logger.h"
#include "mode_chat.h"
#include "output_if_std.h"
#include "sock_tcp.h"
#include "thread_instance.h"

#include <string.h>

static struct thread_instance thread;
static struct args_obj *opts = NULL;

/**
 * @see See header file for interface comments.
 */
static void *modechat_thread(void * arg)
{
    struct thread_instance *thread = (struct thread_instance *)arg;
    struct sockobj server, socket;
    struct formobj form;
    char    recvbuf[1024], sendbuf[4096];
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
        form.ops.form_head = formchat_head;
        form.ops.form_body = formchat_body;
        form.ops.form_foot = formchat_foot;
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

        while (thread_instance_isrunning(thread) == true)
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

                // @todo Exit if server socket has fatal error.
            }
            else
            {
                recvbytes = socket.ops.sock_recv(&socket,
                                                 recvbuf,
                                                 sizeof(recvbuf) - 1);

                if (recvbytes > 0)
                {
                    // Null-terminate the string.
                    recvbuf[recvbytes] = '\0';
                    recvbytes++;

                    form.srclen = recvbytes;
                    formbytes = form.ops.form_body(&form);
                    output_if_std_send(form.dstbuf, formbytes);
                }
                else if (recvbytes < 0)
                {
                    socket.ops.sock_close(&socket);
                    count--;
                    formbytes = form.ops.form_foot(&form);
                    output_if_std_send(form.dstbuf, formbytes);
                }

                if ((recvbytes = input_if_std_recv(recvbuf,
                                                   sizeof(recvbuf),
                                                   timeoutms)) > 0)
                {
                    if (count > 0)
                    {
                        // @todo Fix for partial-send case.
                        socket.ops.sock_send(&socket, recvbuf, recvbytes);
                    }
                }
            }
        }
    }

    return NULL;
}

/**
 * @see See header file for interface comments.
 */
bool modechat_create(struct args_obj * const args)
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
bool modechat_start(void)
{
    bool retval = false;

    thread.function = modechat_thread;
    thread.argument = &thread;

    if (opts == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (thread_instance_create(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to create chat thread\n",
                      __FUNCTION__);
    }
    else if (thread_instance_start(&thread) == false)
    {
        thread_instance_destroy(&thread);
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
bool modechat_stop(void)
{
    bool retval = false;

    if (thread_instance_stop(&thread) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to stop chat thread\n",
                      __FUNCTION__);
    }
    else if (thread_instance_destroy(&thread) == false)
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
