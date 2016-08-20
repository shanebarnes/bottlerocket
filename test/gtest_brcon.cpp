/**
 * @file      gtest_brcon.cpp
 * @brief     Bottlerocket container Google unit tests.
 * @author    Shane Barnes
 * @date      19 Aug 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#include "logger.c"
#include "mutex_obj.c"
#include "util_date.c"
#include "util_string.c"
#include "vector.c"

#include <gtest/gtest.h>

TEST (VectorTest, Vector)
{
    struct vector vec = {0};
    uint32_t i = 0, size = 1024;

    ASSERT_FALSE(vector_create(&vec, 0, 0));
    ASSERT_FALSE(vector_create(NULL, 0, sizeof(uint32_t)));
    ASSERT_TRUE(vector_create(&vec, 0, sizeof(uint32_t)));
    ASSERT_FALSE(vector_create(&vec, 0, sizeof(uint32_t)));

    ASSERT_FALSE(vector_resize(NULL, 0));

    ASSERT_EQ(NULL, vector_getval(NULL, 0));
    ASSERT_EQ(NULL, vector_getval(&vec, 0));

    ASSERT_EQ(i, vector_getsize(NULL));
    ASSERT_EQ(i, vector_getsize(&vec));

    ASSERT_FALSE(vector_insert(NULL, 0, &i));
    ASSERT_FALSE(vector_insert(NULL, 0, NULL));
    ASSERT_FALSE(vector_insert(&vec, 0, NULL));
    ASSERT_FALSE(vector_insert(&vec, 0, &i));

    ASSERT_FALSE(vector_inserttail(NULL, &i));
    ASSERT_FALSE(vector_inserttail(&vec, NULL));
    ASSERT_FALSE(vector_inserttail(NULL, NULL));

    ASSERT_FALSE(vector_delete(NULL, 0));
    ASSERT_FALSE(vector_delete(&vec, 0));

    ASSERT_FALSE(vector_deletetail(NULL));
    ASSERT_FALSE(vector_deletetail(&vec));

    // Add elements to the vector where each element value is equal to the
    // vector position where it is stored.
    for (i = 0; i < size; i++)
    {
        ASSERT_TRUE(vector_inserttail(&vec, &i));
        ASSERT_EQ(i, *((uint32_t*)vector_getval(&vec, i)));
    }

    ASSERT_EQ(size, vector_getsize(&vec));

    // Remove vector members with odd values.
    for (i = vector_getsize(&vec) - 1; i > 0; i--)
    {
        if (i % 2 != 0)
        {
            ASSERT_TRUE(vector_delete(&vec, i));
        }
    }

    // Check remaining vector members for even values.
    for (i = 0; i < vector_getsize(&vec); i++)
    {
        ASSERT_EQ(i * 2, *((uint32_t*)vector_getval(&vec, i)));
    }

    ASSERT_TRUE(vector_resize(&vec, size / 4));
    ASSERT_EQ(size / 4, vector_getsize(&vec));

    // Remove all but the last vector members.
    while (vector_getsize(&vec) > 2)
    {
        ASSERT_TRUE(vector_deletetail(&vec));
    }

    ASSERT_FALSE(vector_destroy(NULL));
    ASSERT_TRUE(vector_destroy(&vec));
    ASSERT_FALSE(vector_destroy(&vec));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
