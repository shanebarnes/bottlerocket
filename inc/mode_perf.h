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
 * @brief args A pointer to an arguments object.
 *
 * @return True if the performance mode of operation was run successfully.
 */
bool modeperf_run(struct args_obj * const args);

#endif // _MODE_PERF_H_
