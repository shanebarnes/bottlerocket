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
 * @brief Initialize a token bucket.
 *
 * @param[in,out] tb   A pointer to a token bucket.
 * @param[in]     rate The token bucket fill rate in tokens per second. A value
 *                     of zero signifies an unlimited token bucket fill rate.
 *
 * @return True if a token bucket was initialized.
 */
bool tokenbucket_init(struct tokenbucket * const tb, const uint64_t rate);

/**
 * @brief Remove a number of tokens from a token bucket.
 *
 * @param[in,out] tb     A pointer to a token bucket.
 * @param[in]     tokens The number of tokens to remove from a token bucket.
 *
 * @return The number of tokens removed from a token bucket or zero if the
 *         number of tokens available in the bucket is less than the number of
 *         tokens requested for removal.
 */
uint64_t tokenbucket_remove(struct tokenbucket * const tb,
                            const uint64_t tokens);

/**
 * @brief Return a number of unused tokens to a token bucket.
 *
 * @param[in,out] tb     A pointer to a token bucket.
 * @param[in]     tokens The number of unused tokens to return to a token
 *                       bucket.
 *
 * @return The number of unused tokens returned to a token bucket.
 */
uint64_t tokenbucket_return(struct tokenbucket * const tb,
                            const uint64_t tokens);

/**
 * @brief Get the amount of delay in microseconds required before a minimum
 *        number of tokens will be available from a token bucket.
 *
 * @param[in,out] tb     A pointer to a token bucket.
 * @param[in]     tokens The minimum number of tokens that must be available
 *                       from a token bucket.
 *
 * @return The amount of delay in microseconds required before a token bucket
 *         will reach a certain size.
 */
uint64_t tokenbucket_delay(struct tokenbucket * const tb,
                           const uint64_t tokens);

#endif // _TOKEN_BUCKET_H_
