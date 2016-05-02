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
#include "util_unit.h"
#include "version.h"

#include <stdio.h>
#include <string.h>

static char str_somaxconn[12];

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
  const char             *lname;  // Attribute long name (e.g., --argument)
  const char              sname;  // Attribute short name (e.g., -a)
  const enum  args_group  group;  // Group identification
  const char             *desc;   // Description
  const char             *dval;   // Default value
  const char             *minval; // Minimum value
  const char             *maxval; // Maximum value
  const bool              oval;   // Optional value
  const bool              oarg;   // Optional attribute
  bool       (*copy)(const char * const val,
                     const char * const min,
                     const char * const max,
                     struct args_obj *args); // Tuple copy function pointer
};

static const char *prefix_skey = "-";
static const char *prefix_lkey = "--";
static uint16_t    groupcount[ARGS_GROUP_MAX];

/**
 * @brief Validate and copy an IP address value to an arguments object.
 *
 * @param[in]     val  A pointer to a value to validate.
 * @param[in]     min  A pointer to a minimum value (NULL if no minimum value).
 * @param[in]     max  A pointer to a maximum value (NULL if no maximum value).
 * @param[in,out] args A pointer to an argument object.
 *
 * @return True if a valid IP address value was copied to an arguments object.
 */
static bool args_copyipaddr(const char * const val,
                            const char * const min,
                            const char * const max,
                            struct args_obj *args)
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
        if ((min != NULL) && (false))
        {
            // Do nothing.
        }
        else if ((max != NULL) && (false))
        {
            // Do nothing.
        }
        else
        {
            retval = utilinet_getaddrfromhost(val,
                                              args->ipaddr,
                                              sizeof(args->ipaddr));
        }
    }

    return retval;
}

/**
 * @brief Validate and copy an IP port number value to an arguments object.
 *
 * @param[in]     val  A pointer to a value to validate.
 * @param[in]     min  A pointer to a minimum value (NULL if no minimum value).
 * @param[in]     max  A pointer to a maximum value (NULL if no maximum value).
 * @param[in,out] args A pointer to an argument object.
 *
 * @return True if a valid IP port number value was copied to an arguments
 *         object.
 */
static bool args_copyipport(const char * const val,
                            const char * const min,
                            const char * const max,
                            struct args_obj *args)
{
    bool retval = false;
    int32_t dmax, dmin, port;

    if ((val == NULL) || (args == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((utilstring_parse(val, "%d", &port) == 1))
        {
            if ((min != NULL) &&
                ((utilstring_parse(min, "%d", &dmin) != 1) ||
                 (port < dmin)))
            {
                // Do nothing.
            }
            else if ((max != NULL) &&
                     ((utilstring_parse(max, "%d", &dmax) != 1) ||
                     (port > dmax)))
            {
                // Do nothing.
            }
            else
            {
                args->ipport = (uint16_t)port;
                retval = true;
            }
        }
    }

    return retval;
}

/**
 * @brief Validate and copy a time value to an arguments object.
 *
 * @param[in]     val  A pointer to a value to validate.
 * @param[in]     min  A pointer to a minimum value (NULL if no minimum value).
 * @param[in]     max  A pointer to a maximum value (NULL if no maximum value).
 * @param[in,out] args A pointer to an argument object.
 *
 * @return True if a valid time value was copied to an arguments object.
 */
static bool args_copytime(const char * const val,
                          const char * const min,
                          const char * const max,
                          struct args_obj *args)
{
    bool retval = false;
    uint64_t dmax, dmin, time;

