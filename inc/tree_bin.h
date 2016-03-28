/**
 * @file      tree_bin.h
 * @brief     Binary search tree interface.
 * @author    Shane Barnes
 * @date      26 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _TREE_BIN_H_
#define _TREE_BIN_H_

#include "system_types.h"

struct treebin_node
{
    void                *key;
    struct treebin_node *left,
                        *right;
};

struct treebin
{
    struct treebin_node *root;
    int32_t             (*compare)(void * const key1, void * const key2);
};

/**
 * @brief Insert a key into a binary tree.
 *
 * @param[in] tree A binary tree.
 * @param[in] node A pointer to a tree node pointer.
 * @param[in] key  A key to add to a binary tree.
 *
 * @return True if a key was added to a binary tree.
 */
bool treebin_insert(const struct treebin tree,
                    struct treebin_node **node,
                    void * const key);

/**
 * @brief Delete a key from a binary tree.
 *
 * @param[in] tree A binary tree.
 * @param[in] node A pointer to a tree node pointer.
 * @param[in] key  A key to delete from a binary tree.
 *
 * @return True if a key was deleted from a binary tree.
 */
bool treebin_delete(const struct treebin tree,
                    struct treebin_node **node,
                    void * const key);
/**
 * @brief Search for a key in a binary tree.
 *
 * @param[in] tree A binary tree.
 * @param[in] node A pointer to a tree node pointer.
 * @param[in] key  A key to find in a binary tree.
 *
 * @return A pointer to a tree node containing a key (and optional value) if a
 *         key was found (NULL on error).
 */
struct treebin_node *treebin_search(const struct treebin tree,
                                    struct treebin_node **node,
                                    void * const key);

/**
 * @brief A sample binary tree compare function for int32_t keys.
 *
 * @param[in] key1 A pointer to a first key.
 * @param[in] key2 A pointer to a second key.
 *
 * @return -1 if key1 < key2,
 *         +1 if key1 > key2, or
 *          0 if key1 == key2 (-1 on error).
 */
int32_t treebin_compareint32(void * const key1, void * const key2);

#endif // _TREE_BIN_H_
