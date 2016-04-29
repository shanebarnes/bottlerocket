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

bool modeperf_create(struct args_obj * const args);

bool modeperf_start(void);

bool modeperf_stop(void);

#endif // _MODE_PERF_H_
