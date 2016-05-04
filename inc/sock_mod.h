/**
 * @file      sock_mod.h
 * @brief     Socket model interface.
 * @author    Shane Barnes
 * @date      03 May 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_MOD_H_
#define _SOCK_MOD_H_

#include "sock_obj.h"
#include "system_types.h"

/**
 * @brief Initialize a socket object as either a client or server model.
 *
 * @param[in,out] obj A pointer to a socket object to initialize.
 *
 * @return True on success.
 */
bool sockmod_init(struct sockobj * const obj);

#endif // _SOCK_MOD_H_
