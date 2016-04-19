/**
 * @file      form_perf.h
 * @brief     Performance mode presentation layer format interface.
 * @author    Shane Barnes
 * @date      18 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _FORM_PERF_H_
#define _FORM_PERF_H_

#include "form_obj.h"
#include "system_types.h"

/**
 * @see form_head() for interface comments.
 */
int32_t formperf_head(struct formobj * const obj);

/**
 * @see form_body() for interface comments.
 */
int32_t formperf_body(struct formobj * const obj);

/**
 * @see form_foot() for interface comments.
 */
int32_t formperf_foot(struct formobj * const obj);

#endif // _FORM_PERF_H_
