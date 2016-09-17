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
#include "system_types.h"

/**
 * @see modeobj_init() for interface comments.
 */
bool modeperf_init(struct args_obj * const args);

/**
 * @see modeobj_start() for interface comments.
 */
bool modeperf_start(void);

/**
 * @see modeobj_stop() for interface comments.
 */
bool modeperf_stop(void);

/**
 * @see modeobj_cancel() for interface comments.
 */
bool modeperf_cancel(void);

#endif // _MODE_PERF_H_
