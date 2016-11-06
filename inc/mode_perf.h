/**
 * @file      mode_perf.h
 * @brief     Performance mode interface.
 * @author    Shane Barnes
 * @date      28 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _MODE_PERF_H_
#define _MODE_PERF_H_

#include "args.h"
#include "mode_obj.h"
#include "system_types.h"

/**
 * @see modeobj_create() for interface comments.
 */
bool modeperf_create(struct modeobj * const mode,
                     const struct args_obj * const args);

/**
 * @see modeobj_destroy() for interface comments.
 */
bool modeperf_destroy(struct modeobj * const mode);

/**
 * @see modeobj_start() for interface comments.
 */
bool modeperf_start(struct modeobj * const mode);

/**
 * @see modeobj_stop() for interface comments.
 */
bool modeperf_stop(struct modeobj * const mode);

/**
 * @see modeobj_cancel() for interface comments.
 */
bool modeperf_cancel(struct modeobj * const mode);

#endif // _MODE_PERF_H_
