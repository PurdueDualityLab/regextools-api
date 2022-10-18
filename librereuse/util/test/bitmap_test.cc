//
// Created by charlie on 10/18/22.
//

#include <gtest/gtest.h>
#include "librereuse/util/bitmap.h"

TEST(BitmapTest, correct_bitsets) {
    rereuse::util::Bitmap bm(4);
    EXPECT_EQ(bm.bitset_count(), 1);

    rereuse::util::Bitmap bm2(12);
    EXPECT_EQ(bm2.bitset_count(), 2);

    rereuse::util::Bitmap bm3(24);
    EXPECT_EQ(bm3.bitset_count(), 3);

    rereuse::util::Bitmap bm4(99);
    EXPECT_EQ(bm4.bitset_count(), 13);
}

TEST(BitmapTest, BitwiseAnd) {
    rereuse::util::Bitmap left(5);
    left.set(4, true); left.set(1, true); left.set(0, true);
    rereuse::util::Bitmap right(3);
    right.set(1, true); right.set(0, true);

    auto result = left & right;
    EXPECT_TRUE(result.get(0));
    EXPECT_TRUE(result.get(1));
    EXPECT_FALSE(result.get(2));
    EXPECT_FALSE(result.get(3));
    EXPECT_FALSE(result.get(4));
    EXPECT_FALSE(result.get(5));
    EXPECT_FALSE(result.get(6));
    EXPECT_FALSE(result.get(7));
}

TEST(BitmapTest, BitwiseAndEq) {
    rereuse::util::Bitmap left(5);
    left.set(4, true); left.set(1, true); left.set(0, true);
    rereuse::util::Bitmap right(3);
    right.set(1, true); right.set(0, true);

    left &= right;
    EXPECT_TRUE(left.get(0));
    EXPECT_TRUE(left.get(1));
    EXPECT_FALSE(left.get(2));
    EXPECT_FALSE(left.get(3));
    EXPECT_FALSE(left.get(4));
    EXPECT_FALSE(left.get(5));
    EXPECT_FALSE(left.get(6));
    EXPECT_FALSE(left.get(7));
}

TEST(BitmapTest, BitwiseOr) {
    rereuse::util::Bitmap left(5);
    left.set(4, true); left.set(1, true); left.set(0, true);
    rereuse::util::Bitmap right(3);
    right.set(1, true); right.set(0, true);

    auto result = left | right;
    EXPECT_TRUE(result.get(0));
    EXPECT_TRUE(result.get(1));
    EXPECT_FALSE(result.get(2));
    EXPECT_FALSE(result.get(3));
    EXPECT_TRUE(result.get(4));
    EXPECT_FALSE(result.get(5));
    EXPECT_FALSE(result.get(6));
    EXPECT_FALSE(result.get(7));
}
