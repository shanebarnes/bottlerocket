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
 * @brief args A pointer to an arguments object.
 *
 * @return True if the chat mode of operation was run successfully.
 */
bool modechat_run(struct args_obj * const args);

#endif // _MODE_CHAT_H_
