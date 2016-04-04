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

bool modechat_create(const struct args_obj * const args);

bool modechat_start(void);

bool modechat_stop(void);

#endif // _MODE_CHAT_H_
