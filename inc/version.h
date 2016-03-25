/**
 * @file      version.h
 * @brief     Version interface.
 * @author    Shane Barnes
 * @date      12 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _VERSION_H_
#define _VERSION_H_

#include "system_types.h"

/**
 * @brief Get the version number of the form X.Y.Z, where
 *          X = major version,
 *          Y = minor version, and
 *          Z = patch version.
 *        A version number in this form is useful for numerical comparison.
 *
 * @return The version release number.
 */
uint64_t version_number(void);

/**
 * @brief Get the major version number.
 *
 * @return The major version number.
 */
uint16_t version_major(void);

/**
 * @brief Get the minor version number.
 *
 * @return The minor version number.
 */
uint16_t version_minor(void);

/**
 * @brief Get the patch version number.
 *
 * @return The patch version number.
 */
uint16_t version_patch(void);

/**
 * @brief Get the version date.
 *
 * @return The version date.
 */
const char * version_date(void);

/**
 * @brief Get the version build date.
 *
 * @return The version build date.
 */
const char * version_build(void);

#endif // _VERSION_H_
