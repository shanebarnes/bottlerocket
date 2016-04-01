/**
 * @file      args.c
 * @brief     Bottlerocket arguments implementation.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "args.h"
#include "logger.h"
#include "util_string.h"
#include "version.h"

#include <stdio.h>
#include <string.h>

struct tuple_element
{
  const char *lname;  // Tuple attribute long name (e.g., --argument)
  const char  sname;  // Tuple attribute short name (e.g., -a)
  const char *desc;   // Tuple description
  const char *dval;   // Tuple default value
  void (*func)(void); // Tuple validation function pointer.
};

static const char *prefix_skey = "-";
static const char *prefix_lkey = "--";

static struct tuple_element options[] =
{
    {"chat",       0,   "enable chat mode",
        "disabled",       NULL},
    {"perf",       0,   "enable performance benchmarking mode",
        "enabled",        NULL},
#if !defined(__APPLE__)
    {"--affinity", 'A', "set CPU affinity",
        "0xFFFFFFFF",     NULL},
#endif
    {"--bind",     'B', "bind to a specific socket address",
        "127.0.0.1:5001", NULL},
    {"--client",   'c', "run as a client",
        "127.0.0.1:5001", NULL},
    {"--server",   's', "run as a server",
        "0.0.0.0:5001",   NULL},
    {"--port",     'p', "server port to listen on or connect to",
        "5001",           NULL},
    {"--help",     'h', "print help information and quit",
        NULL,             NULL},
    {"--version",  'v', "print version information and quit",
        NULL,             NULL}
};

/**
 * @brief Print a bottlerocket arguments  usage message to a file stream.
 *
 * @param[in] stream A pointer to a file stream.
 *
 * @return Void.
 */
static void args_usage(FILE * const stream)
{
    uint32_t i;

    fprintf(stream, "\nusage: bottlerocket [options]\n\n");

    for (i = 0; i < sizeof(options) / sizeof(struct tuple_element); i++)
    {
        fprintf(stream,
                "  %s%c%s %-10s %-40s %s\n",
                options[i].sname != 0 ? prefix_skey : " ",
                options[i].sname != 0 ? options[i].sname : ' ',
                options[i].sname != 0 ? "," : " ",
                options[i].lname,
                options[i].desc,
                options[i].dval == NULL ? "" : options[i].dval);
    }

    fprintf(stream, "\n");
}

/**
 * @brief Get an argument (i.e., key-value pair) from an argument vector and map
 *        it to a bottlerocket argument element.
 *
 * @param[in] argc An argument count.
 * @param[in] argv An argument vector.
 * @param[in] argi A pointer to an argument vector index.
 *
 * @return A character representing the unique bottlerocket argument element (0
 *         on error).
 */
static char args_getarg(const int32_t argc, char ** const argv, int32_t * argi)
{
    char retval = 0, c;
    uint32_t i;

    if ((argv == NULL) || (argi == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 0; i < sizeof(options) / sizeof(struct tuple_element); i++)
        {
            if (utilstring_parse(argv[*argi], "-%c", &c) == 1)
            {
                // Short names are case-sensitive.
                if (c == options[i].sname)
                {
                    retval = c;
                    break;
                }
            }

            // Long names are not-case sensitive.
            if (utilstring_compare(argv[*argi],
                                   options[i].lname,
                                   0,
                                   true) == true)
            {
                retval = options[i].sname;
                break;
            }
        }

        // If an argument attribute name was found, then check the next array
        // index for the argument attribute value if a value is expected. A
        // value is only required if a default value was specified in the
        // matching argument tuple.
        if ((retval != 0) &&
            ((*argi+ 1) < argc) &&
            (options[i].dval != NULL) &&
            (options[i+1].sname != 0))
        {
            // Check if next value starts with a short- or long-key prefix.
            if ((utilstring_compare(argv[*argi+1],
                                    prefix_skey,
                                    strlen(prefix_skey), true) == false) &&
                (utilstring_compare(argv[*argi+1],
                                    prefix_lkey,
                                    strlen(prefix_lkey), true) == false))
            {
                (*argi)++;

                fprintf(stderr, "val = %s\n", argv[*argi]);
            }
            else
            {
                // @todo value error should be output rather than key error.
                retval = 0;
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool args_parse(const int32_t argc,
                char ** const argv,
                struct args_obj * const args)
{
    bool retval = true;
    int32_t i;

    if ((argv == NULL) || (args == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        for (i = 1; (i < argc) && (retval == true); i++)
        {
            switch (args_getarg(argc, argv, &i))
            {
#if !defined(__APPLE__)
                case 'A':
#endif
                case 'B':
                case 'c':
                case 's':
                case 'p':
                    break;
                case 'h':
                    args_usage(stdout);
                    break;
                case 'v':
                    fprintf(stdout,
                            "bottlerocket version %u.%u.%u (%s)\n",
                            version_major(),
                            version_minor(),
                            version_patch(),
                            version_date());
                    break;
                default:
                    fprintf(stderr,
                            "bottlerocket: unknown option '%s'\n",
                            argv[i]);
                    args_usage(stdout);
            }
        }
    }

    return retval;
}
