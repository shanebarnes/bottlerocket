/**
 * @file   bottlerocket.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  A socket benchmark utility implementation.
 */

#include "logger.h"
#include "pthreads_instance.h"
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
            logger_printf(LOGGER_LEVEL_INFO, "Caught SIGSEGV\n");
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

    shutdown = true;
    //exit(signum);
}

/**
 * @brief Thread worker function.
 *
 * @param[in] arg Thread input argument.
 *
 * @return NULL.
 */
void *thread_function(void * arg)
{
    struct pthreads_instance *instance = (struct pthreads_instance *)arg;

    logger_printf(LOGGER_LEVEL_TRACE, "%s: Starting thread\n", __FUNCTION__);

    if (instance != NULL)
    {
        while (instance->shutdown == false)
        {
            logger_printf(LOGGER_LEVEL_TRACE, "Thread running\n");
            usleep(1000 * 1000);
        }
    }
    else
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: thread instance is invalid\n",
                      __FUNCTION__);
    }

    logger_printf(LOGGER_LEVEL_TRACE, "%s: Stopping thread\n", __FUNCTION__);

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
    struct pthreads_instance thread;

    logger_set_level(LOGGER_LEVEL_INFO);

    // Catch and handle signals.
    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGSTOP, signal_handler);
    signal(SIGTERM, signal_handler);

    if (argc > 0 && argv)
    {
        thread.function = thread_function;
        thread.argument = &thread;
        thread.shutdown = false;

        pthreads_instance_create(&thread);
        pthreads_instance_start(&thread);
    }

    pause();

    pthreads_instance_stop(&thread);
    pthreads_instance_destroy(&thread);

    shutdown = true;

    logger_printf(LOGGER_LEVEL_TRACE, "Exiting\n");

    return retval;
}
