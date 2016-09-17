/**
 * @file      bottlerocket.c
 * @brief     A socket benchmark utility implementation.
 * @author    Shane Barnes
 * @date      01 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "args.h"
#include "logger.h"
#include "mode_chat.h"
#include "mode_obj.h"
#include "mode_perf.h"
#include "output_if_instance.h"
#include "output_if_std.h"
#include "system_types.h"
#include "util_debug.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static struct modeobj mode =
{
    .ops.mode_init = NULL,
    .ops.mode_start = NULL,
    .ops.mode_stop = NULL,
    .ops.mode_cancel = NULL
};

/**
 * @brief Signal handler callback function.
 *
 * @param[in] signum The captured signal number.
 *
 * @return Void.
 */
void signal_handler(int signum)
{
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
            utildebug_backtrace();
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGSEGV\n");
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

    if (mode.ops.mode_cancel != NULL)
    {
        mode.ops.mode_cancel();
    }
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
    struct output_if_ops output_if;
    struct args_obj args;
    memset(&args, 0, sizeof(args));

    logger_create();
    output_if.oio_send = output_if_std_send;
    logger_set_output(&output_if);
    logger_set_level(LOGGER_LEVEL_WARN);

    // Catch and handle signals.
    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGSTOP, signal_handler);
    signal(SIGTERM, signal_handler);

    if (args_parse(argc, argv, &args) == false)
    {
        retval = EXIT_FAILURE;
    }
    else
    {
        if (args.mode == ARGS_MODE_CHAT)
        {
            mode.ops.mode_init = modechat_init;
            mode.ops.mode_start = modechat_start;
            mode.ops.mode_stop = modechat_stop;
            mode.ops.mode_cancel = modechat_cancel;
        }
        else if (args.mode == ARGS_MODE_PERF)
        {
            mode.ops.mode_init = modeperf_init;
            mode.ops.mode_start = modeperf_start;
            mode.ops.mode_stop = modeperf_stop;
            mode.ops.mode_cancel = modeperf_cancel;
        }

        mode.ops.mode_init(&args);
        mode.ops.mode_start();
        mode.ops.mode_stop();
    }

    logger_destroy();

    return retval;
}
