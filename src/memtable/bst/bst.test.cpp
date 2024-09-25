
#include <gtest/gtest.h>

#include <optional>
#include <vector>
#include <utility>

#define private public
#include "bst.h"


using namespace std;

TEST(BstTest, Initialization) {
    Bst b(5);
    EXPECT_EQ(b.max_size, 5);
    EXPECT_EQ(b.c_size, 0);
}

TEST(BstTest, Put) {
    Bst b(3);
    EXPECT_EQ(b.put(1, 1), EXIT_SUCCESS);
    EXPECT_EQ(b.c_size, 1);

    EXPECT_EQ(b.put(2, 1), EXIT_SUCCESS);
    EXPECT_EQ(b.c_size, 2);

    EXPECT_EQ(b.put(3, 1), EXIT_SUCCESS);
    EXPECT_EQ(b.c_size, 3);

    EXPECT_EQ(b.put(4, 1), EXIT_FAILURE);
    EXPECT_EQ(b.c_size, 3);

    EXPECT_EQ(b.put(5, 1), EXIT_FAILURE);
    EXPECT_EQ(b.c_size, 3);
}

TEST(BstTest, Get) {
    Bst b(3);
    EXPECT_EQ(b.get(1), std::nullopt);

    b.put(1, 2);
    EXPECT_EQ(b.get(1), 2);

    b.put(2, 3);
    EXPECT_EQ(b.get(2), 3);

    b.put(3, 4);
    EXPECT_EQ(b.get(3), 4);

    b.put(4, 5);
    EXPECT_EQ(b.get(4), std::nullopt);
}

TEST(BstTest, InOrder) {
    Bst b(5);
    b.put(1, 1);
    b.put(5, 5);
    b.put(2, 2);
    b.put(4, 4);
    b.put(3, 3);
    vector<pair<int, int>> vec = {
        {1, 1},
        {2, 2},
        {3, 3},
        {4, 4},
        {5, 5},
    };
    EXPECT_EQ(b.inOrder(), vec);
}

TEST(BstTest, Clear) {
    Bst b(5);
    b.put(1, 1);
    b.put(5, 5);
    b.put(2, 2);
    b.put(4, 4);
    b.put(3, 3);
    EXPECT_EQ(b.c_size, 5);
    b.clear();
    EXPECT_EQ(b.c_size, 0);
    EXPECT_EQ(b.root, nullptr);
}