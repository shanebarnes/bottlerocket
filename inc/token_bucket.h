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
    uint64_t rate; // Bucket fill rate in tokens per second
    uint64_t size; // Bucket size in tokens
    uint64_t tsus; // Last fill Unix timestamp in microseconds
};

/**
 * @brief Initialize a token bucket structure.
 *
 * @param[in,out] tb   A pointer to a token bucket structure.
 * @param[in]     rate The data rate in tokens per second. The token bucket will
 *                     be unlimited if the data rate is zero.
 *
 * @return True if a token bucket structure was initialized.
 */
bool tokenbucket_init(struct tokenbucket * const tb, const uint64_t rate);

/**
 * @brief Request a number of tokens from the token bucket.
 *
 * @param[in,out] tb      A pointer to a token bucket structure.
 * @param[in]     gettoks The number of tokens requested.
 *
 * @return The number of tokens requested or zero if the number of available
 *         tokens in the bucket is less than the number of tokens requested.
 */
uint64_t tokenbucket_gettokens(struct tokenbucket * const tb,
                               const uint64_t gettoks);

#endif // _TOKEN_BUCKET_H_
