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

#define arg_optional true
#define arg_required false
#define val_optional true
#define val_required false

enum args_flag
{
    ARGS_FLAG_NULL      = 0x00000,
    ARGS_FLAG_CHAT      = 0x00001,
    ARGS_FLAG_PERF      = 0x00002,
    ARGS_FLAG_IPV4      = 0x00004,
    ARGS_FLAG_IPV6      = 0x00008,
    ARGS_FLAG_AFFINITY  = 0x00010,
    ARGS_FLAG_BIND      = 0x00020,
    ARGS_FLAG_BANDWIDTH = 0x00040,
    ARGS_FLAG_CLIENT    = 0x00080,
    ARGS_FLAG_INTERVAL  = 0x00100,
    ARGS_FLAG_LEN       = 0x00200,
    ARGS_FLAG_NUM       = 0x00400,
    ARGS_FLAG_PORT      = 0x00800,
    ARGS_FLAG_BACKLOG   = 0x01000,
    ARGS_FLAG_SERVER    = 0x02000,
    ARGS_FLAG_TIME      = 0x04000,
    ARGS_FLAG_UDP       = 0x08000,
    ARGS_FLAG_HELP      = 0x10000,
    ARGS_FLAG_VERSION   = 0x20000
};

struct tuple_element
{
  const char             *lname;  // Attribute long name (e.g., --argument)
  const char              sname;  // Attribute short name (e.g., -a)
  const char             *desc;   // Description
  const char             *dval;   // Default value
  const char             *minval; // Minimum value
  const char             *maxval; // Maximum value
  const bool              oval;   // Optional value
  const bool              oarg;   // Optional attribute
  const uint64_t          cflags; // Conflict flags (i.e., incompatible options)
  bool       (*copy)(const char * const val,
                     const char * const min,
                     const char * const max,
                     struct args_obj *args); // Tuple copy function pointer
};

static char        str_somaxconn[16];
static const char *prefix_skey = "-";
static const char *prefix_lkey = "--";

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
 * @brief Validate and copy a 32-bit integer value to a 32-bit integer variable.
 *
 * @param[in]     val A pointer to a value to validate.
 * @param[in]     min A pointer to a minimum value (NULL if no minimum value).
 * @param[in]     max A pointer to a maximum value (NULL if no maximum value).
 * @param[in,out] num A pointer to a 32-bit integer variable.
 *
 * @return True if a valid 32-bit integer value was copied to an 32-bit integer
 *         variable.
 */
