/**
 * @file      token_bucket.c
 * @brief     Token bucket implementation.
 * @author    Shane Barnes
 * @date      27 May 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "token_bucket.h"
#include "util_date.h"
#include "util_unit.h"

/**
 * @see See header file for interface comments.
 */
bool tokenbucket_init(struct tokenbucket * const tb, const uint64_t rate)
{
    bool retval = false;

    if (tb == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        tb->rate = rate;
        tb->size = rate;
        tb->tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);

        retval = true;
    };

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t tokenbucket_remove(struct tokenbucket * const tb,
                            const uint64_t tokens)
{
    uint64_t retval = 0;
    uint64_t tsus = 0;
    uint64_t addtokens = 0;

    if (tb == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (tb->rate > 0)
        {
            tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
            addtokens = tb->rate * (tsus - tb->tsus) / UNIT_TIME_USEC;

            if (addtokens > 0)
            {
                tb->size += addtokens;
                tb->tsus  = tsus;
            }

            if (tokens > 0)
            {
                if (tb->size >= tokens)
                {
                    tb->size -= tokens;
                    retval    = tokens;
                }
            }
        }
        else
        {
            retval = tokens;
        }
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t tokenbucket_return(struct tokenbucket * const tb,
                            const uint64_t tokens)
{
    uint64_t retval = 0;

    if (tb == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (tb->rate > 0)
        {
            tb->size += tokens;
        }

        retval = tokens;
    }

    return retval;
}
