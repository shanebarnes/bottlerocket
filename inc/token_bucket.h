/**
 * @file      token_bucket.h
 * @brief     Token bucket interface.
 * @author    Shane Barnes
 * @date      27 May 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _TOKEN_BUCKET_H_
#define _TOKEN_BUCKET_H_

#include "system_types.h"

struct tokenbucket
{
    uint64_t intrate; // data rate per interval (tokens per second)
    uint64_t inttoks; // tokens per interval
    uint64_t intusec; // microseconds per interval
    uint64_t size;    // bucket size
    uint64_t tsusec;  // unix timestamp in microseconds
};

/**
 * @brief Initialize a token bucket structure.
 *
 * @param[in,out] tb   A pointer to a token bucket structure.
 * @param[in]     rate The data rate in tokens per second.
 * @param[in]     burst The burst size in tokens.
 *
 * @return True if a token bucket structure was initialized.
 */
bool tokenbucket_init(struct tokenbucket * const tb,
                      const uint64_t rate,
                      const uint64_t burst);

#endif // _TOKEN_BUCKET_H_
