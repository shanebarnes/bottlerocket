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

struct modeobj_priv
{
    struct args_obj   args;
    struct threadpool threadpool;
};

bool modechat_create(struct modeobj * const mode,
                     const struct args_obj * const args)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) &&
                         (mode->priv == NULL) &&
                         (args != NULL)))
    {
        if ((mode->priv = UTILMEM_CALLOC(struct modeobj_priv,
                                         sizeof(struct modeobj_priv),
                                         1)) == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate memory\n",
                          __FUNCTION__);
        }
        else if (memcpy(&mode->priv->args,
                        args,
                        sizeof(mode->priv->args)) == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to copy arguments\n",
                          __FUNCTION__);
        }
        else if (!threadpool_create(&mode->priv->threadpool, args->threads))
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to create thread pool\n",
                          __FUNCTION__);
            UTILMEM_FREE(mode->priv);
            mode->priv = NULL;
        }
        else
        {
            mode->ops.mode_create  = modechat_create;
            mode->ops.mode_destroy = modechat_destroy;
            mode->ops.mode_start   = modechat_start;
            mode->ops.mode_stop    = modechat_stop;
            mode->ops.mode_cancel  = modechat_cancel;

            ret = true;
        }
    }

    return ret;
}

bool modechat_destroy(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        mode->ops.mode_stop(mode);
        threadpool_destroy(&mode->priv->threadpool);
        UTILMEM_FREE(mode->priv);
        mode->priv = NULL;
        memset(&mode->ops, 0, sizeof(mode->ops));

        ret = true;
    }

    return ret;
}

/**
 * @brief Copy a mode object configuration to a socket object configuration.
 *
 * @param[in,out] sock      A pointer to a socket object.
 * @param[in]     mode      A pointer to a mode object.
 * @param[in]     timeoutms Socket timeout in milliseconds.
 *
 * @return True on a successful copy.
 */
static bool modechat_copy(struct sockobj * const sock,
                          struct modeobj_priv * const mode,
                          const int32_t timeoutms)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((sock != NULL) && (mode != NULL)))
    {
        memcpy(sock->conf.ipaddr, mode->args.ipaddr, sizeof(sock->conf.ipaddr));
        sock->conf.ipport        = mode->args.ipport;
        sock->conf.backlog       = mode->args.backlog;
        sock->conf.timeoutms     = timeoutms;
        sock->conf.datalimitbyte = mode->args.datalimitbyte;
        sock->conf.ratelimitbps  = mode->args.ratelimitbps;
        sock->conf.timelimitusec = mode->args.timelimitusec;
        sock->conf.family        = mode->args.family;
        sock->conf.type          = mode->args.type;
        sock->conf.model         = mode->args.arch;

        ret = true;
    }

    return ret;
}

static void *modechat_workerthread(void * const arg)
{
    bool exit = false;
    struct modeobj_priv *mode = (struct modeobj_priv*)arg;
    struct sockobj server, socket;
    struct formobj form;
    struct fionobj fion;
    int32_t count = 0, timeoutms = 500;
    int32_t recvbytes = 0, formbytes = 0;

    modechat_copy(&socket, mode, 0);
    modechat_copy(&server, mode, 0);

    //memcpy(socket.conf.ipaddr, mode->args.ipaddr, sizeof(socket.conf.ipaddr));
    //socket.conf.ipport = mode->args.ipport;
    //socket.conf.timeoutms = 0;
    //socket.conf.timelimitusec = mode->args.timelimitusec;
    //socket.conf.datalimitbyte = mode->args.datalimitbyte;
    //socket.conf.family = opts->family;
    //socket.conf.type = opts->type;
    //socket.conf.model = SOCKOBJ_MODEL_CLIENT;
    //memcpy(server.conf.ipaddr, opts->ipaddr, sizeof(server.conf.ipaddr));
    //server.conf.ipport = opts->ipport;
    //server.conf.timeoutms = 0;
    //server.conf.family = opts->family;
    //server.conf.type = opts->type;
    //server.conf.model = SOCKOBJ_MODEL_SERVER;
    memset(&form, 0, sizeof(form));

    if (!formchat_create(&form, mode->args.buflen))
    {
        // Do nothing.
    }
    else if (!fionpoll_create(&fion))
    {
        form.ops.form_destroy(&form);
    }
    else
    {
        fion.ops.fion_insertfd(&fion, STDIN_FILENO);
        fion.timeoutms = timeoutms;
        fion.pevents   = FIONOBJ_PEVENT_IN;
        fion.ops.fion_setflags(&fion); // ?? fix this

        if (mode->args.arch == SOCKOBJ_MODEL_CLIENT)
        {
            exit = !sockmod_init(&socket);
        }
        else
        {
            exit = !sockmod_init(&server);
        }

        while ((!exit) && (threadpool_isrunning(&mode->threadpool)))
        {
            fion.ops.fion_poll(&fion);

            if (count <= 0)
            {
                if (mode->args.arch == SOCKOBJ_MODEL_CLIENT)
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
                    inputstd_recv(form.srcbuf, mode->args.buflen, 0);
                }
            }
            else
            {
                if (mode->args.arch == SOCKOBJ_MODEL_CLIENT)
                {
                    if ((socket.state & SOCKOBJ_STATE_CONNECT) == 0)
                    {
                        socket.ops.sock_connect(&socket);
                    }
                }

                recvbytes = socket.ops.sock_recv(&socket,
                                                 form.srcbuf,
                                                 mode->args.buflen - 1);

                if (recvbytes > 0)
                {
                    if ((mode->args.arch == SOCKOBJ_MODEL_SERVER) &&
                        (mode->args.echo))
                    {
                        // @todo Fix for partial-send case.
                        socket.ops.sock_send(&socket, form.srcbuf, recvbytes);
                    }

                    // Null-terminate the string.
                    ((char*)form.srcbuf)[recvbytes] = '\0';
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

                    if (mode->args.arch == SOCKOBJ_MODEL_CLIENT)
                    {
                        exit = true;
                    }
                }

                if (fion.ops.fion_getevents(&fion, 0) & FIONOBJ_REVENT_INREADY)
                {
                    if ((recvbytes = inputstd_recv(form.srcbuf,
                                                   mode->args.buflen,
                                                   0)) > 0)
                    {
                        if (count > 0)
                        {
                            // @todo Fix for partial-send case.
                            socket.ops.sock_send(&socket,
                                                 form.srcbuf,
                                                 recvbytes);
                        }
                    }
                }
            }
        }

        fionpoll_destroy(&fion);
        form.ops.form_destroy(&form);
    }

    return NULL;
}

bool modechat_start(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        threadpool_stop(&mode->priv->threadpool);
        ret  = threadpool_start(&mode->priv->threadpool);
        ret &= threadpool_execute(&mode->priv->threadpool,
                                  modechat_workerthread,
                                  mode->priv,
                                  0);

        threadpool_wait(&mode->priv->threadpool, 1/*mode->priv->args.threads*/);
    }

    return ret;
}

bool modechat_stop(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        ret  = mode->ops.mode_cancel(mode);
        ret &= threadpool_stop(&mode->priv->threadpool);
    }

    return ret;
}

bool modechat_cancel(struct modeobj * const mode)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((mode != NULL) && (mode->priv != NULL)))
    {
        ret = threadpool_wake(&mode->priv->threadpool);
    }

    return ret;
}
