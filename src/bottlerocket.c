/**
 * @file   bottlerocket.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  A socket benchmark utility implementation.
 */

#include "logger.h"
#include "manip_string.h"
#include "thread_instance.h"
#include "system_types.h"
#include "util_sysctl.h"

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
 * @brief Thread worker function.
 *
 * @param[in] arg Thread input argument.
 *
 * @return NULL.
 */
void *thread_function(void * arg)
{
    struct thread_instance *instance = (struct thread_instance *)arg;

    logger_printf(LOGGER_LEVEL_TRACE,
                  "%s: Starting thread '%s'\n",
                  __FUNCTION__,
                  instance->name);

    if (instance != NULL)
    {
        while (thread_instance_isrunning(instance) == false)
        {
            logger_printf(LOGGER_LEVEL_TRACE,
                          "Thread '%s' is running\n",
                          instance->name);
            usleep(1000 * 1000);
        }

        logger_printf(LOGGER_LEVEL_TRACE,
                      "%s: Stopping thread '%s'\n",
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
    uint32_t cpucount = util_sysctl_cpuavail();
    struct thread_instance *threads = NULL;
    uint16_t i;

    logger_create();
    logger_set_level(LOGGER_LEVEL_TRACE);

    if (cpucount < 1)
    {
        cpucount = 1;
    }

    threads = (struct thread_instance *)malloc(cpucount * sizeof(struct thread_instance));

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
        for (i = 0; i < cpucount; i++)
        {
            manip_string_concat(threads[i].name,
                                sizeof(threads[i].name),
                                "t-%02d",
                                i);
            threads[i].function = thread_function;
            threads[i].argument = &threads[i];

            thread_instance_create(&threads[i]);
            thread_instance_start(&threads[i]);
        }
    }

    pause();

    for (i = 0; i < cpucount; i++)
    {
        thread_instance_stop(&threads[i]);
        thread_instance_destroy(&threads[i]);
    }

    logger_printf(LOGGER_LEVEL_TRACE, "Exiting\n");
    logger_destroy();

    return retval;
}
