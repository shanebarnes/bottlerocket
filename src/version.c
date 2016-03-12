/**
 * @file   version.c
 * @author Shane Barnes
 * @date   12 Mar 2016
 * @brief  Version implementation.
 */

#include "version.h"

static const uint16_t  major = 0;
static const uint16_t  minor = 0;
static const uint16_t  patch = 0;
static const char     *date  = "Mar 12 2016 14:10:00";
static const char     *build = __DATE__ " " __TIME__;

/**
 * @see See header file for interace comments.
 */
uint64_t version_number(void)
{
    return (uint64_t)(((uint64_t)major << 32) + ((uint64_t)minor << 16) + patch);
}

/**
 * @see See header file for interace comments.
 */
uint16_t version_major(void)
{
    return major;
}

/**
 * @see See header file for interace comments.
 */
uint16_t version_minor(void)
{
    return minor;
}

/**
 * @see See header file for interace comments.
 */
uint16_t version_patch(void)
{
    return patch;
}

/**
 * @see See header file for interace comments.
 */
const char * version_date(void)
{
    return date;
}

/**
 * @see See header file for interace comments.
 */
const char * version_build(void)
{
    return build;
}
