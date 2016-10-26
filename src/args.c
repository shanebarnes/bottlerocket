/**
 * @file      args.c
 * @brief     Bottlerocket arguments implementation.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "arg_obj.h"
#include "args.h"
#include "logger.h"
#include "util_debug.h"
#include "util_inet.h"
#include "util_math.h"
#include "util_string.h"
#include "util_sysctl.h"
#include "version.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ARG_ACTIVE   true
#define ARG_INACTIVE false
#define arg_noobjptr NULL
#define arg_optional true
#define arg_required false
#define val_optional true
#define val_required false

struct argsmap
{
    uint64_t  keys;
    char     *key[64];
    char     *vals[64];
    bool      val[64];
};

enum argsobj_flag
{
    ARGS_FLAG_NULL       = 0LL,
    ARGS_FLAG_CHAT       = 1LL << ('0' - '0' +  1),
    ARGS_FLAG_PERF       = 1LL << ('1' - '0' +  1),
    ARGS_FLAG_REPT       = 1LL << ('2' - '0' +  1),
    ARGS_FLAG_IPV4       = 1LL << ('4' - '0' +  1),
    ARGS_FLAG_IPV6       = 1LL << ('6' - '0' +  1),
    ARGS_FLAG_AFFINITY   = 1LL << ('A' - 'A' + 11),
    ARGS_FLAG_BIND       = 1LL << ('B' - 'A' + 11),
    ARGS_FLAG_BANDWIDTH  = 1LL << ('b' - 'a' + 37),
    ARGS_FLAG_CLIENT     = 1LL << ('c' - 'a' + 37),
    ARGS_FLAG_ECHO       = 1LL << ('e' - 'a' + 37),
    ARGS_FLAG_INTERVAL   = 1LL << ('i' - 'a' + 37),
    ARGS_FLAG_LEN        = 1LL << ('l' - 'a' + 37),
    ARGS_FLAG_OPTNODELAY = 1LL << ('N' - 'A' + 11),
    ARGS_FLAG_NUM        = 1LL << ('n' - 'a' + 37),
    ARGS_FLAG_PARALLEL   = 1LL << ('P' - 'A' + 11),
    ARGS_FLAG_PORT       = 1LL << ('p' - 'a' + 37),
    ARGS_FLAG_BACKLOG    = 1LL << ('q' - 'a' + 37),
    ARGS_FLAG_SERVER     = 1LL << ('s' - 'a' + 37),
    ARGS_FLAG_THREADS    = 1LL << ('T' - 'A' + 11),
    ARGS_FLAG_TIME       = 1LL << ('t' - 'a' + 37),
    ARGS_FLAG_UDP        = 1LL << ('u' - 'a' + 37),
    ARGS_FLAG_HELP       = 1LL << ('h' - 'a' + 37),
    ARGS_FLAG_VERSION    = 1LL << ('v' - 'a' + 37)
};

static char        str_somaxconn[16];
static char        str_nproconln[16];
static const char *prefix_skey = "-";
static const char *prefix_lkey = "--";

static struct argobj options[] =
{
    {
        ARG_INACTIVE,
        "",
        '\0',
        NULL,
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
        ARG_ACTIVE,
        "chat",
        '0',
        "enable chat mode",
        "disabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_PERF | ARGS_FLAG_REPT,
        arg_noobjptr,
        NULL,
        NULL
    },
    {
        ARG_ACTIVE,
        "perf",
        '1',
        "enable performance benchmarking mode",
        "enabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_CHAT | ARGS_FLAG_REPT,
        arg_noobjptr,
        NULL,
        NULL
    },
    {
        ARG_ACTIVE,
        "rept",
        '2',
        "enable repeater mode",
        "disabled",
        NULL,
        NULL,
        val_optional,
        arg_optional,
        ARGS_FLAG_CHAT | ARGS_FLAG_PERF,
        arg_noobjptr,
        NULL,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        '3',
        NULL,
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
        ARG_ACTIVE,
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
        ARG_INACTIVE,
        "",
        '5',
        NULL,
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
        ARG_ACTIVE,
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
        ARG_INACTIVE,
        "",
        '7',
        NULL,
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
        ARG_INACTIVE,
        "",
        '8',
        NULL,
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
        ARG_INACTIVE,
        "",
        '9',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copyuint16,
        NULL
    },
    {
        ARG_ACTIVE,
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
        argobj_copyuint16,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'C',
        NULL,
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
        ARG_INACTIVE,
        "",
        'D',
        NULL,
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
        ARG_INACTIVE,
        "",
        'E',
        NULL,
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
        ARG_INACTIVE,
        "",
        'F',
        NULL,
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
        ARG_INACTIVE,
        "",
        'G',
        NULL,
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
        ARG_INACTIVE,
        "",
        'H',
        NULL,
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
        ARG_INACTIVE,
        "",
        'I',
        NULL,
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
        ARG_INACTIVE,
        "",
        'J',
        NULL,
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
        ARG_INACTIVE,
        "",
        'K',
        NULL,
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
        ARG_INACTIVE,
        "",
        'L',
        NULL,
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
        ARG_INACTIVE,
        "",
        'M',
        NULL,
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
        ARG_ACTIVE,
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
        ARG_INACTIVE,
        "",
        'O',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copyuint32,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'Q',
        NULL,
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
        ARG_INACTIVE,
        "",
        'R',
        NULL,
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
        ARG_INACTIVE,
        "",
        'S',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copyuint32,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'U',
        NULL,
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
        ARG_INACTIVE,
        "",
        'V',
        NULL,
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
        ARG_INACTIVE,
        "",
        'W',
        NULL,
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
        ARG_INACTIVE,
        "",
        'X',
        NULL,
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
        ARG_INACTIVE,
        "",
        'Y',
        NULL,
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
        ARG_INACTIVE,
        "",
        'Z',
        NULL,
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
        ARG_INACTIVE,
        "",
        'a',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copyrateunit,
        NULL
    },
    {
        ARG_ACTIVE,
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
        argobj_copyipaddr,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'd',
        NULL,
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
        ARG_ACTIVE,
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
        ARG_INACTIVE,
        "",
        'f',
        NULL,
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
        ARG_INACTIVE,
        "",
        'g',
        NULL,
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
        ARG_ACTIVE,
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
        ARG_ACTIVE,
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
        argobj_copytimeunit,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'j',
        NULL,
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
        ARG_INACTIVE,
        "",
        'k',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copybyteunit,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'm',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copybyteunit,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'o',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copyuint16,
        NULL
    },
    {
        ARG_ACTIVE,
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
        argobj_copyint32,
        NULL
    },
    {
        ARG_INACTIVE,
        "",
        'r',
        NULL,
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
        ARG_ACTIVE,
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
        argobj_copyipaddr,
        NULL
    },
    {
        ARG_ACTIVE,
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
        argobj_copytimeunit,
        NULL
    },
    {
        ARG_ACTIVE,
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
        ARG_ACTIVE,
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
    },
    {
        ARG_INACTIVE,
        "",
        'w',
        NULL,
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
        ARG_INACTIVE,
        "",
        'x',
        NULL,
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
        ARG_INACTIVE,
        "",
        'y',
        NULL,
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
        ARG_INACTIVE,
        "",
        'z',
        NULL,
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

    for (i = 0; i < sizeof(options) / sizeof(struct argobj); i++)
    {
        if (options[i].status == ARG_ACTIVE)
        {
            fprintf(stream,
                    "  %s%c%s %-11s %-50s %s\n",
                    options[i].sname >= '4' ? prefix_skey : " ",
                    options[i].sname >= '4' ? options[i].sname : ' ',
                    options[i].sname >= '4' ? "," : " ",
                    options[i].lname,
                    options[i].desc,
                    options[i].dval == NULL ? "" : options[i].dval);
        }
    }

    fprintf(stream, "\n");
}

/**
 * @brief Get an argument (i.e., key-value pair) from an argument vector and map
 *        it to a bottlerocket argument element.
 *
 * @param[in]     argc  An argument count.
 * @param[in]     argv  An argument vector.
 * @param[in,out] argi  A pointer to an argument vector index.
 * @param[in,out] flags A pointer to argument flags.
 *
 * @return A flag representing the unique bottlerocket argument element
 *         (ARGS_FLAG_NULL on error).
 */
