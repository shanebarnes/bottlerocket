/**
 * @file      mode_obj.h
 * @brief     Mode of operation object interface.
 * @author    Shane Barnes
 * @date      17 Sep 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _MODE_OBJ_H_
#define _MODE_OBJ_H_

#include "args.h"
#include "system_types.h"

struct modeobj;
struct modeobj_priv;

struct modeobj_ops
{
    /**
     * @brief Create a mode object.
     *
     * @param[in,out] mode A pointer to a mode object.
     * @param[in]     args A pointer to an arguments object.
     *
     * @return True if a mode object was created.
     */
    bool (*mode_create)(struct modeobj * const mode,
                        const struct args_obj * const args);

    /**
     * brief Destroy a mode object.
     *
     * @param[in,out] mode A pointer to a mode object.
     *
     * @return True if a mode object was destroyed.
     */
    bool (*mode_destroy)(struct modeobj * const mode);

    /**
     * @brief Initialize a mode of operation.
     *
     * @param[in] args A pointer to an arguments object.
     *
     * @return True if mode of operation was initialized.
     */
    bool (*mode_init)(struct args_obj * const args);

    /**
     * @brief Start a mode of operation.
     *
     * @param[in,out] mode A pointer to a mode object.
     *
     * @return True if mode of operation was started.
     */
    bool (*mode_start)(struct modeobj * const mode);

    /**
     * @brief Stop a mode of operation.
     *
     * @param[in,out] mode A pointer to a mode object.
     *
     * @return True if mode of operation was stopped.
     */
    bool (*mode_stop)(struct modeobj * const mode);

    /**
     * @brief Cancel a mode of operation (i.e., unblock the mode_start).
     *
     * @param[in,out] mode A pointer to a mode object.
     *
     * @return True if mode of operation was canceled.
     */
    bool (*mode_cancel)(struct modeobj * const mode);
};

struct modeobj
{
    struct modeobj_ops   ops;
    struct modeobj_priv *priv;
};

#endif // _MODE_OBJ_H_
