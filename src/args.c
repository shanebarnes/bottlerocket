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
#include "util_inet.h"
#include "util_string.h"
#include "version.h"

#include <stdio.h>
#include <string.h>

enum args_group
{
    ARGS_GROUP_NONE = 0,
    ARGS_GROUP_MODE = 1,
    ARGS_GROUP_ARCH = 2,
    ARGS_GROUP_INFO = 3,
    ARGS_GROUP_MAX  = 4
};

struct tuple_element
{
  const char             *lname; // Tuple attribute long name (e.g., --argument)
  const char              sname; // Tuple attribute short name (e.g., -a)
  const enum  args_group  group; // Tuple group identification
  const char             *desc;  // Tuple description
  const char             *dval;  // Tuple default value
  const bool              oval;  // Tuple optional value
  const bool              oarg;  // Tuple optional attribute
  bool       (*copy)(const char * const val,
                     struct args_obj *args); // Tuple copy function pointer
};

static const char *prefix_skey = "-";
static const char *prefix_lkey = "--";
static uint16_t    groupcount[ARGS_GROUP_MAX];
/**
 * @brief Validate and copy an IP address value to an arguments object.
 *
 * @param[in]     val  A pointer to a value to validate.
 * @param[in,out] args A pointer to an argument object.
 *
 * @return True if a valid IP address value was copied to an arguments object.
 */
