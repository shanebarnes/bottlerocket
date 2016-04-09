/**
 * @file      list_dlink.c
 * @brief     Doubly linked list implementation.
 * @author    Shane Barnes
 * @date      08 Apr 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "list_dlink.h"
#include "logger.h"

#include <errno.h>

/**
 * @see See header file for interace comments.
 */
bool listdlink_inserthead(struct listdlink * const list, void * const val)
{
    bool retval = false;
    struct listdlink_node *node = NULL;

    if ((list == NULL) || (val == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        node = (struct listdlink_node *)malloc(sizeof(struct listdlink_node));

        if (node == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate node (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            node->val = val;

            if (list->head == NULL)
            {
                node->next = NULL;
                node->prev = NULL;
                list->head = node;
                list->tail = node;
            }
            else
            {
                node->next       = list->head;
                node->prev       = NULL;
                list->head->prev = node;
                list->head       = node;
            }

            list->size++;
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool listdlink_inserttail(struct listdlink * const list, void * const val)
{
    bool retval = false;
    struct listdlink_node *node = NULL;

    if ((list == NULL) || (val == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        node = (struct listdlink_node *)malloc(sizeof(struct listdlink_node));

        if (node == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate node (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            node->val = val;

            if (list->tail == NULL)
            {
                node->next = NULL;
                node->prev = NULL;
                list->head = node;
                list->tail = node;
            }
            else
            {
                node->next       = NULL;
                node->prev       = list->tail;
                list->tail->next = node;
                list->tail       = node;
            }

            list->size++;
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool listdlink_deletehead(struct listdlink * const list)
{
    bool retval = false;

    if (list == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (list->head != NULL)
        {
            if (list->head->next == NULL)
            {
                free(list->head);
                list->head = NULL;
                list->tail = NULL;
            }
            else
            {
                list->head       = list->head->next;
                free(list->head->prev);
                list->head->prev = NULL;
            }

            list->size--;
            retval = true;
        }
    }

    return retval;
}

/**
 * @see See header file for interace comments.
 */
bool listdlink_deletetail(struct listdlink * const list)
{
    bool retval = false;

    if (list == NULL)
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        if (list->tail != NULL)
        {
            if (list->tail->prev == NULL)
            {
                free(list->tail);
                list->head = NULL;
                list->tail = NULL;
            }
            else
            {
                list->tail       = list->tail->prev;
                free(list->tail->next);
                list->tail->next = NULL;
            }

            list->size--;
            retval = true;
        }
    }

    return retval;
}
