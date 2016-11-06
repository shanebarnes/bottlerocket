/**
 * @file      mode_rept.h
 * @brief     Repeater mode interface.
 * @author    Shane Barnes
 * @date      06 Oct 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _MODE_REPT_H_
#define _MODE_REPT_H_

#include "args.h"
#include "mode_obj.h"
#include "system_types.h"

/**
 * @see modeobj_create() for interface comments.
 */
bool moderept_create(struct modeobj * const mode,
                     const struct args_obj * const args);

/**
 * @see modeobj_destroy() for interface comments.
 */
bool moderept_destroy(struct modeobj * const mode);

/**
 * @see modeobj_start() for interface comments.
 */
bool moderept_start(struct modeobj * const mode);

/**
 * @see modeobj_stop() for interface comments.
 */
bool moderept_stop(struct modeobj * const mode);

/**
 * @see modeobj_cancel() for interface comments.
 */
bool moderept_cancel(struct modeobj * const mode);

#endif // _MODE_REPT_H_
