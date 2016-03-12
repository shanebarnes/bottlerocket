/**
 * @file   cli_options.c
 * @author Shane Barnes
 * @date   12 Mar 2016
 * @brief  Command-line interface options implementation.
 */

#include "cli_options.h"

#include <stdio.h>

/**
 * @brief Print a command-line usage message to a file stream.
 *
 * @param[in] stream A pointer to a file stream.
 *
 * @return Void.
 */
static void cli_options_usage(FILE * const stream)
{
    fprintf(stream, "\nUsage: bottlerocket [options]\n\n");
    fprintf(stream, "  -h            Print help information and quit\n");
    fprintf(stream, "  -v            Print version information and quit\n");
    fprintf(stream, "\n");
}

/**
 * @see See header file for interace comments.
 */
bool cli_options_decode(const int32_t argc,
                        char ** const argv,
                        struct cli_options_instance * const instance)
{
    bool retval = false;

    if (instance != NULL)
    {
        if ((argc > 1) && (argv != NULL))
        {
            instance->exit = true;
            cli_options_usage(stdout);
        }
        else
        {
            instance->exit = false;
        }
    }

    return retval;
}
