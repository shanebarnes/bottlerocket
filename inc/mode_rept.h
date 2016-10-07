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
#include "system_types.h"

/**
 * @see modeobj_int() for interface comments.
 */
bool moderept_init(struct args_obj * const args);

/**
 * @see modeobj_start() for interface comments.
 */
bool moderept_start(void);

/**
 * @see modeobj_stop() for interface comments.
 */
bool moderept_stop(void);

/**
 * @see modeobj_cancel() for interface comments.
 */
bool moderept_cancel(void);

#endif // _MODE_REPT_H_
