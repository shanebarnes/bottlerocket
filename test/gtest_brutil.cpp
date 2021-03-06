/**
 * @file      gtest_brutil.cpp
 * @brief     Bottlerocket utility Google unit tests.
 * @author    Shane Barnes
 * @date      18 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.h"
#include "mutex_obj.h"
#include "output_if_std.h"
#include "util_date.h"
#include "util_string.h"

#include <gtest/gtest.h>

TEST (ManipStringTest, Compare)
{
    // Empty strings or null pointers.
    ASSERT_TRUE(utilstring_compare("", "", 0, false));
    ASSERT_TRUE(utilstring_compare("", "", 0, true));
    ASSERT_FALSE(utilstring_compare(NULL, "", 0, false));
    ASSERT_FALSE(utilstring_compare("", NULL, 0, false));
    ASSERT_FALSE(utilstring_compare(NULL, "", 0, true));
    ASSERT_FALSE(utilstring_compare("", NULL, 0, true));
    ASSERT_FALSE(utilstring_compare(NULL, NULL, 0, false));
    ASSERT_FALSE(utilstring_compare(NULL, NULL, 0, true));

    // Strings containing only alphabetic characters.
    ASSERT_TRUE(utilstring_compare("string", "string", 0, false));
    ASSERT_TRUE(utilstring_compare("string", "string", 0, true));
    ASSERT_FALSE(utilstring_compare("string", "STRING", 0, false));
    ASSERT_TRUE(utilstring_compare("string", "STRING", 0, true));

    // Strings containing only numeric characters.
    ASSERT_TRUE(utilstring_compare("123", "123", 0, false));
    ASSERT_TRUE(utilstring_compare("123", "123", 0, true));

    // String containing alphanumeric characters.
    ASSERT_TRUE(utilstring_compare("string123", "string123", 0, false));
    ASSERT_TRUE(utilstring_compare("string123", "string123", 0, true));
    ASSERT_FALSE(utilstring_compare("string123", "STRING123", 0, false));
    ASSERT_TRUE(utilstring_compare("string123", "STRING123", 0, true));
}

TEST (ManipStringTest, Concat)
{
    char buf[16];
    const size_t bufsize = sizeof(buf);
    const char *str = "string";
    const int32_t strsize = (int32_t)strlen(str);

    // Empty buffer, empty strings, or null pointers.
    ASSERT_EQ(-1, utilstring_concat(buf, 0, NULL, NULL));
    ASSERT_EQ(-1, utilstring_concat(buf, 0, "%s", NULL));
    ASSERT_EQ(-1, utilstring_concat(buf, 0, NULL, ""));
    ASSERT_EQ(-1, utilstring_concat(buf, 0, NULL, str));
    ASSERT_EQ(-1, utilstring_concat(buf, 0, "", ""));
    ASSERT_EQ(-1, utilstring_concat(buf, 0, "%s", str));
    ASSERT_EQ(-1, utilstring_concat(NULL, 0, "%s", str));
    ASSERT_EQ(6, utilstring_concat(buf, bufsize, "%s", NULL)); // ??

    // Single- and multiple-variable concatenations.
    ASSERT_EQ(0, utilstring_concat(buf, bufsize, "%s", ""));
    ASSERT_EQ(1, utilstring_concat(buf, bufsize, "%d", 0));
    ASSERT_EQ(2, utilstring_concat(buf, bufsize, "%d", -1));
    ASSERT_EQ(2, utilstring_concat(buf, bufsize, "%d", 10));
    ASSERT_EQ(strsize, utilstring_concat(buf, bufsize, "%s", str));
    ASSERT_EQ(strsize + 1, utilstring_concat(buf, bufsize, "%s%d", str, 1));

    // Buffer overflow.
    ASSERT_EQ(strsize, utilstring_concat(buf, strsize, "%s", str));
    ASSERT_EQ(strsize - 1, utilstring_concat(buf, strsize - 1, "%s", str));
}