static bool args_copyint32(const char * const val,
                           const char * const min,
                           const char * const max,
                           int32_t * const num)
{
    bool retval = false;
    int32_t dmax, dmin, tmp;

    if ((val == NULL) || (num == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((utilstring_parse(val, "%d", &tmp) == 1))
        {
            if ((min != NULL) &&
                ((utilstring_parse(min, "%d", &dmin) != 1) ||
                 (tmp < dmin)))
            {
                // Do nothing.
            }
            else if ((max != NULL) &&
                     ((utilstring_parse(max, "%d", &dmax) != 1) ||
                     (tmp > dmax)))
            {
                // Do nothing.
            }
            else
            {
                *num = tmp;
                retval = true;
            }
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
    int32_t port = -1;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (args_copyint32(val, min, max, &port) == true)
    {
        args->ipport = (uint16_t)port;
        retval = true;
    }

    return retval;
}

static bool args_copybacklog(const char * const val,
                             const char * const min,
                             const char * const max,
                             struct args_obj *args)
{
    bool retval = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (args_copyint32(val, min, max, &args->backlog) == true)
    {
        retval = true;
    }

    return retval;
}

static bool args_copybitrate(const char * const val,
                             const char * const min,
                             const char * const max,
                             uint64_t  * const num)
{
    bool retval = false;
    int64_t dmax, dmin, tmp;

    if ((val == NULL) || (num == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((tmp = utilunit_getbitrate(val)) >= 0)
        {
            if ((min != NULL) &&
                (((dmin = utilunit_getbitrate(min)) < 0) ||
                 (tmp < dmin)))
            {
                // Do nothing.
            }
            else if ((max != NULL) &&
                     (((dmax = utilunit_getbitrate(max)) < 0) ||
                      (tmp > dmax)))
            {
                // Do nothing.
            }
            else
            {
                *num = (uint64_t)tmp;
                retval = true;
            }
        }
    }

    return retval;
}

static bool args_copyratelimit(const char * const val,
                               const char * const min,
                               const char * const max,
                               struct args_obj *args)
{
    bool retval = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (args_copybitrate(val, min, max, &args->ratelimitbps) == true)
    {
        retval = true;
    }

    return retval;
}

static bool args_copybytes(const char * const val,
                           const char * const min,
                           const char * const max,
                           uint64_t  * const num)
{
    bool retval = false;
    uint64_t dmax, dmin, tmp;

    if ((val == NULL) || (num == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if ((tmp = utilunit_getbytes(val)) > 0)
        {
            if ((min != NULL) &&
                (((dmin = utilunit_getbytes(min)) == 0) ||
                 (tmp < dmin)))
            {
                // Do nothing.
            }
            else if ((max != NULL) &&
                    (((dmax = utilunit_getbytes(max)) == 0) ||
                     (tmp > dmax)))
            {
                // Do nothing.
            }
            else
            {
                *num = tmp;
                retval = true;
            }
        }
    }

    return retval;
}

static bool args_copybuflen(const char * const val,
                            const char * const min,
                            const char * const max,
                            struct args_obj *args)
{
    bool retval = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (args_copybytes(val, min, max, &args->buflen) == true)
    {
        retval = true;
    }

    return retval;
}

static bool args_copydatalimit(const char * const val,
                               const char * const min,
                               const char * const max,
                               struct args_obj *args)
{
    bool retval = false;

    if (args == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (args_copybytes(val, min, max, &args->datalimitbyte) == true)
    {
        retval = true;
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
                args->timelimitusec = utilunit_getsecs(val, UNIT_TIME_USEC);
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
        "enable chat mode",
        "disabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_PERF,
        NULL
    },
    {
        "perf",
        2,
        "enable performance benchmarking mode",
        "enabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_CHAT,
        NULL
    },
    {
        "--ipv4",
        '4',
        "only use IPv4",
        "enabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_IPV6,
        NULL
    },
    {
        "--ipv6",
        '6',
        "only use IPv6",
        "disabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_IPV4,
        NULL
    },
//#if !defined(__APPLE__)
    {
        "--affinity",
        'A',
        "set CPU affinity",
        "0xFFFFFFFF",
        NULL,
        NULL,
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        NULL
    },
//#endif
    {
        "--bind",
        'B',
        "bind to a specific socket address",
        "127.0.0.1:0",
        "0",
        "65535",
        val_required,
        arg_optional,
        ARGS_FLAG_SERVER,
        args_copyipport
    },
    {
        "--bandwidth",
        'b',
        "target bandwidth in bits per second",
        "0bps",
        "0bps",
        "999Ebps",
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        args_copyratelimit
    },
    {
        "--client",
        'c',
        "run as a client",
        "127.0.0.1",
        NULL,
        NULL,
        val_optional,
        arg_required,
        ARGS_FLAG_SERVER,
        args_copyipaddr
    },
    {
        "--interval",
        'i',
        "time between periodic bandwidth reports",
        "1s",
        "100ms",
        "1000y",
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        args_copytime
    },
    {
        "--len",
        'l',
        "length of buffer to read or write",
        "128kB",
        "1",
        "10MB",
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        args_copybuflen
    },
    {
        "--num",
        'n',
        "number of bytes to send or receive",
        "1MB",
        "1B",
        "999EB",
        val_required,
        arg_optional,
        ARGS_FLAG_TIME,
        args_copydatalimit
    },
    {
        "--port",
        'p',
        "server port to listen on or connect to",
        "5001",
        "0",
        "65535",
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        args_copyipport
    },
    {
        "--backlog",
        'q',
        "server backlog queue length",
        str_somaxconn,
        "1",
        str_somaxconn,
        val_required,
        arg_optional,
        ARGS_FLAG_CLIENT,
        args_copybacklog
    },
    {
        "--server",
        's',
        "run as a server",
        "0.0.0.0",
        NULL,
        NULL,
        val_optional,
        arg_required,
        ARGS_FLAG_CLIENT,
        args_copyipaddr
    },
    {
        "--time",
        't',
        "maximum time duration to send data",
        "10s",
        "1ps",
        "1000y",
        val_required,
        arg_optional,
        ARGS_FLAG_NUM,
        args_copytime
    },
    {
        "--udp",
        'u',
        "use UDP sockets instead of TCP sockets",
        "disabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_NULL,
        NULL
    },
    {
        "--help",
        'h',
        "print help information and quit",
        NULL,
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_NULL,
        NULL
    },
    {
        "--version",
        'v',
        "print version information and quit",
        NULL,
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_NULL,
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
                "  %s%c%s %-11s %-40s %s\n",
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
 * @param[in,out] mask A pointer to an options mask.
 * @param[in,out] args A pointer to a bottlerocket arguments structure to
 *                     populate.
 *
 * @return A character representing the unique bottlerocket argument element (0
 *         on error).
 */
static char args_getarg(const int32_t argc,
                        char ** const argv,
                        int32_t *argi,
                        uint64_t *mask,
                        struct args_obj *args)
{
    char retval = 0, c;
    char *name = NULL;
    uint64_t flag;
    uint32_t i;

    if ((argv == NULL) || (argi == NULL) || (mask == NULL) || (args == NULL))
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
                    name = argv[*argi];
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
                name = argv[*argi];
                break;
            }
        }

        if (retval == 0)
        {
            fprintf(stderr, "\nunknown option '%s'\n", name);
        }
        else if (((flag = (1 << i)) == 0) ||
                 (*mask & flag) ||
                 ((*mask = *mask | flag) == 0))
        {
            fprintf(stderr, "\nduplicate option '%s'\n", name);
            retval = 0;
        }
        else if ((options[i].cflags & *mask) != 0)
        {
             fprintf(stderr, "\nincompatible option '%s'\n", name);
             retval = 0;
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
                            name,
                            argv[*argi]);
                    retval = 0;
                }
            }
            else
            {
                if (options[i].oval == val_required)
                {
                    fprintf(stderr, "\nmissing value for option '%s'\n", name);
                    retval = 0;
                }
                else if (options[i].copy(options[i].dval,
                                         options[i].minval,
                                         options[i].maxval,
                                         args) == false)
                {
                    fprintf(stderr,
                            "\ninvalid option '%s %s'\n",
                            name,
                            options[i].dval);
                    retval = 0;
                }
            }
        }
        else
        {
            if (options[i].oval == val_required)
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
    bool retval = false;
    uint64_t flags = 0;
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
        utilstring_fromi32(SOMAXCONN, str_somaxconn, sizeof(str_somaxconn));
        args->mode = ARGS_MODE_PERF;
        args->arch = SOCKOBJ_MODEL_CLIENT;
        args->family = AF_INET;
        args->type = SOCK_STREAM;
        args_copyipaddr("0.0.0.0", "0.0.0.0", "0.0.0.0", args);
        args_copyipport("5001", "5001", "5001", args);
        args_copydatalimit("1MB", "1MB", "1MB", args);
        args_copyratelimit("0bps", "0bps", "0bps", args);
        //args_copytime("0s", "0s", "0s", args);
        args->timelimitusec = 0;
        args_copybuflen("128kB", "128kB", "128kB", args);

        if (argc > 1)
        {
            retval = true;
        }
        else
        {
            args_usage(stdout);
        }

        for (i = 1; (i < argc) && (retval == true); i++)
        {
            switch (args_getarg(argc, argv, &i, &flags, args))
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
                case '4':
                    args->family = AF_INET;
                    break;
                case '6':
                    args->family = AF_INET6;
                    break;
#if !defined(__APPLE__)
                case 'A':
                    break;
#endif
                case 'B':
                    break;
                case 'b':
                    break;
                case 'c':
                    args->arch = SOCKOBJ_MODEL_CLIENT;
                    break;
                case 'l':
                    break;
                case 'n':
                    break;
                case 's':
                    args->arch = SOCKOBJ_MODEL_SERVER;
                    if ((flags & ARGS_FLAG_NUM) == 0)
                    {
                        args->datalimitbyte = 0;
                    }
                    break;
                case 'p':
                    break;
                case 'q':
                    break;
                case 't':
                    if ((flags & ARGS_FLAG_NUM) == 0)
                    {
                        args->datalimitbyte = 0;
                    }
                    break;
                case 'u':
                    args->type = SOCK_DGRAM;
                    if ((flags & ARGS_FLAG_BANDWIDTH) == 0)
                    {
                        args_copyratelimit("1Mbps", "1Mbps", "1Mbps", args);
                    }
                    if ((flags & ARGS_FLAG_LEN) == 0)
                    {
                        args_copybuflen("1kB", "1kB", "1kB", args);
                    }
                    break;
                case 'h':
                    args_usage(stdout);
                    retval = false;
                    break;
                case 'v':
                    fprintf(stdout,
                            "bottlerocket version %u.%u.%u (%s)\n",
                            version_major(),
                            version_minor(),
                            version_patch(),
                            version_date());
                    retval = false;
                    break;
                default:
                    args_usage(stdout);
                    retval = false;
                    break;
            }
        }
    }

    return retval;
}
