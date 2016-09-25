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
#include "util_debug.h"
#include "util_inet.h"
#include "util_math.h"
#include "util_string.h"
#include "util_sysctl.h"
#include "util_unit.h"
#include "version.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define arg_noobjptr NULL
#define arg_optional true
#define arg_required false
#define val_optional true
#define val_required false

static char        str_somaxconn[16];
static char        str_nproconln[16];
static const char *prefix_skey = "-";
static const char *prefix_lkey = "--";

enum argsopt_flag
{
    ARGS_FLAG_NULL       = 0x000000,
    ARGS_FLAG_CHAT       = 0x000001,
    ARGS_FLAG_PERF       = 0x000002,
    ARGS_FLAG_IPV4       = 0x000004,
    ARGS_FLAG_IPV6       = 0x000008,
    ARGS_FLAG_AFFINITY   = 0x000010,
    ARGS_FLAG_BIND       = 0x000020,
    ARGS_FLAG_BANDWIDTH  = 0x000040,
    ARGS_FLAG_CLIENT     = 0x000080,
    ARGS_FLAG_ECHO       = 0x000100,
    ARGS_FLAG_INTERVAL   = 0x000200,
    ARGS_FLAG_LEN        = 0x000400,
    ARGS_FLAG_OPTNODELAY = 0x000800,
    ARGS_FLAG_NUM        = 0x001000,
    ARGS_FLAG_PARALLEL   = 0x002000,
    ARGS_FLAG_PORT       = 0x004000,
    ARGS_FLAG_BACKLOG    = 0x008000,
    ARGS_FLAG_SERVER     = 0x010000,
    ARGS_FLAG_THREADS    = 0x020000,
    ARGS_FLAG_TIME       = 0x040000,
    ARGS_FLAG_UDP        = 0x080000,
    ARGS_FLAG_HELP       = 0x100000,
    ARGS_FLAG_VERSION    = 0x200000
};

struct argsopt
{
    const char      *lname;  // Attribute long name (e.g., --argument)
    const char       sname;  // Attribute short name (e.g., -a)
    const char      *desc;   // Description
    const char      *dval;   // Default value
    const char      *minval; // Minimum value
    const char      *maxval; // Maximum value
    const bool       oval;   // Optional value
    const bool       oarg;   // Optional attribute
    const uint64_t   cflags; // Conflict flags (i.e., incompatible options)
    struct args_obj *args;  // Pointer to current argument values.
                            // Option copy function pointer
    bool            (*copy)(const struct argsopt * const opt,
                            const char * const src,
                            void * const dst);
    void            *dest;   // Copy destination.
};

/**
 * @see
 */
static bool args_copyipaddr(const struct argsopt * const opt,
                            const char * const src,
                            void * const dst)
{
    bool ret = false;

    if (UTILDEBUG_VERIFY((opt != NULL) &&
                         (opt->args != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)) == true)
    {
        if ((opt->minval != NULL) && (false))
        {
            // Do nothing.
        }
        else if ((opt->maxval != NULL) && (false))
        {
            // Do nothing.
        }
        else
        {
            ret = utilinet_getaddrfromhost(src,
                                           opt->args->family,
                                           dst,
                                           INET6_ADDRSTRLEN);
        }
    }

    return ret;
}

/**
 * @see
 */
