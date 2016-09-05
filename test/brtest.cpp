/**
 * @file      brtest.cpp
 * @brief     Bottlerocket Google unit tests.
 * @author    Shane Barnes
 * @date      05 Sep 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.c"
#include "mutex_obj.c"
#include "output_if_std.c"
#include "util_date.c"
#include "util_debug.c"
#include "util_string.c"
#include "vector.c"

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    int ret = 0;
    struct output_if_ops output_if;

    logger_create();
    output_if.oio_send = output_if_std_send;
    logger_set_output(&output_if);
    logger_set_level(LOGGER_LEVEL_OFF);

    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();

    logger_destroy();

    return ret;
}
