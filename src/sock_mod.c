/**
 * @file      sock_mod.c
 * @brief     Socket model implementation.
 * @author    Shane Barnes
 * @date      03 May 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "sock_mod.h"
#include "sock_tcp.h"
#include "sock_udp.h"

#include <string.h>

/**
 * @brief Initialize a socket object as a socket client.
 *
 * @param[in,out] sock A pointer to a socket object to initialize as a client.
 *
 * @return True on success.
 */
static bool sockmod_initclient(struct sockobj * const sock)
{
    bool retval = false;

    // Fix create function so that config structure is not reset.
    struct sockobj_conf conf;
    memcpy(&conf, &sock->conf, sizeof(conf));

    if ((sock->conf.type == SOCK_STREAM) && (socktcp_create(sock) == false))
    {
    }
    else if ((sock->conf.type == SOCK_DGRAM) &&
             (sockudp_create(sock) == false))
    {
    }
    else if (memcpy(&sock->conf, &conf, sizeof(sock->conf)) == NULL)
    {
        sock->ops.sock_destroy(sock);
    }
    else if (sock->ops.sock_open(sock) == false)
    {
        sock->ops.sock_destroy(sock);
    }
    //else if (sock->ops.sock_bind(sock) == false)
    //{
    //    sock->ops.sock_close(sock);
    //}
    else
    {
        sock->ops.sock_connect(sock);
        sock->event.timeoutms = conf.timeoutms;
        retval = true;
    }

    return retval;
}

/**
 * @brief Initialize a socket object as a socket server.
 *
 * @param[in,out] sock A pointer to a socket object to initialize as a server.
 *
 * @return True on success.
 */
static bool sockmod_initserver(struct sockobj * const sock)
{
    bool retval = false;

    // Fix create function so that config structure is not reset.
    struct sockobj_conf conf;
    memcpy(&conf, &sock->conf, sizeof(conf));

    if ((sock->conf.type == SOCK_STREAM) && (socktcp_create(sock) == false))
    {
    }
    else if ((sock->conf.type == SOCK_DGRAM) &&
             (sockudp_create(sock) == false))
    {
    }
    else if (memcpy(&sock->conf, &conf, sizeof(sock->conf)) == NULL)
    {
        sock->ops.sock_destroy(sock);
    }
    else if (sock->ops.sock_open(sock) == false)
    {
        sock->ops.sock_destroy(sock);
    }
    else if (sock->ops.sock_bind(sock) == false)
    {
        sock->ops.sock_close(sock);
    }
    else if (sock->ops.sock_listen(sock, 1) == false)
    {
        sock->ops.sock_close(sock);
    }
    else
    {
        sock->event.timeoutms = conf.timeoutms;
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool sockmod_init(struct sockobj * const sock)
{
    bool retval = false;

    if (sock == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        switch (sock->conf.model)
        {
            case SOCKOBJ_MODEL_CLIENT:
                retval = sockmod_initclient(sock);
                break;
            case SOCKOBJ_MODEL_SERVER:
                retval = sockmod_initserver(sock);
                break;
            default:
                logger_printf(LOGGER_LEVEL_ERROR,
                              "%s: unsupported socket model %d\n",
                              __FUNCTION__,
                              sock->conf.model);
                break;
        }
    }

    return retval;
}
