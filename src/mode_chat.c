/**
 * @file      mode_chat.c
 * @brief     Chat mode implementation.
 * @author    Shane Barnes
 * @date      02 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "input_if_std.h"
#include "logger.h"
#include "mode_chat.h"
#include "output_if_std.h"
#include "sock_tcp.h"
#include "thread_instance.h"
#include "util_ioctl.h"
#include "util_string.h"

#include <string.h>

static struct thread_instance thread;

/**
 * @see See header file for interace comments.
 */
static void *modechat_thread(void * arg)
{
    struct thread_instance *thread = (struct thread_instance *)arg;
    struct sockobj server, socket;
    char     recvbuf[1024], sendbuf[4096];
    uint16_t cols = 0, rows = 0;
    int32_t  lmargin = 0, rmargin = 0;
    int32_t  count = 0, timeoutms = 0;
    int32_t  recvbytes = 0, sendbytes = 0, tempbytes = 0;

    if (thread == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        socktcp_create(&server);
        server.ipaddr = "127.0.0.1";
        server.ipport = 5001;

        if ((server.ops.soo_open(&server) == false) ||
            (server.ops.soo_bind(&server) == false) ||
            (server.ops.soo_listen(&server, 1) == false))
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
                if (server.ops.soo_accept(&server, &socket) == true)
                {
                    logger_printf(LOGGER_LEVEL_DEBUG,
                                  "%s: server accepted connection on %s\n",
                                  __FUNCTION__,
                                  server.addrself.sockaddrstr);
                    socket.event.timeoutms = timeoutms;
                    count++;
                }

                // @todo Exit if server socket has fatal error.
            }
            else
            {
                recvbytes = socket.ops.soo_recv(&socket,
                                                recvbuf,
                                                sizeof(recvbuf) - 1);

                if (recvbytes > 0)
                {
                    logger_printf(LOGGER_LEVEL_ERROR,
                                  "%s: bytes received = %d\n",
                                  __FUNCTION__,
                                  recvbytes);

                    // Null-terminate the string.
                    recvbuf[recvbytes] = '\0';
                    recvbytes++;

                    utilioctl_gettermsize(&rows, &cols);
                    rmargin = cols;
                    lmargin = cols / 2;

                    sendbytes = 0;

                    while (sendbytes < recvbytes)
                    {
                        if ((recvbytes - sendbytes) < (rmargin - lmargin))
                        {
                            rmargin = lmargin + (recvbytes - sendbytes);
                        }

                        // Set the left margin and create a substring that is
                        // left justified.
                        tempbytes = utilstring_concat(sendbuf,
                                                      sizeof(sendbuf),
                                                      "%*s%-*.*s\n",
                                                      lmargin,
                                                      "",
                                                      rmargin - lmargin,
                                                      rmargin - lmargin,
                                                      recvbuf + sendbytes);

                        tempbytes = output_if_std_send(sendbuf, tempbytes);

                        if (tempbytes > 0)
                        {
                            sendbytes += (rmargin - lmargin);
                        }
                    }
                }
                else if (recvbytes < 0)
                {
                    socket.ops.soo_close(&socket);
                    count--;
                }

                if (input_if_std_recv(recvbuf, sizeof(recvbuf), timeoutms) > 0)
                {
                    //pos = (pos < 0 ? cols / 4 : -cols / 4);
                    //logger_printf(LOGGER_LEVEL_ERROR,
                    //              "%s: read from input (%u, %u, %d): '%*s'\n",
                    //              __FUNCTION__,
                    //              rows,
                    //              cols,
                    //              pos,
                    //              pos,
                    //              buf);
                }
            }
        }
    }

    return NULL;
}

/**
 * @see See header file for interace comments.
 */
bool modechat_create(const struct args_obj * const args)
{
    bool retval = false;

    if (args == NULL)
    {

    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool modechat_start(void)
{
    bool retval = false;

    thread.function = modechat_thread;
    thread.argument = &thread;

    if (thread_instance_create(&thread) == false)
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
 * @see See header file for interace comments.
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
