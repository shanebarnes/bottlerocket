/**
 * @file      mode_chat.c
 * @brief     Chat mode implementation.
 * @author    Shane Barnes
 * @date      02 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "fion_poll.h"
#include "form_chat.h"
#include "input_std.h"
#include "logger.h"
#include "mode_chat.h"
#include "output_if_std.h"
#include "sock_mod.h"
#include "sock_tcp.h"
#include "sock_udp.h"
#include "thread_pool.h"
#include "util_debug.h"
#include "util_mem.h"
#include "util_string.h"

#include <string.h>
#include <unistd.h>

static struct threadpool pool;
static struct args_obj *opts = NULL;

static void *modechat_thread(void * const arg)
{
    bool exit = false;
    struct threadpool *tpool = (struct threadpool*)arg;
    struct sockobj server, socket;
    struct formobj form;
    struct fionobj fion;
    char *recvbuf = NULL, *sendbuf = NULL;
    int32_t count = 0, timeoutms = 500;
    int32_t recvbytes = 0, formbytes = 0;

    memcpy(socket.conf.ipaddr, opts->ipaddr, sizeof(socket.conf.ipaddr));
    socket.conf.ipport = opts->ipport;
    socket.conf.timeoutms = 0;
    socket.conf.timelimitusec = opts->timelimitusec;
    socket.conf.datalimitbyte = opts->datalimitbyte;
    socket.conf.family = opts->family;
    socket.conf.type = opts->type;
    socket.conf.model = SOCKOBJ_MODEL_CLIENT;
    memcpy(server.conf.ipaddr, opts->ipaddr, sizeof(server.conf.ipaddr));
    server.conf.ipport = opts->ipport;
    server.conf.timeoutms = 0;
    server.conf.family = opts->family;
    server.conf.type = opts->type;
    server.conf.model = SOCKOBJ_MODEL_SERVER;

    if (!UTILDEBUG_VERIFY(tpool != NULL))
    {
        // Do nothing.
    }
    else if ((recvbuf = UTILMEM_MALLOC(char, sizeof(char), opts->buflen)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: receive buffer allocation failed\n",
                      __FUNCTION__);
    }
    else if ((sendbuf = UTILMEM_MALLOC(char, sizeof(char), opts->buflen)) == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: send buffer allocation failed\n",
                      __FUNCTION__);

        UTILMEM_FREE(recvbuf);
        recvbuf = NULL;
    }
    else if (!fionpoll_create(&fion))
    {
        UTILMEM_FREE(recvbuf);
        UTILMEM_FREE(sendbuf);
    }
    else
    {
        fion.ops.fion_insertfd(&fion, STDIN_FILENO);
        fion.timeoutms = timeoutms;
        fion.pevents   = FIONOBJ_PEVENT_IN;
        fion.ops.fion_setflags(&fion); // ?? fix this

        formchat_init(&form);
        form.srcbuf = recvbuf;
        form.srclen = opts->buflen;
        form.dstbuf = sendbuf;
        form.dstlen = opts->buflen;

        if (opts->arch == SOCKOBJ_MODEL_CLIENT)
        {
            exit = !sockmod_init(&socket);
        }
        else
        {
            exit = !sockmod_init(&server);
        }

        while ((!exit) && (threadpool_isrunning(tpool)))
        {
            fion.ops.fion_poll(&fion);

            if (count <= 0)
            {
                if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                {
                    form.sock = &socket;
                    formbytes = form.ops.form_head(&form);
                    output_if_std_send(form.dstbuf, formbytes);
                    fion.ops.fion_insertfd(&fion, socket.fd);
                    count++;
                }
                else
                {
                    if (server.ops.sock_accept(&server, &socket))
                    {
                        logger_printf(LOGGER_LEVEL_DEBUG,
                                      "%s: server accepted connection on %s\n",
                                      __FUNCTION__,
                                      server.addrself.sockaddrstr);
                        socket.event.timeoutms = 0;
                        form.sock = &socket;
                        formbytes = form.ops.form_head(&form);
                        output_if_std_send(form.dstbuf, formbytes);
                        fion.ops.fion_insertfd(&fion, socket.fd);
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

                // Flush input.
                if (fion.ops.fion_getevents(&fion, 0) & FIONOBJ_REVENT_INREADY)
                {
                    inputstd_recv(recvbuf, opts->buflen, 0);
                }
            }
            else
            {
                if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                {
                    if ((socket.state & SOCKOBJ_STATE_CONNECT) == 0)
                    {
                        socket.ops.sock_connect(&socket);
                    }
                }

                recvbytes = socket.ops.sock_recv(&socket,
                                                 recvbuf,
                                                 opts->buflen - 1);

                if (recvbytes > 0)
                {
                    if ((opts->arch == SOCKOBJ_MODEL_SERVER) && (opts->echo))
                    {
                        // @todo Fix for partial-send case.
                        socket.ops.sock_send(&socket, recvbuf, recvbytes);
                    }

                    // Null-terminate the string.
                    recvbuf[recvbytes] = '\0';
                    recvbytes++;

                    form.srclen = recvbytes;
                    formbytes = form.ops.form_body(&form);
                    output_if_std_send(form.dstbuf, formbytes);
                }
                else if (recvbytes < 0)
                {
                    fion.ops.fion_deletefd(&fion, socket.fd);
                    socket.ops.sock_close(&socket);
                    socket.ops.sock_destroy(&socket);
                    count--;
                    formbytes = form.ops.form_foot(&form);
                    output_if_std_send(form.dstbuf, formbytes);

                    if (opts->arch == SOCKOBJ_MODEL_CLIENT)
                    {
                        exit = true;
                    }
                }

                if (fion.ops.fion_getevents(&fion, 0) & FIONOBJ_REVENT_INREADY)
                {
                    if ((recvbytes = inputstd_recv(recvbuf,
                                                   opts->buflen,
                                                   0)) > 0)
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

        fionpoll_destroy(&fion);
        UTILMEM_FREE(recvbuf);
        UTILMEM_FREE(sendbuf);
    }

    return NULL;
}

bool modechat_init(struct args_obj * const args)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY(args != NULL))
    {
        opts = args;
        ret = true;
    }

    return ret;
}

bool modechat_start(void)
{
    bool ret = false;

    if (!UTILDEBUG_VERIFY(opts != NULL))
    {
        // Do nothing.
    }
    else if (!threadpool_create(&pool, opts->threads))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to create chat threads\n",
                      __FUNCTION__);
    }
    else if (!threadpool_start(&pool))
    {
        threadpool_destroy(&pool);
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to start chat threads\n",
                      __FUNCTION__);
    }
    else
    {
        ret = threadpool_execute(&pool, modechat_thread, &pool, 0);
        threadpool_wait(&pool, 1/*opts->threads*/);
    }

    return ret;
}

bool modechat_stop(void)
{
    bool ret = false;

    if (!threadpool_stop(&pool))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to stop chat threads\n",
                      __FUNCTION__);
    }
    else if (!threadpool_destroy(&pool))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to destroy chat threads\n",
                      __FUNCTION__);
    }
    else
    {
        ret = true;
    }

    return ret;
}

bool modechat_cancel(void)
{
    return threadpool_wake(&pool);
}