static bool args_getarg(const uint32_t argc,
                        char ** const argv,
                        struct argsmap * const map)
{
    bool ret = true;
    uint64_t flag = ARGS_FLAG_NULL;
    char *name = NULL;
    uint32_t i, j;
    char c;

    if (UTILDEBUG_VERIFY((argv != NULL) && (map != NULL)))
    {
        for (i = 1; ret && (i < argc); i++)
        {
            flag = ARGS_FLAG_NULL;
            name = NULL;

            for (j = 0; j < sizeof(options) / sizeof(struct argobj); j++)
            {
                if (utilstring_parse(argv[i], "-%c", &c) == 1)
                {
                    // Short names are case-sensitive.
                    if (c == options[j].sname)
                    {
                        flag = 1LL << j;
                        name = argv[i];
                        break;
                    }
                }

                // Long names are not-case sensitive.
                if (utilstring_compare(argv[i], options[j].lname, 0, true))
                {
                    flag = 1LL << j;
                    name = argv[i];
                    break;
                }
            }

            if (flag == ARGS_FLAG_NULL)
            {
                fprintf(stderr, "\nunknown option '%s'\n", name);
                ret = false;
            }
            else if ((map->keys & flag) || ((map->keys = map->keys | flag) == 0))
            {
                fprintf(stderr, "\nduplicate option '%s'\n", name);
                ret = false;
            }
            else if ((options[j].cflags & map->keys) != 0)
            {
                 fprintf(stderr, "\nincompatible option '%s'\n", name);
                 ret = false;
            }
            else if (options[j].dval == NULL)
            {
                // Do nothing.
            }
            else if (options[j].copy == NULL)
            {
                // Do nothing.
            }
            // If an argument attribute name was found, then check the next
            // array index for the argument attribute value if a value is
            // expected (i.e., default value is not null).
            else if ((i + 1) < argc)
            {
                // The argument attribute value cannot start with either a
                // short- or long-key prefix.
                if ((!utilstring_compare(argv[i+1],
                                         prefix_skey,
                                         strlen(prefix_skey), true)) &&
                    (!utilstring_compare(argv[i+1],
                                         prefix_lkey,
                                         strlen(prefix_lkey), true)))
                {
                    map->key[j] = name;
                    map->vals[j] = argv[i+1];
                    i++;
                }
                else
                {
                    if (options[j].oval == val_required)
                    {
                        fprintf(stderr,
                                "\nmissing value for option '%s' (limits: [%s, %s])\n",
                                name,
                                options[j].minval,
                                options[j].maxval);
                        ret = false;
                    }

                    map->key[j] = name;
                    map->vals[j] = NULL;
                }
            }
            else
            {
                if (options[j].oval == val_required)
                {
                    fprintf(stderr,
                            "\nmissing value for option '%s' (limits: [%s, %s])\n",
                            argv[i],
                            options[j].minval,
                            options[j].maxval);
                    ret = false;
                }
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

        if ((options[i].status == ARG_ACTIVE) && (options[i].dest != NULL))
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

static bool args_setarg(struct argobj * const arg, char * const val)
{
    bool ret = true;

    if ((val == NULL) || (arg->copy == NULL))
    {
        // Do nothing.
    }
    else if (!arg->copy(arg, val, arg->dest))
    {
        fprintf(stderr,
                "\ninvalid option '%s %s' (limits: [%s, %s])\n",
                arg->lname,
                val,
                arg->minval,
                arg->maxval);
        ret = false;
    }

    return ret;
}

static bool args_validate(struct argsmap * const map,
                          struct args_obj * const args)
{
    bool ret = (map->keys == ARGS_FLAG_NULL ? false : true);
    struct argobj *opt = NULL;
    uint32_t i;
    uint64_t key;

    for (i = 0; ret && (i < 64); i++)
    {
        key = 1LL << i;

        if (map->keys & key)
        {
            switch (key)
            {
                case ARGS_FLAG_CHAT:
                    args->mode = ARGS_MODE_CHAT;
                    break;
                case ARGS_FLAG_PERF:
                    args->mode = ARGS_MODE_PERF;
                    break;
                case ARGS_FLAG_REPT:
                    args->mode = ARGS_MODE_REPT;
                    break;
                case ARGS_FLAG_IPV4:
                    // if ip address flag already set and AF_INET != args->family, then we have a problem!
                    args->family = AF_INET;
                    utilinet_getaddrfromhost(args->ipaddr,
                                             args->family,
                                             args->ipaddr,
                                             sizeof(args->ipaddr));
                    break;
                case ARGS_FLAG_IPV6:
                    args->family = AF_INET6;
                    utilinet_getaddrfromhost(args->ipaddr,
                                             args->family,
                                             args->ipaddr,
                                             sizeof(args->ipaddr));
                    break;
                case ARGS_FLAG_AFFINITY:
                    break;
                case ARGS_FLAG_BIND:
                    break;
                case ARGS_FLAG_BANDWIDTH:
                    break;
                case ARGS_FLAG_CLIENT:
                    args->arch = SOCKOBJ_MODEL_CLIENT;
                    break;
                case ARGS_FLAG_ECHO:
                    args->echo = true;
                    break;
                case ARGS_FLAG_LEN:
                    break;
                case ARGS_FLAG_OPTNODELAY:
                    args->opts.nodelay = true;
                    break;
                case ARGS_FLAG_NUM:
                    break;
                case ARGS_FLAG_SERVER:
                    args->arch = SOCKOBJ_MODEL_SERVER;
                    if ((map->keys & ARGS_FLAG_BANDWIDTH) == 0)
                    {
                        opt = &options[utilmath_log2(ARGS_FLAG_BANDWIDTH)];
                        opt->copy(opt, "0bps", opt->dest);
                    }
                    if ((map->keys & ARGS_FLAG_NUM) == 0)
                    {
                        args->datalimitbyte = 0;
                    }
                    if ((map->keys & ARGS_FLAG_PARALLEL) == 0)
                    {
                        args->maxcon = 0;
                    }
                    break;
                case ARGS_FLAG_PARALLEL:
                    if ((map->keys & ARGS_FLAG_BACKLOG) == 0)
                    {
                        args->backlog = args->maxcon;
                    }
                    break;
                case ARGS_FLAG_PORT:
                    break;
                case ARGS_FLAG_BACKLOG:
                    break;
                case ARGS_FLAG_THREADS:
                    break;
                case ARGS_FLAG_TIME:
                    if ((map->keys & ARGS_FLAG_NUM) == 0)
                    {
                        args->datalimitbyte = 0;
                    }
                    break;
                case ARGS_FLAG_UDP:
                    args->type = SOCK_DGRAM;
                    if ((map->keys & ARGS_FLAG_BANDWIDTH) == 0)
                    {
                        opt = &options[utilmath_log2(ARGS_FLAG_BANDWIDTH)];
                        opt->copy(opt, "1Mbps", opt->dest);
                    }
                    if ((map->keys & ARGS_FLAG_LEN) == 0)
                    {
                        // @todo what about default mtu?
                        opt = &options[utilmath_log2(ARGS_FLAG_LEN)];
                        opt->copy(opt, "1kB", opt->dest);
                    }
                    break;
                case ARGS_FLAG_HELP:
                    args_usage(stdout);
                    ret = false;
                    break;
                case ARGS_FLAG_VERSION:
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

            if (ret)
            {
                ret = args_setarg(&options[i], map->vals[i]);
            }
        }
    }

    return ret;
}

bool args_parse(const int32_t argc,
                char ** const argv,
                struct args_obj * const args)
{
    bool ret = false;
    struct argsmap map;

    if (UTILDEBUG_VERIFY((argv != NULL) && (args != NULL)))
    {
        memset(&map, 0, sizeof(map));
        args_init(args);

        if ((args_getarg(argc, argv, &map)) && (args_validate(&map, args)))
        {
            ret = true;
        }
    }

    return ret;
}
