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
        tb->intrate = rate;
        tb->inttoks = rate;
        tb->intusec = UNIT_TIME_USEC;
        tb->tsusec  = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
        tb->size    = 0;

        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
uint64_t tokenbucket_gettokens(struct tokenbucket * const tb,
                               const uint64_t reqtoks)
{
    uint64_t retval = 0;
    uint64_t tsusec = 0;
    uint64_t newtoks = 0;

    if (tb == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (tb->intrate > 0)
        {
            tsusec = utildate_gettstime(DATE_CLOCK_MONOTONIC, UNIT_TIME_USEC);
            newtoks = tb->inttoks * (tsusec - tb->tsusec) / tb->intusec;

            if (newtoks > 0)
            {
                tb->size += newtoks;
                tb->tsusec = tsusec;
            }

            if (reqtoks > 0)
            {
                if (tb->size >= reqtoks)
                {
                    tb->size -= reqtoks;
                    retval    = reqtoks;
                }
            }
        }
        else
        {
            retval = reqtoks;
        }
    }

    return retval;
}
