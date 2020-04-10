#ifndef TST_IMAGECLASS_H
#define TST_IMAGECLASS_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

TEST(ImageClass, ImageClass)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}

#endif // TST_IMAGECLASS_H
