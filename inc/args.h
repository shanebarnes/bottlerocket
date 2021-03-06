/**
 * @file      args.h
 * @brief     Bottlerocket arguments interface.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _ARGS_H_
#define _ARGS_H_

#include "sock_obj.h"
#include "system_types.h"

#include <netinet/in.h>

enum args_mode
{
    ARGS_MODE_NULL = 0x00,
    ARGS_MODE_CHAT = 0x01,
    ARGS_MODE_PERF = 0x02,
    ARGS_MODE_REPT = 0x04
};

struct args_opts
{
    bool nodelay;
};

struct args_obj
{
    enum args_mode     mode;
    int32_t            family;
    uint16_t           affinity;
    uint64_t           ratelimitbps;
    char               ipaddr[INET6_ADDRSTRLEN];
    enum sockobj_model arch;
    bool               echo;
    uint64_t           intervalusec;
    uint64_t           buflen;
    struct args_opts   opts;
    uint64_t           datalimitbyte;
    uint32_t           maxcon;
    uint16_t           ipport;
    int32_t            backlog;
    uint32_t           threads;
    uint64_t           timelimitusec;
    int32_t            type;
    uint16_t           loglevel;
};

/**
 * @brief Parse and map command-line arguments into a bottlerocket argument
 *        structure.
 *
 * @param[in]     argc The command-line argument count.
 * @param[in]     argv The command-line argument vector.
 * @param[in,out] args A pointer to a bottlerocket arguments structure.
 *
 * @return True if a command-line arguments  were successfully parsed and
 *         mapped.
 */
bool args_parse(const int32_t argc,
                char ** const argv,
                struct args_obj * const args);

#endif // _ARGS_H_
