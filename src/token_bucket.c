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
#include "util_unit.h"

#include <string.h>

/**
 * @see See header file for interface comments.
 */
bool tokenbucket_init(struct tokenbucket * const tb,
                      const uint64_t rate,
                      const uint64_t burst)
{
    bool retval = false;
    uint32_t i = 0;

    if ((tb == NULL) || (burst == 0))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        memset(tb, 0, sizeof(struct tokenbucket));

        if (rate > 0)
        {
            do
            {
                tb->intusec = burst * ++i * 8 * UNIT_TIME_USEC / rate;
            }
            while (tb->intusec < 10000);

            tb->inttoks = burst * i;
        }
        else
        {
            tb->inttoks = burst;
        }

        tb->intrate = rate;

        retval = true;
    }

    return retval;
}
