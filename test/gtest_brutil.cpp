/**
 * @file   gtest_brutil.c
 * @author Shane Barnes
 * @date   01 Mar 2016
 * @brief  Bottlerocket utility Google unit tests.
 */

#include "manip_string.c"

#include <gtest/gtest.h>

TEST (ManipStringTest, CompareEmpty)
{
    ASSERT_TRUE(manip_string_compare("", "", false));
    ASSERT_TRUE(manip_string_compare("", "", true));
    ASSERT_FALSE(manip_string_compare(NULL, "", false));
    ASSERT_FALSE(manip_string_compare("", NULL, false));
    ASSERT_FALSE(manip_string_compare(NULL, "", true));
    ASSERT_FALSE(manip_string_compare("", NULL, true));
    ASSERT_FALSE(manip_string_compare(NULL, NULL, false));
    ASSERT_FALSE(manip_string_compare(NULL, NULL, true));
}

TEST (ManipStringTest, CompareAlpha)
{
    ASSERT_TRUE(manip_string_compare("string", "string", false));
    ASSERT_TRUE(manip_string_compare("string", "string", true));
    ASSERT_FALSE(manip_string_compare("string", "STRING", false));
    ASSERT_TRUE(manip_string_compare("string", "STRING", true));
}

TEST (ManipStringTest, CompareNum)
{
    ASSERT_TRUE(manip_string_compare("123", "123", false));
    ASSERT_TRUE(manip_string_compare("123", "123", true));
}

TEST (ManipStringTest, CompareAlphaNum)
{
    ASSERT_TRUE(manip_string_compare("string123", "string123", false));
    ASSERT_TRUE(manip_string_compare("string123", "string123", true));
    ASSERT_FALSE(manip_string_compare("string123", "STRING123", false));
    ASSERT_TRUE(manip_string_compare("string123", "STRING123", true));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