static bool args_copyuint16(const struct argsopt * const opt,
                            const char * const src,
                            void * const dst)
{
    bool ret = false;
    uint32_t max, min, val;

    if (UTILDEBUG_VERIFY((opt != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)) == true)
    {
        if ((utilstring_parse(src, "%u", &val) == 1))
        {
            if ((opt->minval != NULL) &&
                ((utilstring_parse(opt->minval, "%u", &min) != 1) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((opt->maxval != NULL) &&
                     ((utilstring_parse(opt->maxval, "%u", &max) != 1) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint16_t*)dst = (uint16_t)val;
                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @see
 */
static bool args_copyint32(const struct argsopt * const opt,
                           const char * const src,
                           void * const dst)
{
    bool ret = false;
    int32_t max, min, val;

    if (UTILDEBUG_VERIFY((opt != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)) == true)
    {
        if ((utilstring_parse(src, "%d", &val) == 1))
        {
            if ((opt->minval != NULL) &&
                ((utilstring_parse(opt->minval, "%d", &min) != 1) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((opt->maxval != NULL) &&
                     ((utilstring_parse(opt->maxval, "%d", &max) != 1) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(int32_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @see
 */
static bool args_copyuint32(const struct argsopt * const opt,
                            const char * const src,
                            void * const dst)
{
    bool ret = false;
    uint32_t max, min, val;

    if (UTILDEBUG_VERIFY((opt != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)) == true)
    {
        if ((utilstring_parse(src, "%u", &val) == 1))
        {
            if ((opt->minval != NULL) &&
                ((utilstring_parse(opt->minval, "%u", &min) != 1) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((opt->maxval != NULL) &&
                     ((utilstring_parse(opt->maxval, "%u", &max) != 1) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint32_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @see
 */
static bool args_copyrateunit(const struct argsopt * const opt,
                              const char * const src,
                              void  * const dst)
{
    bool ret = false;
    int64_t max, min, val;

    if (UTILDEBUG_VERIFY((opt != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)) == true)
    {
        if ((val = utilunit_getbitrate(src)) >= 0)
        {
            if ((opt->minval != NULL) &&
                (((min = utilunit_getbitrate(opt->minval)) < 0) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((opt->maxval != NULL) &&
                     (((max = utilunit_getbitrate(opt->maxval)) < 0) ||
                      (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint64_t*)dst = (uint64_t)val;
                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @see
 */
static bool args_copybyteunit(const struct argsopt * const opt,
                              const char * const src,
                              void * const dst)
{
    bool ret = false;
    uint64_t max, min, val;

    if (UTILDEBUG_VERIFY((opt != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)) == true)
    {
        if ((val = utilunit_getbytes(src)) > 0)
        {
            if ((opt->minval != NULL) &&
                (((min = utilunit_getbytes(opt->minval)) == 0) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((opt->maxval != NULL) &&
                    (((max = utilunit_getbytes(opt->maxval)) == 0) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint64_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @see
 */
static bool args_copytimeunit(const struct argsopt * const opt,
                              const char * const src,
                              void * const dst)
{
    bool ret = false;
    uint64_t max, min, val;

    if (UTILDEBUG_VERIFY((opt != NULL) &&
                         (src != NULL) &&
                         (dst != NULL)) == true)
    {
        if ((val = utilunit_getsecs(src, UNIT_TIME_USEC)) > 0)
        {
            if ((opt->minval != NULL) &&
                (((min = utilunit_getsecs(opt->minval, UNIT_TIME_USEC)) == 0) ||
                 (val < min)))
            {
                // Do nothing.
            }
            else if ((opt->maxval != NULL) &&
                    (((max = utilunit_getsecs(opt->maxval, UNIT_TIME_USEC)) == 0) ||
                     (val > max)))
            {
                // Do nothing.
            }
            else
            {
                *(uint64_t*)dst = val;
                ret = true;
            }
        }
    }

    return ret;
}

static struct argsopt options[] =
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
        arg_noobjptr,
        NULL,
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
        arg_noobjptr,
        NULL,
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
        arg_noobjptr,
        NULL,
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
        arg_noobjptr,
        NULL,
        NULL
    },
    {
        "--affinity",
        'A',
        "set CPU affinity",
        str_nproconln,
        "1",
        str_nproconln,
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        arg_noobjptr,
        args_copyuint16,
        NULL
    },
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
        arg_noobjptr,
        args_copyuint16,
        NULL
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
        arg_noobjptr,
        args_copyrateunit,
        NULL
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
        arg_noobjptr,
        args_copyipaddr,
        NULL
    },
    {
        "--echo",
        'e',
        "echo reception back to client",
        "disabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_CLIENT,
        arg_noobjptr,
        NULL,
        NULL
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
        arg_noobjptr,
        args_copytimeunit,
        NULL
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
        arg_noobjptr,
        args_copybyteunit,
        NULL
    },
    {
        "--nodelay",
        'N',
        "set TCP no delay (disable Nagle's algorithm)",
        "disabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_UDP,
        arg_noobjptr,
        NULL,
        NULL
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
        arg_noobjptr,
        args_copybyteunit,
        NULL
    },
    {
        "--parallel",
        'P',
        "maximum number of concurrent connections",
        "1",
        "0",
        "10000",
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        arg_noobjptr,
        args_copyuint32,
        NULL
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
        arg_noobjptr,
        args_copyuint16,
        NULL
    },
    {
        "--backlog",
        'q',
        "client or server backlog queue length",
        str_somaxconn,
        "0",
        str_somaxconn,
        val_required,
        arg_optional,
        ARGS_FLAG_NULL,
        arg_noobjptr,
        args_copyint32,
        NULL
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
        arg_noobjptr,
        args_copyipaddr,
        NULL
    },
    {
        "--threads",
        'T',
        "number of threads to use",
        "1",
        "1",
        str_nproconln,
        val_required,
        arg_optional,
        ARGS_FLAG_CHAT,
        arg_noobjptr,
        args_copyuint32,
        NULL
    },
    {
        "--time",
        't',
        "maximum time duration to send data",
        "0s",
        "0s",
        "1000y",
        val_required,
        arg_optional,
        ARGS_FLAG_NUM,
        arg_noobjptr,
        args_copytimeunit,
        NULL
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
        ARGS_FLAG_OPTNODELAY,
        arg_noobjptr,
        NULL,
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
        arg_noobjptr,
        NULL,
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
        arg_noobjptr,
        NULL,
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

    for (i = 0; i < sizeof(options) / sizeof(struct argsopt); i++)
    {
        fprintf(stream,
                "  %s%c%s %-11s %-50s %s\n",
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
 *
 * @return A character representing the unique bottlerocket argument element (0
 *         on error).
 */
static char args_getarg(const int32_t argc,
                        char ** const argv,
                        int32_t *argi,
                        uint64_t *mask)
{
    char ret = 0, c;
    char *name = NULL;
    uint64_t flag;
    uint32_t i;

    if (UTILDEBUG_VERIFY((argv != NULL) &&
                         (argi != NULL) &&
                         (mask != NULL)) == true)
    {
        for (i = 0; i < sizeof(options) / sizeof(struct argsopt); i++)
        {
            if (utilstring_parse(argv[*argi], "-%c", &c) == 1)
            {
                // Short names are case-sensitive.
                if (c == options[i].sname)
                {
                    ret = c;
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
                ret = options[i].sname;
                name = argv[*argi];
                break;
            }
        }

        if (ret == 0)
        {
            fprintf(stderr, "\nunknown option '%s'\n", name);
        }
        else if (((flag = (1 << i)) == 0) ||
                 (*mask & flag) ||
                 ((*mask = *mask | flag) == 0))
        {
            fprintf(stderr, "\nduplicate option '%s'\n", name);
            ret = 0;
        }
        else if ((options[i].cflags & *mask) != 0)
        {
             fprintf(stderr, "\nincompatible option '%s'\n", name);
             ret = 0;
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

                if (options[i].copy(&options[i],
                                    argv[*argi],
                                    options[i].dest) == false)
                {
                    fprintf(stderr,
                            "\ninvalid option '%s %s' (limits: [%s, %s])\n",
                            name,
                            argv[*argi],
                            options[i].minval,
                            options[i].maxval);
                    ret = 0;
                }
            }
            else
            {
                if (options[i].oval == val_required)
                {
                    fprintf(stderr,
                            "\nmissing value for option '%s' (limits: [%s, %s])\n",
                            name,
                            options[i].minval,
                            options[i].maxval);
                    ret = 0;
                }
                else if (options[i].copy(&options[i],
                                         options[i].dval,
                                         options[i].dest) == false)
                {
                    fprintf(stderr,
                            "\ninvalid option '%s %s' (limits: [%s, %s])\n",
                            name,
                            options[i].dval,
                            options[i].minval,
                            options[i].maxval);
                    ret = 0;
                }
            }
        }
        else
        {
            if (options[i].oval == val_required)
            {
                fprintf(stderr,
                        "\nmissing value for option '%s' (limits: [%s, %s])\n",
                        argv[*argi],
                        options[i].minval,
                        options[i].maxval);
                ret = 0;
            }
            else if (options[i].copy(&options[i],
                                     options[i].dval,
                                     options[i].dest) == false)
            {
                fprintf(stderr,
                        "\ninvalid option '%s %s' (limits: [%s, %s])\n",
                        argv[*argi],
                        options[i].dval,
                        options[i].minval,
                        options[i].maxval);
                ret = 0;
            }
        }
    }

    return ret;
}

/**
 * @brief Initialze an arguments object with default values.
 *
 * @param[in,out] args A pointer to an arguments object.
 *
 * @return Void.
 */
static void args_init(struct args_obj * const args)
{
    uint32_t i;

    // Get system constants.
    utilstring_fromi32(SOMAXCONN, str_somaxconn, sizeof(str_somaxconn));
    utilstring_fromi32((int32_t)utilsysctl_getcpusavail(),
                       str_nproconln,
                       sizeof(str_nproconln));

    // Map argument fields to options array.
    args->mode = ARGS_MODE_PERF;
    args->family = AF_INET;
    options[utilmath_log2(ARGS_FLAG_AFFINITY)].dest = &args->affinity;
    options[utilmath_log2(ARGS_FLAG_BIND)].dest = &args->ipport;
    options[utilmath_log2(ARGS_FLAG_BANDWIDTH)].dest = &args->ratelimitbps;
    options[utilmath_log2(ARGS_FLAG_CLIENT)].dest = &args->ipaddr;
    args->arch = SOCKOBJ_MODEL_CLIENT;
    args->echo = false;
    options[utilmath_log2(ARGS_FLAG_INTERVAL)].dest = &args->interval;
    options[utilmath_log2(ARGS_FLAG_LEN)].dest = &args->buflen;
    args->opts.nodelay = true;
    options[utilmath_log2(ARGS_FLAG_NUM)].dest = &args->datalimitbyte;
    options[utilmath_log2(ARGS_FLAG_PARALLEL)].dest = &args->maxcon;
    options[utilmath_log2(ARGS_FLAG_PORT)].dest = &args->ipport;
    options[utilmath_log2(ARGS_FLAG_BACKLOG)].dest = &args->backlog;
    options[utilmath_log2(ARGS_FLAG_SERVER)].dest = &args->ipaddr;
    options[utilmath_log2(ARGS_FLAG_THREADS)].dest = &args->threads;
    options[utilmath_log2(ARGS_FLAG_TIME)].dest = &args->timelimitusec;
    args->type = SOCK_STREAM;

    // Copy default options to arguments object.
    for (i = 0; i < sizeof(options) / sizeof(options[0]); i++)
    {
        options[i].args = args;

        if (options[i].dest != NULL)
        {
            if (options[i].copy != NULL)
            {
                options[i].copy(&options[i],
                                options[i].dval,
                                options[i].dest);
            }

            logger_printf(LOGGER_LEVEL_INFO, "%s: %s=%" PRIu64 "\n",
                          __FUNCTION__,
                          options[i].lname,
                          *(uint64_t*)options[i].dest);
        }
    }

    options[utilmath_log2(ARGS_FLAG_TIME)].minval = "1us";
}

static bool args_validate(const char c,
                          const uint32_t pos,
                          const uint64_t flags,
                          struct args_obj * const args)
{
    bool ret = true;
    struct argsopt *opt = NULL;

    logger_printf(LOGGER_LEVEL_DEBUG,
                  "%s: validating argument (pos = %u, val=%c)\n",
                  __FUNCTION__,
                  pos,
                  (!isalnum((int32_t)c) ? '0' + c : c));

    switch (c)
    {
        case 1:
            if (pos == 1)
            {
                args->mode = ARGS_MODE_CHAT;
            }
            else
            {
                args_usage(stdout);
                ret = false;
            }
            break;
        case 2:
            if (pos == 1)
            {
                args->mode = ARGS_MODE_PERF;
            }
            else
            {
                args_usage(stdout);
                ret = false;
            }
            break;
        case '4':
// if ip address flag already set and AF_INET != args->family, then we have a problem!
            args->family = AF_INET;
            utilinet_getaddrfromhost(args->ipaddr,
                                     args->family,
                                     args->ipaddr,
                                     sizeof(args->ipaddr));
            break;
        case '6':
            args->family = AF_INET6;
            utilinet_getaddrfromhost(args->ipaddr,
                                     args->family,
                                     args->ipaddr,
                                     sizeof(args->ipaddr));
            break;
        case 'A':
            break;
        case 'B':
            break;
        case 'b':
            break;
        case 'c':
            args->arch = SOCKOBJ_MODEL_CLIENT;
            break;
        case 'e':
            args->echo = true;
            break;
        case 'l':
            break;
        case 'N':
            args->opts.nodelay = true;
            break;
        case 'n':
            break;
        case 's':
            args->arch = SOCKOBJ_MODEL_SERVER;
            if ((flags & ARGS_FLAG_NUM) == 0)
            {
                args->datalimitbyte = 0;
            }
            if ((flags & ARGS_FLAG_PARALLEL) == 0)
            {
                args->maxcon = 0;
            }
            break;
        case 'P':
            if ((flags & ARGS_FLAG_BACKLOG) == 0)
            {
                args->backlog = args->maxcon;
            }
            break;
        case 'p':
            break;
        case 'q':
            break;
        case 'T':
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
                // @todo server should accept at any rate by default.
                opt = &options[utilmath_log2(ARGS_FLAG_BANDWIDTH)];
                opt->copy(opt, "1Mbps", opt->dest);
            }
            if ((flags & ARGS_FLAG_LEN) == 0)
            {
                // @todo what about default mtu?
                opt = &options[utilmath_log2(ARGS_FLAG_LEN)];
                opt->copy(opt, "1kB", opt->dest);
            }
            break;
        case 'h':
            args_usage(stdout);
            ret = false;
            break;
        case 'v':
            fprintf(stdout,
                    "bottlerocket version %u.%u.%u (%s)\n",
                    version_major(),
                    version_minor(),
                    version_patch(),
                    version_date());
            ret = false;
            break;
        default:
            args_usage(stdout);
            ret = false;
            break;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool args_parse(const int32_t argc,
                char ** const argv,
                struct args_obj * const args)
{
    bool ret = false;
    uint64_t flags = 0;
    int32_t i;

    if (UTILDEBUG_VERIFY((argv != NULL) && (args != NULL)) == true)
    {
        if (argc > 1)
        {
            args_init(args);
            ret = true;

            // @todo Validate a second time in order to catch options with
            //       unordered dependencies (i.e., bottlerocket -s ::1 -6)?
            for (i = 1; (i < argc) && (ret == true); i++)
            {
                ret = args_validate(args_getarg(argc, argv, &i, &flags),
                                    i,
                                    flags,
                                    args);
            }
        }
        else
        {
            args_usage(stdout);
        }
    }

    return ret;
}
