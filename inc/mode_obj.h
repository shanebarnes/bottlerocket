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

struct modeobj_ops
{
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
     * @return True if mode of operation was started.
     */
    bool (*mode_start)(void);

    /**
     * @brief Stop a mode of operation.
     *
     * @return True if mode of operation was stopped.
     */
    bool (*mode_stop)(void);
};

struct modeobj
{
    struct modeobj_ops ops;
};

#endif // _MODE_OBJ_H_
