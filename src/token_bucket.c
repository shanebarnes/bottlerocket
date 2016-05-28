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
        tb->size = 0;
        tb->tsus = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);

        retval = true;
    };

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t tokenbucket_gettokens(struct tokenbucket * const tb,
                               const uint64_t gettoks)
{
    uint64_t retval = 0;
    uint64_t tsus = 0;
    uint64_t addtoks = 0;

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
            addtoks = tb->rate * (tsus - tb->tsus) / UNIT_TIME_USEC;

            if (addtoks > 0)
            {
                tb->size += addtoks;
                tb->tsus  = tsus;
            }

            if (gettoks > 0)
            {
                if (tb->size >= gettoks)
                {
                    tb->size -= gettoks;
                    retval    = gettoks;
                }
            }
        }
        else
        {
            retval = gettoks;
        }
    }

    return retval;
}
