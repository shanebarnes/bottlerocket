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
#include "mode_obj.h"
#include "system_types.h"

/**
 * @see modeobj_create() for interface comments.
 */
bool modechat_create(struct modeobj * const mode,
                     const struct args_obj * const args);

/**
 * @see modeobj_destroy() for interface comments.
 */
bool modechat_destroy(struct modeobj * const mode);

/**
 * @see modeobj_start() for interface comments.
 */
bool modechat_start(struct modeobj * const mode);

/**
 * @see modeobj_stop() for interface comments.
 */
bool modechat_stop(struct modeobj * const mode);

/**
 * @see modeobj_cancel() for interface comments.
 */
bool modechat_cancel(struct modeobj * const mode);

#endif // _MODE_CHAT_H_
