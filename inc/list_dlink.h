/**
 * @file      list_dlink.h
 * @brief     Doubly linked list interface.
 * @author    Shane Barnes
 * @date      08 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _LIST_DLINK_H_
#define _LIST_DLINK_H_

#include "system_types.h"

struct listdlink_node
{
    void                  *val;
    struct listdlink_node *prev,
                          *next;
};

struct listdlink
{
    struct listdlink_node *head;
    struct listdlink_node *tail;
    uint32_t               size;
};

/**
 * @brief Insert a value at the beginning of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 * @param[in]     val  A value to add to the beginning of a doubly linked list.
 *
 * @return True if a value was inserted at the beginning of a doubly linked
 *         list.
 */
bool listdlink_inserthead(struct listdlink * const list, void * const val);

/**
 * @brief Insert a value at the end of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 * @param[in]     val  A value to insert at the end of a doubly linked list.
 *
 * @return True if a value was inserted at the end of a doubly linked list.
 */
bool listdlink_inserttail(struct listdlink * const list, void * const val);

/**
 * @brief Delete a value at the beginning of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 *
 * @return True if a value was deleted from the beginning of a doubly linked
 *         list.
 */
bool listdlink_deletehead(struct listdlink * const list);

/**
 * @brief Delete a value at the end of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 *
 * @return True if a value was deleted from the end of a doubly linked list.
 */
bool listdlink_deletetail(struct listdlink * const list);

#endif // _LIST_DLINK_H_
