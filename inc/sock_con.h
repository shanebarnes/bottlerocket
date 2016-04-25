/**
 * @file      sock_con.h
 * @brief     Socket container interface.
 * @author    Shane Barnes
 * @date      23 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_CON_H_
#define _SOCK_CON_H_

#include "fion_obj.h"
#include "sock_obj.h"
#include "system_types.h"
#include "vector.h"

struct sockcon
{
    struct fionobj fion;
    struct vector  sock;
    int32_t        maxcon;
};

bool sockcon_create(struct sockcon * const con);

bool sockcon_destroy(struct sockcon * const con);

struct sockobj *sockcon_get(struct sockcon * const con, const uint32_t index);

bool sockcon_insert(struct sockcon * const con, struct sockobj * const obj);

bool sockcon_delete(struct sockcon * const con, const uint32_t index);

bool sockcon_poll(struct sockcon * const con);

#endif // _SOCK_CON_H_
