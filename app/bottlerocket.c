/**
 * @file      bottlerocket.c
 * @brief     A socket benchmark utility implementation.
 * @author    Shane Barnes
 * @date      01 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "args.h"
#include "input_if_std.h"
#include "logger.h"
#include "output_if_instance.h"
#include "output_if_std.h"
#include "sock_tcp.h"
#include "sock_udp.h"
#include "system_types.h"
#include "thread_instance.h"
#include "util_ioctl.h"
#include "util_string.h"
#include "util_sysctl.h"

#include <errno.h>
#if !defined(__CYGWIN__)
    #include <execinfo.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Signal handler callback function.
 *
 * @param[in] signum The captured signal number.
 *
 * @return Void.
 */
void signal_handler(int signum)
{
#if !defined(__CYGWIN__)
    void *callstack[128];
    int32_t i, frames;
    char **strs = NULL;
#endif
    switch (signum)
    {
        case SIGHUP:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGHUP\n");
            break;
        case SIGINT:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGINT\n");
            break;
        case SIGKILL:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGKILL\n");
            break;
        case SIGQUIT:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGQUIT\n");
            break;
        case SIGSEGV:
#if !defined(__CYGWIN__)
            frames = backtrace(callstack, 128);
            strs = backtrace_symbols(callstack, frames);
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGSEGV\n");
            for (i = 0; i < frames; ++i)
            {
                logger_printf(LOGGER_LEVEL_ERROR, "%s\n", strs[i]);
            }
            free(strs);
#endif
            exit(signum);
            break;
        case SIGSTOP:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGSTOP\n");
            break;
        case SIGTERM:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGTERM\n");
            break;
        default:
            logger_printf(LOGGER_LEVEL_INFO, "Caught signal %d\n", signum);
    }

    //exit(signum);
}

/**
 * @brief TCP client thread function.
 *
 * @param[in] arg Thread input argument.
 *
 * @return NULL.
 */
void *thread_client_tcp(void * arg)
{
    struct thread_instance *instance = (struct thread_instance *)arg;
    char buf[2048];
    int32_t error, sendcount;
    struct sockobj client;

    logger_printf(LOGGER_LEVEL_DEBUG,
                  "%s: starting thread '%s'\n",
                  __FUNCTION__,
                  instance->name);

    if (instance != NULL)
    {
        memset(&client, 0, sizeof(client));
        socktcp_create(&client);
        client.ipaddr = "127.0.0.1";
        client.ipport = 5001;

        if ((client.ops.soo_open(&client) == true) &&
            ((client.event.timeoutms = 1000) > 0) &&
            /*(client.ops.soo_bind(&client) == true) &&*/
            (client.ops.soo_connect(&client) == true))
        {
            logger_printf(LOGGER_LEVEL_DEBUG,
                          "%s: connected to %s\n",
                          __FUNCTION__,
                          client.addrpeer.sockaddrstr);

            client.event.timeoutms = 1000;
            sendcount = 0;

            while ((thread_instance_isrunning(instance) == false) &&
                   (sendcount < 5))
            {
                error = client.ops.soo_send(&client, buf, sizeof(buf));

                if (error > 0)
                {
                    sendcount++;
                }
                else if (error < 0)
                {
                    break;
                }

                client.ops.soo_recv(&client, buf, sizeof(buf));
            }

            client.ops.soo_close(&client);
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: connection to %s:%u failed (%d)\n",
                          __FUNCTION__,
                          client.ipaddr,
                          client.ipport,
                          errno);
        }

        logger_printf(LOGGER_LEVEL_DEBUG,
                      "%s: stopping thread '%s'\n",
                      __FUNCTION__,
                      instance->name);
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: thread instance is invalid\n",
                      __FUNCTION__);
    }

    return NULL;
}

/**
 * @brief TCP server thread function.
 *
 * @param[in] arg Thread input argument.
 *
 * @return NULL.
 */
