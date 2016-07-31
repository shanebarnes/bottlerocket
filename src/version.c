/**
 * @file      version.c
 * @brief     Version implementation.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "version.h"

static const uint16_t  major = 0;
static const uint16_t  minor = 2;
static const uint16_t  patch = 5;
static const char     *date  = "Jul 30 2016 20:59:00";
static const char     *build = __DATE__ " " __TIME__;

/**
 * @see See header file for interface comments.
 */
uint64_t version_number(void)
{
    return (uint64_t)(((uint64_t)major << 32) + ((uint64_t)minor << 16) + patch);
}

/**
 * @see See header file for interface comments.
 */
uint16_t version_major(void)
{
    return major;
}

/**
 * @see See header file for interface comments.
 */
uint16_t version_minor(void)
{
    return minor;
}

/**
 * @see See header file for interface comments.
 */
uint16_t version_patch(void)
{
    return patch;
}

/**
 * @see See header file for interface comments.
 */
const char * version_date(void)
{
    return date;
}

/**
 * @see See header file for interface comments.
 */
const char * version_build(void)
{
    return build;
}