static bool args_copyipaddr(const char * const val, struct args_obj *args)
{
    bool retval = false;

    if ((val == NULL) || (args == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        retval = utilinet_getaddrfromhost(val,
                                          args->ipaddr,
                                          sizeof(args->ipaddr));
    }

    return retval;
}

/**
 * @brief Validate and copy an IP port number value to an arguments object.
 *
 * @param[in]     val  A pointer to a value to validate.
 * @param[in,out] args A pointer to an argument object.
 *
 * @return True if a valid IP port number value was copied to an arguments
 *         object.
 */
static bool args_copyipport(const char * const val, struct args_obj *args)
{
    bool retval = false;
    int32_t port;

    if ((val == NULL) || (args == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((utilstring_parse(val, "%d", &port) == 1) &&
            ((port & 0xFFFF0000) == 0))
        {
            args->ipport = (uint16_t)port;
            retval = true;
        }
    }

    return retval;
}

static struct tuple_element options[] =
{
    {"chat",
     0,
     ARGS_GROUP_NONE,
     "enable chat mode",
     "enabled",
     true,
     true,
     NULL
    },
    {"perf",
     0,
     ARGS_GROUP_NONE,
     "enable performance benchmarking mode",
     "disabled",
     true,
     true,
     NULL
    },
#if !defined(__APPLE__)
    {"--affinity",
     'A',
     ARGS_GROUP_NONE,
     "set CPU affinity",
     "0xFFFFFFFF",
     false,
     true,
     NULL
    },
#endif
    {"--bind",
     'B',
     ARGS_GROUP_NONE,
     "bind to a specific socket address",
     "127.0.0.1:0",
     false,
     true,
     args_copyipport
    },
    {"--client",
     'c',
     ARGS_GROUP_ARCH,
     "run as a client",
     "127.0.0.1",
     true,
     false,
     args_copyipaddr
    },
    {"--server",
     's',
     ARGS_GROUP_ARCH,
     "run as a server",
     "0.0.0.0",
     true,
     false,
     args_copyipaddr
    },
    {"--port",
     'p',
     ARGS_GROUP_NONE,
     "server port to listen on or connect to",
     "5001",
     false,
     true,
     args_copyipport
    },
    {"--help",
     'h',
     ARGS_GROUP_INFO,
     "print help information and quit",
     NULL,
     false,
     true,
     NULL
    },
    {"--version",
     'v',
     ARGS_GROUP_INFO,
     "print version information and quit",
     NULL,
     false,
     true,
     NULL
    }
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
 * @param[in]     argc An argument count.
 * @param[in]     argv An argument vector.
 * @param[in,out] argi A pointer to an argument vector index.
 * @param[in,out] args A pointer to a bottlerocket arguments structure to
 *                     populate.
 *
 * @return A character representing the unique bottlerocket argument element (0
 *         on error).
 */
static char args_getarg(const int32_t argc,
                        char ** const argv,
                        int32_t *argi,
                        struct args_obj *args)
{
    char retval = 0, c;
    uint32_t i;

    if ((argv == NULL) || (argi == NULL) || (args == NULL))
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
                    groupcount[options[i].group]++;
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
                groupcount[options[i].group]++;
                break;
            }
        }


        if (retval == 0)
        {
            fprintf(stderr,
                    "\nunkown option '%s'\n",
                    argv[*argi]);
        }
        else if (options[i].dval == NULL)
        {
            // Do nothing.
        }
        else if (options[i].copy == NULL)
        {
            // Do nothing.
        }
        // If an argument attribute name was found, then check the next array
        // index for the argument attribute value if a value is expected (i.e.,
        // default value is not null).
        else if ((*argi + 1) < argc)
        {
            // The argument attribute value cannot start with either a short- or
            // long-key prefix.
            if ((utilstring_compare(argv[*argi+1],
                                    prefix_skey,
                                    strlen(prefix_skey), true) == false) &&
                (utilstring_compare(argv[*argi+1],
                                    prefix_lkey,
                                    strlen(prefix_lkey), true) == false))
            {
                (*argi)++;

                if (options[i].copy(argv[*argi], args) == false)
                {
                    fprintf(stderr,
                            "\ninvalid option '%s %s'\n",
                            argv[*argi-1],
                            argv[*argi]);
                    retval = 0;
                }
            }
            else
            {
                if (options[i].oval == false)
                {
                    fprintf(stderr,
                            "\nmissing value for option '%s'\n",
                            argv[*argi]);
                    retval = 0;
                }
                else if (options[i].copy(options[i].dval, args) == false)
                {
                    fprintf(stderr,
                            "\ninvalid option '%s %s'\n",
                            argv[*argi],
                            options[i].dval);
                    retval = 0;
                }
            }
        }
        else
        {
            if (options[i].oval == false)
            {
                fprintf(stderr,
                        "\nmissing value for option '%s'\n",
                        argv[*argi]);
                retval = 0;
            }
            else if (options[i].copy(options[i].dval, args) == false)
            {
                fprintf(stderr,
                        "\ninvalid option '%s %s'\n",
                        argv[*argi],
                        options[i].dval);
                retval = 0;
            }
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
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
        // Set defaults.
        memset(groupcount, 0, sizeof(groupcount));
        args->mode = ARGS_MODE_CHAT;
        args->arch = ARGS_ARCH_NULL;
        args_copyipport("5001", args);

        for (i = 1; (i < argc) && (retval == true); i++)
        {
            switch (args_getarg(argc, argv, &i, args))
            {
#if !defined(__APPLE__)
                case 'A':
                    break;
#endif
                case 'B':
                    break;
                case 'c':
                    if (args->arch == ARGS_ARCH_NULL)
                    {
                        args->arch = ARGS_ARCH_CLIENT;
                    }
                    else
                    {
                        args_usage(stdout);
                        retval = false;
                    }
                    break;
                case 's':
                    if (args->arch == ARGS_ARCH_NULL)
                    {
                        args->arch = ARGS_ARCH_SERVER;
                    }
                    else
                    {
                        args_usage(stdout);
                        retval = false;
                    }
                    break;
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
                    args_usage(stdout);
                    retval = false;
            }
        }

        if (retval == true)
        {
            if (groupcount[ARGS_GROUP_INFO] > 0)
            {
                retval = false;
            }
            else if (groupcount[ARGS_GROUP_ARCH] != 1)
            {
                // Arguments in this group are mutually exclusive.

                args_usage(stdout);
                retval = false;
            }
        }
    }

    return retval;
}
