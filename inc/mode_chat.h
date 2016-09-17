/**
 * @file      mode_chat.h
 * @brief     Chat mode interface.
 * @author    Shane Barnes
 * @date      02 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _MODE_CHAT_H_
#define _MODE_CHAT_H_

#include "args.h"
#include "system_types.h"

/**
 * @see modeobj_int() for interface comments.
 */
bool modechat_init(struct args_obj * const args);

/**
 * @see modeobj_start() for interface comments.
 */
bool modechat_start(void);

/**
 * @see modeobj_stop() for interface comments.
 */
bool modechat_stop(void);

/**
 * @see modeobj_cancel() for interface comments.
 */
bool modechat_cancel(void);

#endif // _MODE_CHAT_H_
