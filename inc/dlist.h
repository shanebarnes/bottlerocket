/**
 * @file      dlist.h
 * @brief     Doubly linked list interface.
 * @author    Shane Barnes
 * @date      08 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _DLIST_H_
#define _DLIST_H_

#include "system_types.h"

struct dlist_node
{
    void              *val;
    struct dlist_node *prev,
                      *next;
};

struct dlist
{
    struct dlist_node *head;
    struct dlist_node *tail;
    uint32_t           size;
};

/**
 * @brief Insert a value after a node in a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 * @param[in]     node A node in a doubly linked list after which to insert a
 *                     new value.
 * @param[in]     val  A value to add to a doubly linked list.
 *
 * @return True if a value was inserted after a node in a doubly linked list.
 */
bool dlist_insertafter(struct dlist * const list,
                       struct dlist_node * const node,
                       void * const val);

/**
 * @brief Insert a value before a node in a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 * @param[in]     node A node in a doubly linked list before which to insert a
 *                     new value.
 * @param[in]     val  A value to add to a doubly linked list.
 *
 * @return True if a value was inserted before a node in a doubly linked list.
 */
bool dlist_insertbefore(struct dlist * const list,
                        struct dlist_node * const node,
                        void * const val);

/**
 * @brief Insert a value at the beginning of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 * @param[in]     val  A value to add to the beginning of a doubly linked list.
 *
 * @return True if a value was inserted at the beginning of a doubly linked
 *         list.
 */
bool dlist_inserthead(struct dlist * const list, void * const val);

/**
 * @brief Insert a value at the end of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 * @param[in]     val  A value to insert at the end of a doubly linked list.
 *
 * @return True if a value was inserted at the end of a doubly linked list.
 */
bool dlist_inserttail(struct dlist * const list, void * const val);

/**
 * @brief Remove a node from a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 * @param[in]     node A node to remove from a doubly linked list.
 *
 * @return True if a node was removed from a doubly linked list.
 */
bool dlist_remove(struct dlist * const list, struct dlist_node * const node);

/**
 * @brief Remove a node at the beginning of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 *
 * @return True if a node was removed from the beginning of a doubly linked
 *         list.
 */
bool dlist_removehead(struct dlist * const list);

/**
 * @brief Remove a node at the end of a doubly linked list.
 *
 * @param[in,out] list A pointer to a doubly linked list.
 *
 * @return True if a node was removed from the end of a doubly linked list.
 */
bool dlist_removetail(struct dlist * const list);

#endif // _DLIST_H_
