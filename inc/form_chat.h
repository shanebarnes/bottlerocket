/**
 * @file      form_chat.h
 * @brief     Chat mode presentation layer format interface.
 * @author    Shane Barnes
 * @date      16 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _FORM_CHAT_H_
#define _FORM_CHAT_H_

#include "form_obj.h"
#include "system_types.h"

/**
 * @see form_create() for interface comments.
 */
bool formchat_create(struct formobj * const obj, const int32_t bufsize);

/**
 * @see form_head() for interface comments.
 */
int32_t formchat_head(struct formobj * const obj);

/**
 * @see form_body() for interface comments.
 */
int32_t formchat_body(struct formobj * const obj);

/**
 * @see form_foot() for interface comments.
 */
int32_t formchat_foot(struct formobj * const obj);

#endif // _FORM_CHAT_H_
