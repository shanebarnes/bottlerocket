/**
 * @file   bottlerocket.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  A socket benchmark utility implementation.
 */

#include "system_types.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static bool m_bShutdown = false;

/**
 * @brief Signal handler callback function.
 *
 * @param nSigNum The captured signal number.
 *
 * @return Void.
 */
void signal_handler(int nSigNum)
{
    switch (nSigNum)
    {
        case SIGHUP:
        case SIGINT:
        case SIGKILL:
        case SIGSEGV:
        case SIGSTOP:
        case SIGTERM:
        default:
            m_bShutdown = true;
    }

    exit(nSigNum);
}

/**
 * @brief Bottlerocket entry point.
 *
 * @param argc Command-line argument count.
 * @param argv Command-line argument vector containing the values of the
 *             command-line arguments.
 *
 * @return nRetVal - EXIT_SUCCESS is returned on success or error value is
 *                   returned on failure.
 */
int32_t main(int argc, char **argv)
{
    int32_t nRetVal = EXIT_SUCCESS;

    // Catch and handle signals.
    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGSTOP, signal_handler);

    if (argc > 0 && argv)
    {

    }

    m_bShutdown = true;

    return nRetVal;
}