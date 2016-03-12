/**
 * @file   cli_options.h
 * @author Shane Barnes
 * @date   12 Mar 2016
 * @brief  Command-line interface options interface.
 */

#ifndef _CLI_OPTIONS_H_
#define _CLI_OPTIONS_H_

#include "system_types.h"

struct cli_options_instance
{
    bool exit;
};

/**
 * @brief Decode an array of CLI arguments.
 *
 * @param[in]     argc     The CLI argument count.
 * @param[in]     argv     The CLI argument array.
 * @param[in,out] instance A pointer to a CLI options instance.
 *
 * @return True if CLI argument array was successfully decoded.
 */
bool cli_options_decode(const int32_t argc,
                        char ** const argv,
                        struct cli_options_instance * const instance);

#endif // _CLI_OPTIONS_H_
