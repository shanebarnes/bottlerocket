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
 * @brief Initialize performance mode arguments.
 *
 * @param[in] args A pointer to an arguments object..
 *
 * @return True if performance mode was initialized.
 */
bool modeperf_init(struct args_obj * const args);

/**
 * @brief Start performance mode.
 *
 * @return True if performance mode was started.
 */
bool modeperf_start(void);

/**
 * @brief Stop performance mode
 *
 * @return True if performance mode was stopped.
 */
bool modeperf_stop(void);

#endif // _MODE_PERF_H_
