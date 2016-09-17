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
 * @brief Initialize chat mode arguments.
 *
 * @param[in] args A pointer to an arguments object..
 *
 * @return True if chat mode was initialized.
 */
bool modechat_init(struct args_obj * const args);

/**
 * @brief Start chat mode.
 *
 * @return True if chat mode was started.
 */
bool modechat_start(void);

/**
 * @brief Stop the chat mode.
 *
 * @return True if chat mode was stopped.
 */
bool modechat_stop(void);

#endif // _MODE_CHAT_H_
