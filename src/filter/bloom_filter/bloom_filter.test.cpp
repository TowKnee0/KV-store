#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cmath>
#define private public
#include "bloom_filter.h"
using namespace testing;

TEST(bloom_filter_tests, TEST_ADD){
    BloomFilter bloomFilter(10);
    bloomFilter.addKey(1);
    EXPECT_TRUE(bloomFilter.hasKey(1));
}

TEST(bloom_filter_tests, TEST_NO_FALSE_NEGATIVES){
    BloomFilter bloomFilter(10);
    for (int i = 0; i < 1000; i++){
        bloomFilter.addKey(i);
    }
    for (int i = 0; i < 1000; i++){
        EXPECT_TRUE(bloomFilter.hasKey(i));
    }
}

TEST(bloom_filter_tests, TEST_NUM_FUNCS){
    int bits = 10;
    BloomFilter bloomFilter(bits);
    int expected_funcs = ceil(bits * log(2));
    EXPECT_EQ(bloomFilter.numHashFunctions, expected_funcs);
}