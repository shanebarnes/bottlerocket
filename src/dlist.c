/**
 * @file      dlist.c
 * @brief     Doubly linked list implementation.
 * @author    Shane Barnes
 * @date      08 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "dlist.h"
#include "logger.h"

#include <errno.h>

/**
 * @see See header file for interface comments.
 */
bool dlist_insertafter(struct dlist * const list,
                       struct dlist_node * const node,
                       void * const val)
{
    bool ret = false;
    struct dlist_node *new = NULL;

    if (list == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        new = malloc(sizeof(struct dlist_node));

        if (new == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate node (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            new->val = val;

            if (list->size == 0)
            {
                new->next  = new->prev  = NULL;
                list->head = list->tail = new;
            }
            else if (list->tail == node)
            {
                list->tail       = new;
                list->tail->next = NULL;
                list->tail->prev = node;
                node->next       = list->tail;
            }
            else
            {
                new->next = node->next;
                new->prev = node;

                (node->next)->prev = new;
                node->next         = new;
            }

            list->size++;
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool dlist_insertbefore(struct dlist * const list,
                        struct dlist_node * const node,
                        void * const val)
{
    bool ret = false;
    struct dlist_node *new = NULL;

    if (list == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        new = malloc(sizeof(struct dlist_node));

        if (new == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate node (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            new->val = val;

            if (list->size == 0)
            {
                new->next  = new->prev  = NULL;
                list->head = list->tail = new;
            }
            else if (list->head == node)
            {
                list->head       = new;
                list->head->next = node;
                list->head->prev = NULL;
                node->prev       = list->head;
            }
            else
            {
                new->next = node;
                new->prev = node->prev;

                (node->prev)->next = new;
                node->prev         = new;
            }

            list->size++;
            ret = true;
        }
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool dlist_inserthead(struct dlist * const list, void * const val)
{
    return dlist_insertbefore(list, list->head, val);
}

/**
 * @see See header file for interface comments.
 */
bool dlist_inserttail(struct dlist * const list, void * const val)
{
    return dlist_insertafter(list, list->tail, val);
}

/**
 * @see See header file for interface comments.
 */
bool dlist_remove(struct dlist * const list, struct dlist_node * const node)
{
    bool ret = false;

    if ((list == NULL) || (list->size == 0) || (node == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (list->size == 1)
        {
            list->head = list->tail = NULL;
        }
        else if (list->head == node)
        {
            list->head       = node->next;
            list->head->prev = NULL;
        }
        else if (list->tail == node)
        {
            list->tail       = node->prev;
            list->tail->next = NULL;
        }
        else
        {
            (node->next)->prev = node->prev;
            (node->prev)->next = node->next;
        }

        free(node);

        list->size--;
        ret = true;
    }

    return ret;
}

/**
 * @see See header file for interface comments.
 */
bool dlist_removehead(struct dlist * const list)
{
    return dlist_remove(list, list->head);
}

/**
 * @see See header file for interface comments.
 */
bool dlist_removetail(struct dlist * const list)
{
    return dlist_remove(list, list->tail);
}