    if ((val == NULL) || (args == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((time = utilunit_getsecs(val, UNIT_TIME_PSEC)) > 0)
        {
            if ((min != NULL) &&
                (((dmin = utilunit_getsecs(min, UNIT_TIME_PSEC)) == 0) ||
                 (time < dmin)))
            {
                // Do nothing.
            }
            else if ((max != NULL) &&
                    (((dmax = utilunit_getsecs(max, UNIT_TIME_PSEC)) == 0) ||
                     (time > dmax)))
            {
                // Do nothing.
            }
            else
            {
                args->maxtimeusec = utilunit_getsecs(val, UNIT_TIME_USEC);
                retval = true;
            }
        }
    }

    return retval;
}

static struct tuple_element options[] =
{
    {
     "chat",
     1,
     ARGS_GROUP_MODE,
     "enable chat mode",
     "disabled",
     NULL,
     NULL,
     true,
     true,
     NULL
    },
    {
     "perf",
     2,
     ARGS_GROUP_MODE,
     "enable performance benchmarking mode",
     "enabled",
     NULL,
     NULL,
     true,
     true,
     NULL
    },
#if !defined(__APPLE__)
    {
     "--affinity",
     'A',
     ARGS_GROUP_NONE,
     "set CPU affinity",
     "0xFFFFFFFF",
     NULL,
     NULL,
     false,
     true,
     NULL
    },
#endif
    {
     "--bind",
     'B',
     ARGS_GROUP_NONE,
     "bind to a specific socket address",
     "127.0.0.1:0",
     "0",
     "65535",
     false,
     true,
     args_copyipport
    },
    {
     "--client",
     'c',
     ARGS_GROUP_ARCH,
     "run as a client",
     "127.0.0.1",
     NULL,
     NULL,
     true,
     false,
     args_copyipaddr
    },
    {
     "--server",
     's',
     ARGS_GROUP_ARCH,
     "run as a server",
     "0.0.0.0",
     NULL,
     NULL,
     true,
     false,
     args_copyipaddr
    },
    {
     "--port",
     'p',
     ARGS_GROUP_NONE,
     "server port to listen on or connect to",
     "5001",
     "0",
     "65535",
     false,
     true,
     args_copyipport
    },
    {
     "--backlog",
     'q',
     ARGS_GROUP_NONE,
     "server backlog queue length",
     str_somaxconn,
     "1",
     str_somaxconn,
     false,
     true,
     NULL
    },
    {
     "--time",
     't',
     ARGS_GROUP_NONE,
     "maximum time duration to send data",
     "10s",
     "1ps",
     "1000y",
     false,
     true,
     args_copytime
    },
    {
     "--help",
     'h',
     ARGS_GROUP_INFO,
     "print help information and quit",
     NULL,
     NULL,
     NULL,
     false,
     true,
     NULL
    },
    {
     "--version",
     'v',
     ARGS_GROUP_INFO,
     "print version information and quit",
     NULL,
     NULL,
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
                options[i].sname >= '0' ? prefix_skey : " ",
                options[i].sname >= '0' ? options[i].sname : ' ',
                options[i].sname >= '0' ? "," : " ",
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
            fprintf(stderr, "\nunknown option '%s'\n", argv[*argi]);
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

                if (options[i].copy(argv[*argi],
                                    options[i].minval,
                                    options[i].maxval,
                                    args) == false)
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
                else if (options[i].copy(options[i].dval,
                                         options[i].minval,
                                         options[i].maxval,
                                         args) == false)
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
            else if (options[i].copy(options[i].dval,
                                     options[i].minval,
                                     options[i].maxval,
                                     args) == false)
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
        utilstring_fromi32(SOMAXCONN, str_somaxconn, sizeof(str_somaxconn));

        // Set defaults.
        memset(groupcount, 0, sizeof(groupcount));
        args->mode = ARGS_MODE_PERF;
        args->arch = ARGS_ARCH_NULL;
        args_copyipport("5001", "5001", "5001", args);
        args_copytime("10s", "10s", "10s", args);

        for (i = 1; (i < argc) && (retval == true); i++)
        {
            switch (args_getarg(argc, argv, &i, args))
            {
                case 1:
                    if (i == 1)
                    {
                        args->mode = ARGS_MODE_CHAT;
                    }
                    else
                    {
                        args_usage(stdout);
                        retval = false;
                    }
                    break;
                case 2:
                    if (i == 1)
                    {
                        args->mode = ARGS_MODE_PERF;
                    }
                    else
                    {
                        args_usage(stdout);
                        retval = false;
                    }
                    break;
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
                case 't':
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
            else if ((groupcount[ARGS_GROUP_MODE] > 1) ||
                     (groupcount[ARGS_GROUP_ARCH] != 1))
            {
                // Arguments in these groups are mutually exclusive.
                args_usage(stdout);
                retval = false;
            }
        }
    }

    return retval;
}