void *thread_server_tcp(void * arg)
{
    struct thread_instance *instance = (struct thread_instance *)arg;
    char buf[2048];
    int32_t error;
    int32_t count = 0, recvbytes = 0;
    struct sockobj server, socket;

    logger_printf(LOGGER_LEVEL_DEBUG,
                  "%s: starting thread '%s'\n",
                  __FUNCTION__,
                  instance->name);

    if (instance != NULL)
    {
        memset(&server, 0, sizeof(server));
        socktcp_create(&server);
        server.ipaddr = "127.0.0.1";
        server.ipport = 5001;

        if ((server.ops.soo_open(&server) == true) &&
            (server.ops.soo_bind(&server) == true) &&
            (server.ops.soo_listen(&server, 1) == true))
        {
            server.event.timeoutms = 1000;

            logger_printf(LOGGER_LEVEL_DEBUG,
                          "%s: listening on %s\n",
                          __FUNCTION__,
                          server.addrself.sockaddrstr);

            while (thread_instance_isrunning(instance) == false)
            {
                if (count <= 0)
                {
                    if (server.ops.soo_accept(&server, &socket) == true)
                    {
                        logger_printf(LOGGER_LEVEL_DEBUG,
                                      "%s: server accepted connection on %s\n",
                                      __FUNCTION__,
                                      server.addrself.sockaddrstr);

                        socket.event.timeoutms = 1000;
                        count++;
                    }
                }
                else
                {
                    error = socket.ops.soo_recv(&socket, buf, sizeof(buf));

                    if (error > 0)
                    {
                        logger_printf(LOGGER_LEVEL_DEBUG,
                                      "%s: read %d bytes from %s\n",
                                      __FUNCTION__,
                                      error,
                                      socket.addrself.sockaddrstr);
                        recvbytes += error;
                    }
                    else if (error < 0)
                    {
                        socket.ops.soo_close(&socket);
                        count--;
                        logger_printf(LOGGER_LEVEL_DEBUG,
                                      "%s: read a total of %d bytes from %s\n",
                                      __FUNCTION__,
                                      recvbytes,
                                      socket.addrself.sockaddrstr);
                        recvbytes = 0;
                    }
                }
            }

            if (count > 0)
            {
                socket.ops.soo_close(&socket);
                count--;
            }
        }

        logger_printf(LOGGER_LEVEL_DEBUG,
                      "%s: stopping thread '%s'\n",
                      __FUNCTION__,
                      instance->name);
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: thread instance is invalid\n",
                      __FUNCTION__);
    }

    return NULL;
}

/**
 * @brief UDP server thread function.
 *
 * @param[in] arg Thread input argument.
 *
 * @return NULL.
 */
void *thread_server_udp(void * arg)
{
    struct thread_instance *instance = (struct thread_instance *)arg;
    char buf[2048];
    int32_t error;
    struct sockobj server;

    logger_printf(LOGGER_LEVEL_DEBUG,
                  "%s: starting thread '%s'\n",
                  __FUNCTION__,
                  instance->name);

    if (instance != NULL)
    {
        memset(&server, 0, sizeof(server));
        sockudp_create(&server);
        server.ipaddr = "127.0.0.1";
        server.ipport = 5001;

        if ((server.ops.soo_open(&server) == true) &&
            (server.ops.soo_bind(&server) == true))
        {
            server.event.timeoutms = 1000;

            logger_printf(LOGGER_LEVEL_DEBUG,
                          "%s: listening on %s\n",
                          __FUNCTION__,
                          server.addrself.sockaddrstr);

            while (thread_instance_isrunning(instance) == false)
            {
                error = server.ops.soo_recv(&server, buf, sizeof(buf));

                if (error >= 0)
                {
                    if (error > 0)
                    {
                        logger_printf(LOGGER_LEVEL_DEBUG,
                                      "%s: read %d bytes from %s:%u\n",
                                      __FUNCTION__,
                                      error,
                                      server.addrpeer.ipaddr,
                                      server.addrpeer.ipport);
                    }
                }
                else
                {
                    break;
                }
            }

            server.ops.soo_close(&server);
        }

        logger_printf(LOGGER_LEVEL_DEBUG,
                      "%s: stopping thread '%s'\n",
                      __FUNCTION__,
                      instance->name);
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: thread instance is invalid\n",
                      __FUNCTION__);
    }

    return NULL;
}

/**
 * @brief UDP client thread function.
 *
 * @param[in] arg Thread input argument.
 *
 * @return NULL.
 */
