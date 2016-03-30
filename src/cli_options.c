/**
 * @file      cli_options.c
 * @brief     Command-line interface options implementation.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "cli_options.h"

#include <stdio.h>

static struct tuple_element options[] =
{
    {"--help",    "-h", "Print help information and quit",    NULL},
    {"--version", "-v", "Print version information and quit", NULL}
};

/**
 * @brief Print a command-line usage message to a file stream.
 *
 * @param[in] stream A pointer to a file stream.
 *
 * @return Void.
 */
static void cli_options_usage(FILE * const stream)
{
    uint32_t i;

    fprintf(stream, "\nUsage: bottlerocket [options]\n\n");

    for (i = 0; i < sizeof(options) / sizeof(struct tuple_element); i++)
    {
        fprintf(stream,
                "  %-2s, %-10s %s\n",
                options[i].sname,
                options[i].lname,
                options[i].desc);
    }

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
