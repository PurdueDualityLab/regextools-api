//
// Created by charlie on 1/25/23.
//

#include <gtest/gtest.h>

#include <vector>

#include "librereuse/util/chunk.h"

TEST(Chunk, split_into_chunks) {
    std::vector<int> numbers {1, 2, 3, 4, 5, 6, 7, 8};

    auto chunks = rereuse::util::chunk(std::move(numbers), 2);
    std::vector<int> exp_1 {1, 2, 3, 4};
    std::vector<int> exp_2 {5, 6, 7, 8};
    EXPECT_EQ(chunks.size(), 2);
    EXPECT_EQ(chunks[0], exp_1);
    EXPECT_EQ(chunks[1], exp_2);
}

TEST(Chunk, split_into_chunks_2) {
    std::vector<int> numbers {1, 2, 3, 4, 5, 6, 7, 8};

    auto chunks = rereuse::util::chunk(std::move(numbers), 4);
    std::vector<int> exp_1 {1, 2};
    std::vector<int> exp_2 {3, 4};
    std::vector<int> exp_3 {5, 6};
    std::vector<int> exp_4 {7, 8};
    EXPECT_EQ(chunks.size(), 4);
    EXPECT_EQ(chunks[0], exp_1);
    EXPECT_EQ(chunks[1], exp_2);
    EXPECT_EQ(chunks[2], exp_3);
    EXPECT_EQ(chunks[3], exp_4);
}