void *thread_client_udp(void * arg)
{
    struct thread_instance *instance = (struct thread_instance *)arg;
    char buf[248];
    int32_t error, sendcount;
    struct sockobj client;

    logger_printf(LOGGER_LEVEL_DEBUG,
                  "%s: starting thread '%s'\n",
                  __FUNCTION__,
                  instance->name);

    if (instance != NULL)
    {
        memset(&client, 0, sizeof(client));
        sockudp_create(&client);
        client.ipaddr = "127.0.0.1";
        client.ipport = 5001;

        if ((client.ops.soo_open(&client) == true) &&
            ((client.event.timeoutms = 1000) > 0) /*&&
            (client.ops.soo_bind(&client) == true)*/ &&
            (client.ops.soo_connect(&client) == true))
        {
            logger_printf(LOGGER_LEVEL_DEBUG,
                          "%s: connected to %s\n",
                          __FUNCTION__,
                          client.addrpeer.sockaddrstr);

            client.event.timeoutms = 1000;
            sendcount = 0;

            while ((thread_instance_isrunning(instance) == false) &&
                   (sendcount < 5))
            {
                error = client.ops.soo_send(&client, buf, sizeof(buf));
logger_printf(LOGGER_LEVEL_ERROR, "%s: sending\n", __FUNCTION__); //??
                if (error > 0)
                {
                    sendcount++;
                }
                else if (error < 0)
                {
                    break;
                }

                client.ops.soo_recv(&client, buf, sizeof(buf));
            }

            client.ops.soo_close(&client);
        }
        else
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: connection to %s:%u failed (%d)\n",
                          __FUNCTION__,
                          client.ipaddr,
                          client.ipport,
                          errno);
        }

        logger_printf(LOGGER_LEVEL_DEBUG,
                      "%s: stopping thread '%s'\n",
                      __FUNCTION__,
                      instance->name);
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: thread instance is invalid\n",
                      __FUNCTION__);
    }

    return NULL;
}

/**
 * @brief Input thread function.
 *
 * @param[in] arg Thread input argument.
 *
 * @return NULL.
 */
void *thread_input(void * arg)
{
    struct thread_instance *instance = (struct thread_instance *)arg;
    char buf[2048];
    uint16_t cols = 0, rows = 0;
    int32_t pos = 0;

    logger_printf(LOGGER_LEVEL_DEBUG,
                  "%s: starting thread '%s'\n",
                  __FUNCTION__,
                  instance->name);

    if (instance != NULL)
    {
        while (thread_instance_isrunning(instance) == false)
        {
            if (input_if_std_recv(buf, sizeof(buf), 1000) > 0)
            {
                utilioctl_gettermsize(&rows, &cols);
                pos = (pos < 0 ? cols / 4 : -cols / 4);
                logger_printf(LOGGER_LEVEL_DEBUG,
                              "%s: read from input (%u, %u, %d): '%*s'\n",
                              __FUNCTION__,
                              rows,
                              cols,
                              pos,
                              pos,
                              buf);
            }
        }

        logger_printf(LOGGER_LEVEL_DEBUG,
                      "%s: stopping thread '%s'\n",
                      __FUNCTION__,
                      instance->name);
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: thread instance is invalid\n",
                      __FUNCTION__);
    }

    return NULL;
}

/**
 * @brief Bottlerocket entry point.
 *
 * @param argc Command-line argument count.
 * @param argv Command-line argument vector containing the values of the
 *             command-line arguments.
 *
 * @return EXIT_SUCCESS on success or error value on failure.
 */
int32_t main(int argc, char **argv)
{
    int32_t retval = EXIT_SUCCESS;
    uint32_t threadcount = utilsysctl_getcpusavail();
    struct thread_instance *threads = NULL;
    struct output_if_ops output_if;
    struct args_obj args;
    uint16_t i;

    if (args_parse(argc, argv, &args) == false)
    {
        exit(EXIT_FAILURE);
    }

    logger_create();
    output_if.oio_send = output_if_std_send;
    logger_set_output(&output_if);
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Catch and handle signals.
    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGSTOP, signal_handler);
    signal(SIGTERM, signal_handler);

    if (threadcount != 3)
    {
        threadcount = 3;
    }

    threads = (struct thread_instance *)malloc(threadcount * sizeof(struct thread_instance));

    if (argc > 0 && argv)
    {
        for (i = 0; i < threadcount; i++)
        {
            utilstring_concat(threads[i].name,
                              sizeof(threads[i].name),
                              "t-%02d",
                              i);
            switch (i)
            {
                case 0:
                    threads[i].function = thread_server_tcp;
                    break;
                case 1:
                    threads[i].function = thread_server_udp;
                    break;
                case 2:
                    threads[i].function = thread_input;
                    break;
                case 3:
                    threads[i].function = thread_client_tcp;
                    break;
                case 4:
                    threads[i].function = thread_client_udp;
                    break;
                default:
                    break;
            }
            threads[i].argument = &threads[i];

            thread_instance_create(&threads[i]);
            thread_instance_start(&threads[i]);
        }
    }

    pause();

    for (i = 0; i < threadcount; i++)
    {
        thread_instance_stop(&threads[i]);
        thread_instance_destroy(&threads[i]);
    }

    logger_printf(LOGGER_LEVEL_TRACE, "%s: exiting\n", __FUNCTION__);
    logger_destroy();

    return retval;
}
