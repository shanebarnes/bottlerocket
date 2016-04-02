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

#include "system_types.h"

#include <netinet/in.h>

enum args_mode
{
    ARGS_MODE_PERF = 0x00,
    ARGS_MODE_CHAT = 0x01
};

enum args_arch
{
    ARGS_ARCH_CLIENT = 0x00,
    ARGS_ARCH_SERVER = 0x01,
    ARGS_ARCH_PEER2P = 0x02
};

struct args_obj
{
    enum args_mode mode;
    enum args_arch arch;
    char           ipaddr[INET6_ADDRSTRLEN];
    uint16_t       ipport;
};

/**
 * @brief Parse a one-dimensional argument vector into a bottlerocket argument
 *        structure.
 *
 * @param[in]     argc The CLI argument count.
 * @param[in]     argv The CLI argument vector.
 * @param[in,out] args A pointer to a bottlerocket arguments structure.
 *
 * @return True if an argument vector was successfully parsed.
 */
bool args_parse(const int32_t argc,
                char ** const argv,
                struct args_obj * const args);

#endif // _ARGS_H_
