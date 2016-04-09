/**
 * @file      tree_bin.c
 * @brief     Binary search tree implementation.
 * @author    Shane Barnes
 * @date      26 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "tree_bin.h"

#include <errno.h>

/**
 * @see See header file for interface comments.
 */
bool treebin_insert(const struct treebin tree,
                    struct treebin_node **node,
                    void * const key)
{
    bool retval = false;

    if ((tree.compare == NULL) || (node == NULL) || (key == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (*node == NULL)
    {
        *node = (struct treebin_node *)malloc(sizeof(struct treebin_node));

        if (*node == NULL)
        {
            logger_printf(LOGGER_LEVEL_ERROR,
                          "%s: failed to allocate node (%d)\n",
                          __FUNCTION__,
                          errno);
        }
        else
        {
            (*node)->left = (*node)->right = NULL;
            (*node)->key = key;
            retval = true;
        }
    }
    else if (tree.compare(key, (*node)->key) < 0)
    {
        retval = treebin_insert(tree, &((*node)->left), key);
    }
    else if (tree.compare(key, (*node)->key) > 0)
    {
        retval = treebin_insert(tree, &((*node)->right), key);
    }
    else
    {
        (*node)->key = key; // Replace existing key/value.
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
bool treebin_delete(const struct treebin tree,
                    struct treebin_node **node,
                    void * const key)
{
    bool retval = false;

    if ((tree.compare == NULL) || (node == NULL) || (key == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (*node == NULL)
    {
        retval = true;
    }
    else
    {
        // @todo Remove node without removing children or sub-trees.
        treebin_delete(tree, &((*node)->left), key);
        treebin_delete(tree, &((*node)->right), key);
        free(*node);
        *node = NULL;
        retval = true;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
struct treebin_node *treebin_search(const struct treebin tree,
                                    struct treebin_node **node,
                                    void * const key)
{
    struct treebin_node *retval = NULL;

    if ((tree.compare == NULL) || (node == NULL) || (key == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else if (*node == NULL)
    {
        // The node has no children.
    }
    else if (tree.compare(key, (*node)->key) < 0)
    {
        retval = treebin_search(tree, &((*node)->left), key);
    }
    else if (tree.compare(key, (*node)->key) > 0)
    {
        retval = treebin_search(tree, &((*node)->right), key);
    }
    else
    {
        retval = (*node)->key;
    }

    return retval;
}

/**
 * @see See header file for interface comments.
 */
int32_t treebin_compareint32(void * const key1, void * const key2)
{
    int32_t retval = -1;
    int32_t k1 , k2;

    if ((key1 == NULL) || (key2 == NULL))
    {
        logger_printf(LOGGER_LEVEL_ERROR,
                      "%s: parameter validation failed\n",
                      __FUNCTION__);
    }
    else
    {
        k1 = *((int32_t *)(key1));
        k2 = *((int32_t *)(key2));

        if (k1 < k2)
        {
            retval = -1;
        }
        else if (k1 > k2)
        {
            retval = 1;
        }
        else
        {
            retval = 0;
        }
    }

    return retval;
}
