/**
 * @file   bottlerocket.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  A socket benchmark utility implementation.
 */

#include "logger.h"
#include "system_types.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static bool shutdown = false;

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
        case SIGINT:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGINT\n");
        case SIGKILL:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGKILL\n");
        case SIGSEGV:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGSEGV\n");
        case SIGSTOP:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGSTOP\n");
        case SIGTERM:
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGTERM\n");
        default:
            shutdown = true;
    }

    exit(signum);
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

    logger_set_level(LOGGER_LEVEL_INFO);

    // Catch and handle signals.
    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGSTOP, signal_handler);

    if (argc > 0 && argv)
    {

    }

    shutdown = true;

    logger_printf(LOGGER_LEVEL_TRACE, "Exiting\n");

    return retval;
}
