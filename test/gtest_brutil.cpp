/**
 * @file   gtest_brutil.cpp
 * @author Shane Barnes
 * @date   18 Mar 2016
 * @brief  Bottlerocket utility Google unit tests.
 */

#include "manip_string.c"

#include <gtest/gtest.h>

TEST (ManipStringTest, Compare)
{
    // Empty strings or null pointers.
    ASSERT_TRUE(manip_string_compare("", "", false));
    ASSERT_TRUE(manip_string_compare("", "", true));
    ASSERT_FALSE(manip_string_compare(NULL, "", false));
    ASSERT_FALSE(manip_string_compare("", NULL, false));
    ASSERT_FALSE(manip_string_compare(NULL, "", true));
    ASSERT_FALSE(manip_string_compare("", NULL, true));
    ASSERT_FALSE(manip_string_compare(NULL, NULL, false));
    ASSERT_FALSE(manip_string_compare(NULL, NULL, true));

    // Strings containing only alphabetic characters.
    ASSERT_TRUE(manip_string_compare("string", "string", false));
    ASSERT_TRUE(manip_string_compare("string", "string", true));
    ASSERT_FALSE(manip_string_compare("string", "STRING", false));
    ASSERT_TRUE(manip_string_compare("string", "STRING", true));

    // Strings containing only numeric characters.
    ASSERT_TRUE(manip_string_compare("123", "123", false));
    ASSERT_TRUE(manip_string_compare("123", "123", true));

    // String containing alphanumeric characters.
    ASSERT_TRUE(manip_string_compare("string123", "string123", false));
    ASSERT_TRUE(manip_string_compare("string123", "string123", true));
    ASSERT_FALSE(manip_string_compare("string123", "STRING123", false));
    ASSERT_TRUE(manip_string_compare("string123", "STRING123", true));
}

TEST (ManipStringTest, Concat)
{
    char buf[16];
    const size_t bufsize = sizeof(buf);
    const char *str = "string";
    const int32_t strsize = (int32_t)strlen(str);

    // Empty buffer, empty strings, or null pointers.
    ASSERT_EQ(-1, manip_string_concat(buf, 0, NULL, NULL));
    ASSERT_EQ(-1, manip_string_concat(buf, 0, "%s", NULL));
    ASSERT_EQ(-1, manip_string_concat(buf, 0, NULL, ""));
    ASSERT_EQ(-1, manip_string_concat(buf, 0, NULL, str));
    ASSERT_EQ(-1, manip_string_concat(buf, 0, "", ""));
    ASSERT_EQ(-1, manip_string_concat(buf, 0, "%s", str));
    ASSERT_EQ(-1, manip_string_concat(NULL, 0, "%s", str));
    ASSERT_EQ(6, manip_string_concat(buf, bufsize, "%s", NULL)); // ??

    // Single- and multiple-variable concatenations.
    ASSERT_EQ(0, manip_string_concat(buf, bufsize, "%s", ""));
    ASSERT_EQ(1, manip_string_concat(buf, bufsize, "%d", 0));
    ASSERT_EQ(2, manip_string_concat(buf, bufsize, "%d", -1));
    ASSERT_EQ(2, manip_string_concat(buf, bufsize, "%d", 10));
    ASSERT_EQ(strsize, manip_string_concat(buf, bufsize, "%s", str));
    ASSERT_EQ(strsize + 1, manip_string_concat(buf, bufsize, "%s%d", str, 1));

    // Buffer overflow.
    ASSERT_EQ(strsize, manip_string_concat(buf, strsize, "%s", str));
    ASSERT_EQ(strsize, manip_string_concat(buf, strsize - 1, "%s", str));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
