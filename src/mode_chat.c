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
#include "util_string.h"

#include <signal.h>
#include <string.h>
#include <unistd.h>

static struct threadpool pool;
static struct args_obj *opts = NULL;

/**
 * @see See header file for interface comments.
 */
static void *modechat_thread(void * const arg)
{
    bool exit = false;
    struct threadpool *pool = (struct threadpool*)arg;
    struct sockobj server, socket;
    struct formobj form;
    struct fionobj fion;
    char recvbuf[65536], sendbuf[65536];
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

    if (UTILDEBUG_VERIFY(pool != NULL) == false)
    {
        // Do nothing.
    }
    else if (fionpoll_create(&fion) == false)
    {
        // Do nothing.
    }
    else
    {
        fion.ops.fion_insertfd(&fion, STDIN_FILENO);
        fion.timeoutms = timeoutms;
        fion.pevents   = FIONOBJ_PEVENT_IN;
        fion.ops.fion_setflags(&fion); // ?? fix this

        formchat_init(&form);
        form.srcbuf = recvbuf;
        form.srclen = sizeof(recvbuf);
        form.dstbuf = sendbuf;
        form.dstlen = sizeof(sendbuf);

        if (opts->arch == SOCKOBJ_MODEL_CLIENT)
        {
            exit = !sockmod_init(&socket);
        }
        else
        {
            exit = !sockmod_init(&server);
        }

        while ((exit == false) && (threadpool_isrunning(pool) == true))
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
                    if (server.ops.sock_accept(&server, &socket) == true)
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
                    inputstd_recv(recvbuf, sizeof(recvbuf), 0);
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
                                                 sizeof(recvbuf) - 1);

                if (recvbytes > 0)
                {
                    if ((opts->arch == SOCKOBJ_MODEL_SERVER) &&
                        (opts->echo == true))
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
                                                   sizeof(recvbuf),
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
    }

    return NULL;
}

/**
 * @brief Run the chat mode tasks.
 *
 * @param[in,out] arg A pointer to a thread pool.
 */
static void *modechat_run(void *arg)
{
    struct threadpool *pool = (struct threadpool*)arg;

    if (UTILDEBUG_VERIFY(pool != NULL) == true)
    {
        threadpool_execute(pool, modechat_thread, pool);
        threadpool_wait(pool, 1);
    }

    raise(SIGTERM);

    return NULL;
}

/**
 * @see See header file for interface comments.
 */
bool modechat_init(struct args_obj * const args)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(args != NULL) == true)
    {
        opts = args;
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool modechat_start(void)
{
    bool retval = false;

    if (UTILDEBUG_VERIFY(opts != NULL) == false)
    {
        // Do nothing.
    }
    else if (threadpool_create(&pool, 2) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to create chat threads\n",
                      __FUNCTION__);
    }
    else if (threadpool_start(&pool) == false)
    {
        threadpool_destroy(&pool);
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to start chat threads\n",
                      __FUNCTION__);
    }
    else
    {
        retval = threadpool_execute(&pool, modechat_run, &pool);
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool modechat_stop(void)
{
    bool retval = false;

    if (threadpool_stop(&pool) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to stop chat threads\n",
                      __FUNCTION__);
    }
    else if (threadpool_destroy(&pool) == false)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: failed to destroy chat threads\n",
                      __FUNCTION__);
    }
    else
    {
        retval = true;
    }

    return retval;
}
