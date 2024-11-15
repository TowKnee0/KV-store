#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../buffer/ext_hash/ext_hash.h"
#include "../../io/io.h"

#define private public
#include "btree.h"

using namespace testing;

// Create a mock for IOHandler
class MockIOHandler : public IO {
 public:
  MOCK_METHOD(int, writeToFile,
              (const string filename, (const vector<pair<int, int>> &data),
               int pageOffset));
  MOCK_METHOD((optional<vector<pair<int, int>>>), readPageFromFile,
              (const string filename, int pageNumber));
  MOCK_METHOD(int, numPagesInFile, (const string filename));
  MOCK_METHOD(vector<string>, getDbFilenames, ());
  MOCK_METHOD(int, getPageSize, ());
  MOCK_METHOD(int, getPairSize, ());
};

vector<pair<int, int>> expected_b_tree = {
    {1928, 7},  {4337, 7},  {4711, 7},  {4907, 7},  {5729, 7},  {5901, 7},  {7306, 7},  {8445, 7},  {8692, 7},  {13003, 7},
    {13153, 7}, {14787, 7}, {15251, 7}, {15668, 7}, {16201, 7}, {16250, 7}, {18292, 7}, {19592, 7}, {19762, 7}, {20404, 7},
    {21336, 7}, {21996, 7}, {23069, 7}, {23227, 7}, {24053, 7}, {24855, 7}, {25342, 7}, {26097, 7}, {27340, 7}, {27744, 7},
    {27811, 7}, {28740, 7}, {30116, 7}, {32497, 7}, {32952, 7}, {34035, 7}, {35523, 7}, {36108, 7}, {36830, 7}, {37254, 7},
    {37509, 7}, {37704, 7}, {38104, 7}, {39032, 7}, {39731, 7}, {40393, 7}, {40886, 7}, {42694, 7}, {43450, 7}, {44348, 7},
    {45443, 7}, {45613, 7}, {48474, 7}, {48669, 7}, {48911, 7}, {51270, 7}, {53209, 7}, {54334, 7}, {55640, 7}, {59105, 7},
    {59808, 7}, {61366, 7}, {62411, 7}, {63081, 7}, {64851, 7}, {68529, 7}, {68558, 7}, {70250, 7}, {70883, 7}, {71235, 7},
    {72561, 7}, {73671, 7}, {74208, 7}, {74298, 7}, {75478, 7}, {76509, 7}, {76628, 7}, {79776, 7}, {82843, 7}, {83553, 7},
    {84785, 7}, {84824, 7}, {84942, 7}, {85225, 7}, {85790, 7}, {88498, 7}, {89181, 7}, {89514, 7}, {89796, 7}, {90002, 7},
    {90061, 7}, {90818, 7}, {91226, 7}, {95409, 10}, {95549, 7}, {96389, 7}, {96724, 7}, {96998, 7}, {99048, 7}, {99792, 7},
    {BTree::INF, BTree::INF}, {BTree::INF, BTree::INF}, {4907, 7}, {7306, 7}, {13003, 7}, {16250, 7}, {19762, 7}, {21996, 7},
    {26097, 7}, {27811, 7}, {32497, 7}, {37254, 7}, {38104, 7}, {40393, 7}, {45613, 7}, {48911, 7}, {54334, 7}, {63081, 7},
    {68558, 7}, {71235, 7}, {76509, 7}, {82843, 7}, {84824, 7}, {89514, 7}, {90061, 7}, {95409, 10}, {99792, 7},
    {BTree::INF, BTree::INF}, {BTree::INF, BTree::INF}, {15251, 7}, {24053, 7}, {35523, 7}, {59808, 7}, {74208, 7}, {85790, 7},
    {BTree::INF, BTree::INF}, {BTree::INF, BTree::INF}, {BTree::INF, BTree::INF}, {43450, 7}, {96724, 7}, {BTree::INF, BTree::INF}
};

TEST(BTreeTest, WriteTest) {
  // Create a mock IOHandler
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  shared_ptr<ExtHash> buffer = make_shared<ExtHash>(5, 5, .8, 1, mockIOHandler);

  string filename = "test_btree";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .WillRepeatedly(Return(-1));

  BTree btree(filename, "test_db", mockIOHandler, buffer, false);

  // 100 ints
  vector<int> values = {
      1928,  4337,  4711,  4907,  5729,  5901,  7306,  8445,  8692,  13003,
      13153, 14787, 15251, 15668, 16201, 16250, 18292, 19592, 19762, 20404,
      21336, 21996, 23069, 23227, 24053, 24855, 25342, 26097, 27340, 27744,
      27811, 28740, 30116, 32497, 32952, 34035, 35523, 36108, 36830, 37254,
      37509, 37704, 38104, 39032, 39731, 40393, 40886, 42694, 43450, 44348,
      45443, 45613, 48474, 48669, 48911, 51270, 53209, 54334, 55640, 59105,
      59808, 61366, 62411, 63081, 64851, 68529, 68558, 70250, 70883, 71235,
      72561, 73671, 74208, 74298, 75478, 76509, 76628, 79776, 82843, 83553,
      84785, 84824, 84942, 85225, 85790, 88498, 89181, 89514, 89796, 90002,
      90061, 90818, 91226, 95409, 95549, 96389, 96724, 96998, 99048, 99792};

  // Create pairs with random second values
  vector<pair<int, int>> data;
  for (int value : values) {
    data.emplace_back(value, value == 95409 ? 10 : 7);
  }

  // Expect the writeToFile method to be called once
  EXPECT_CALL(*mockIOHandler, writeToFile(filename, data, 0))
      .Times(1)
      .WillOnce(Return(1));

  // Mock readPageFromFile calls to return slices of data
  for (int i = 0; i < ceil(double(100) / 3); ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] =
          i * 3 + j < 100 ? data[i * 3 + j] : make_pair(btree.INF, btree.INF);
    }

    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
  }

  vector<pair<int, int>> expected_internal_nodes(41);
  for (int i = 0; i < 39; i++) {
    expected_internal_nodes[i] = expected_b_tree[102 + i];
  }

  // Expect the writeToFile method to be called to write the internal
  // nodes
  EXPECT_CALL(*mockIOHandler, writeToFile(filename, _, 35))
      .Times(1)
      .WillRepeatedly(Return(0));

  // Expect the writeToFile method to be called to write metdata
  EXPECT_CALL(*mockIOHandler, writeToFile(filename, _, 48))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_EQ(btree.write(data), EXIT_SUCCESS);

  // Rerun the construct function to test that we get the desired output.
  vector<pair<int, int>> internal_nodes = btree.contructInternalNodes();
  for (int i = 0; i < internal_nodes.size(); i++) {
    EXPECT_EQ(internal_nodes[i].first, expected_internal_nodes[i].first);
    EXPECT_EQ(internal_nodes[i].second, expected_internal_nodes[i].second);
  }
}

TEST(BTreeTest, GetTest) {
  // Create a mock IOHandler
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  shared_ptr<ExtHash> buffer = make_shared<ExtHash>(5, 5, .8, 1, mockIOHandler);

  string filename = "test_btree";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  // B tree has 141 keys, 141/3 = 47 pages. Plus 1 for metadata.
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .WillRepeatedly(Return(48));
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 48))
      .WillRepeatedly(Return(
          make_optional<vector<pair<int, int>>>({{0, 3}, {1, 4}, {2, 100}})));

  BTree btree(filename, "test_db", mockIOHandler, buffer, false);

  // Mock readPageFromFile calls to return the pages
  for (int i = 0; i < ceil(static_cast<double>(expected_b_tree.size()) / 3);
       ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] = i * 3 + j < expected_b_tree.size()
                          ? expected_b_tree[i * 3 + j]
                          : make_pair(btree.INF, btree.INF);
    }

    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
  }

  // Call the get method
  optional<int> result = btree.get(1928);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 7);

  // Test getting 95409
  result = btree.get(95409);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);

  // Test getting a value not in the tree
  result = btree.get(74208 + 1);
  EXPECT_FALSE(result.has_value());
}

TEST(BTreeTest, GetTestBinarySearch) {
  // Create a mock IOHandler

  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  shared_ptr<ExtHash> buffer = make_shared<ExtHash>(5, 5, .8, 1, mockIOHandler);
  string filename = "test_btree";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  // B tree has 141 keys, 141/3 = 47 pages. Plus 1 for metadata.
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .WillRepeatedly(Return(48));
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 48))
      .WillRepeatedly(Return(
          make_optional<vector<pair<int, int>>>({{0, 3}, {1, 4}, {2, 100}})));

  BTree btree(filename, "test_db", mockIOHandler, buffer, true);

  // Mock readPageFromFile calls to return the pages
  for (int i = 0; i < ceil(static_cast<double>(expected_b_tree.size()) / 3);
       ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] = i * 3 + j < expected_b_tree.size()
                          ? expected_b_tree[i * 3 + j]
                          : make_pair(btree.INF, btree.INF);
    }

    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
  }

  // Call the get method
  optional<int> result = btree.get(1928);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 7);

  // Test getting 95409
  result = btree.get(95409);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);

  // Test getting a value not in the tree
  result = btree.get(74208 + 1);
  EXPECT_FALSE(result.has_value());
}

TEST(BTreeTest, ScanTest) {
  // Create a mock IOHandler
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  shared_ptr<ExtHash> buffer = make_shared<ExtHash>(5, 5, .8, 1, mockIOHandler);

  string filename = "test_btree";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  // B tree has 141 keys, 141/3 = 47 pages. Plus 1 for metadata.
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .WillRepeatedly(Return(48));
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 48))
      .WillRepeatedly(Return(
          make_optional<vector<pair<int, int>>>({{0, 3}, {1, 4}, {2, 100}})));

  BTree btree(filename, "test_db", mockIOHandler, buffer, false);

  // Mock readPageFromFile calls to return the pages
  for (int i = 0; i < ceil(static_cast<double>(expected_b_tree.size()) / 3);
       ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] = i * 3 + j < expected_b_tree.size()
                          ? expected_b_tree[i * 3 + j]
                          : make_pair(btree.INF, btree.INF);
    }

    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
  }

  // Call the scan method
  vector<pair<int, int>> result = btree.scan(1928, 15251);
  vector<pair<int, int>> expected_result = {
      {1928, 7},  {4337, 7},  {4711, 7}, {4907, 7}, {5729, 7},
      {5901, 7},  {7306, 7},  {8445, 7}, {8692, 7}, {13003, 7},
      {13153, 7}, {14787, 7}, {15251, 7}};

  // Includes endpoint not in tree
  result = btree.scan(89796, 99792 + 1);
  expected_result = {{89796, 7}, {90002, 7},  {90061, 7}, {90818, 7},
                     {91226, 7}, {95409, 10}, {95549, 7}, {96389, 7},
                     {96724, 7}, {96998, 7},  {99048, 7}, {99792, 7}};
  EXPECT_EQ(result, expected_result);

  // Include start point not in tree
  result = btree.scan(1928 - 100, 15251);
  expected_result = {{1928, 7},  {4337, 7},  {4711, 7}, {4907, 7}, {5729, 7},
                     {5901, 7},  {7306, 7},  {8445, 7}, {8692, 7}, {13003, 7},
                     {13153, 7}, {14787, 7}, {15251, 7}};
  EXPECT_EQ(result, expected_result);
}

TEST(BTreeTest, ScanTestBinarySearch) {
  // Create a mock IOHandler
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  shared_ptr<ExtHash> buffer = make_shared<ExtHash>(5, 5, .8, 1, mockIOHandler);

  string filename = "test_btree";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  // B tree has 141 keys, 141/3 = 47 pages. Plus 1 for metadata.
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .WillRepeatedly(Return(48));
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 48))
      .WillRepeatedly(Return(
          make_optional<vector<pair<int, int>>>({{0, 3}, {1, 4}, {2, 100}})));

  BTree btree(filename, "test_db", mockIOHandler, buffer, true);

  // Mock readPageFromFile calls to return the pages
  for (int i = 0; i < ceil(static_cast<double>(expected_b_tree.size()) / 3);
       ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] = i * 3 + j < expected_b_tree.size()
                          ? expected_b_tree[i * 3 + j]
                          : make_pair(btree.INF, btree.INF);
    }

    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
  }

  // Call the scan method
  vector<pair<int, int>> result = btree.scan(1928, 15251);
  vector<pair<int, int>> expected_result = {
      {1928, 7},  {4337, 7},  {4711, 7}, {4907, 7}, {5729, 7},
      {5901, 7},  {7306, 7},  {8445, 7}, {8692, 7}, {13003, 7},
      {13153, 7}, {14787, 7}, {15251, 7}};

  // Includes endpoint not in tree
  result = btree.scan(89796, 99792 + 1);
  expected_result = {{89796, 7}, {90002, 7},  {90061, 7}, {90818, 7},
                     {91226, 7}, {95409, 10}, {95549, 7}, {96389, 7},
                     {96724, 7}, {96998, 7},  {99048, 7}, {99792, 7}};
  EXPECT_EQ(result, expected_result);

  // Include start point not in tree
  result = btree.scan(1928 - 100, 15251);
  expected_result = {{1928, 7},  {4337, 7},  {4711, 7}, {4907, 7}, {5729, 7},
                     {5901, 7},  {7306, 7},  {8445, 7}, {8692, 7}, {13003, 7},
                     {13153, 7}, {14787, 7}, {15251, 7}};
  EXPECT_EQ(result, expected_result);
}

TEST(BTreeTest, MergeBTreesTest) {
  // Create the first BTree to merge

  // Create a mock IOHandler
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  shared_ptr<ExtHash> buffer = make_shared<ExtHash>(5, 5, .8, 1, mockIOHandler);

  string filename = "test_btree1";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  // B tree has 141 keys, 141/3 = 47 pages. Plus 1 for metadata.
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .WillRepeatedly(Return(48));
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 48))
      .WillRepeatedly(Return(
          make_optional<vector<pair<int, int>>>({{0, 3}, {1, 4}, {2, 100}})));

  BTree btree(filename, "test_db", mockIOHandler, buffer, true);

  // Mock readPageFromFile calls to return the pages
  for (int i = 0; i < ceil(static_cast<double>(expected_b_tree.size()) / 3);
       ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] = i * 3 + j < expected_b_tree.size()
                          ? expected_b_tree[i * 3 + j]
                          : make_pair(btree.INF, btree.INF);
    }

    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
  }

  // Create the second BTree to merge
  // Create the first BTree to merge

  string filename2 = "test_btree2";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  // B tree has 141 keys, 141/3 = 47 pages. Plus 1 for metadata.
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename2))
      .WillRepeatedly(Return(48));
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename2, 48))
      .WillRepeatedly(Return(
          make_optional<vector<pair<int, int>>>({{0, 3}, {1, 4}, {2, 100}})));

  BTree btree2(filename2, "test_db", mockIOHandler, buffer, true);

  vector<pair<int, int>> expected_b_tree2 = {{1736, 7},
                                             {2292, 7},
                                             {3602, 7},
                                             {4094, 7},
                                             {4315, 7},
                                             {5520, 7},
                                             {8576, 7},
                                             {8590, 7},
                                             {9104, 7},
                                             {10051, 7},
                                             {10284, 7},
                                             {11317, 7},
                                             {14306, 7},
                                             {14413, 7},
                                             {14655, 7},
                                             {16373, 7},
                                             {16484, 7},
                                             {16828, 7},
                                             {17704, 7},
                                             {18221, 7},
                                             {18989, 7},
                                             {19752, 7},
                                             {20551, 7},
                                             {21556, 7},
                                             {24386, 7},
                                             {25676, 7},
                                             {26783, 7},
                                             {29165, 7},
                                             {29460, 7},
                                             {29860, 7},
                                             {31887, 7},
                                             {32477, 7},
                                             {32564, 7},
                                             {34173, 7},
                                             {37793, 7},
                                             {38279, 7},
                                             {38414, 7},
                                             {38421, 7},
                                             {39157, 7},
                                             {40742, 7},
                                             {41528, 7},
                                             {42005, 7},
                                             {42735, 7},
                                             {44236, 7},
                                             {44828, 7},
                                             {45432, 7},
                                             {46046, 7},
                                             {46123, 7},
                                             {46231, 7},
                                             {46681, 7},
                                             {47365, 7},
                                             {47805, 7},
                                             {49718, 7},
                                             {52813, 7},
                                             {53005, 7},
                                             {53074, 7},
                                             {54729, 7},
                                             {55623, 7},
                                             {57221, 7},
                                             {58020, 7},
                                             {58395, 7},
                                             {59735, 7},
                                             {60063, 7},
                                             {62750, 7},
                                             {62916, 7},
                                             {64109, 7},
                                             {64951, 7},
                                             {65044, 7},
                                             {66377, 7},
                                             {66585, 7},
                                             {68390, 7},
                                             {69402, 7},
                                             {69511, 7},
                                             {71724, 7},
                                             {72232, 7},
                                             {72308, 7},
                                             {74566, 7},
                                             {74601, 7},
                                             {74804, 7},
                                             {75807, 7},
                                             {76047, 7},
                                             {77638, 7},
                                             {78010, 7},
                                             {78236, 7},
                                             {78790, 7},
                                             {80530, 7},
                                             {81172, 7},
                                             {82152, 7},
                                             {83006, 7},
                                             {84217, 7},
                                             {88172, 7},
                                             {88242, 7},
                                             {88768, 7},
                                             {89025, 7},
                                             {89646, 7},
                                             {91446, 7},
                                             {92117, 7},
                                             {92840, 7},
                                             {94537, 7},
                                             {98760, 7},
                                             {btree.INF, btree.INF},
                                             {btree.INF, btree.INF},
                                             {4094, 7},
                                             {8576, 7},
                                             {10051, 7},
                                             {16373, 7},
                                             {17704, 7},
                                             {19752, 7},
                                             {29165, 7},
                                             {31887, 7},
                                             {34173, 7},
                                             {40742, 7},
                                             {42735, 7},
                                             {45432, 7},
                                             {47805, 7},
                                             {53005, 7},
                                             {55623, 7},
                                             {62750, 7},
                                             {64951, 7},
                                             {66585, 7},
                                             {72308, 7},
                                             {74804, 7},
                                             {77638, 7},
                                             {82152, 7},
                                             {88172, 7},
                                             {89025, 7},
                                             {98760, 7},
                                             {btree.INF, btree.INF},
                                             {btree.INF, btree.INF},
                                             {14306, 7},
                                             {24386, 7},
                                             {38414, 7},
                                             {58395, 7},
                                             {69511, 7},
                                             {78790, 7},
                                             {btree.INF, btree.INF},
                                             {btree.INF, btree.INF},
                                             {btree.INF, btree.INF},
                                             {46231, 7},
                                             {92117, 7},
                                             {btree.INF, btree.INF}};

  // Mock readPageFromFile calls to return the pages
  for (int i = 0; i < ceil(static_cast<double>(expected_b_tree2.size()) / 3);
       ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] = i * 3 + j < expected_b_tree2.size()
                          ? expected_b_tree2[i * 3 + j]
                          : make_pair(btree.INF, btree.INF);
    }

    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename2, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
  }

  string filename3 = "test_btree_merged";
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename3))
      .WillRepeatedly(Return(-1));

  vector<pair<int, int>> expected_b_tree3 = {
    {1736, 7}, {1928, 7}, {2292, 7}, {3602, 7}, {4094, 7}, {4315, 7}, {4337, 7}, {4711, 7}, {4907, 7}, {5520, 7}, 
    {5729, 7}, {5901, 7}, {7306, 7}, {8445, 7}, {8576, 7}, {8590, 7}, {8692, 7}, {9104, 7}, {10051, 7}, {10284, 7}, 
    {11317, 7}, {13003, 7}, {13153, 7}, {14306, 7}, {14413, 7}, {14655, 7}, {14787, 7}, {15251, 7}, {15668, 7}, 
    {16201, 7}, {16250, 7}, {16373, 7}, {16484, 7}, {16828, 7}, {17704, 7}, {18221, 7}, {18292, 7}, {18989, 7}, 
    {19592, 7}, {19752, 7}, {19762, 7}, {20404, 7}, {20551, 7}, {21336, 7}, {21556, 7}, {21996, 7}, {23069, 7}, 
    {23227, 7}, {24053, 7}, {24386, 7}, {24855, 7}, {25342, 7}, {25676, 7}, {26097, 7}, {26783, 7}, {27340, 7}, 
    {27744, 7}, {27811, 7}, {28740, 7}, {29165, 7}, {29460, 7}, {29860, 7}, {30116, 7}, {31887, 7}, {32477, 7}, 
    {32497, 7}, {32564, 7}, {32952, 7}, {34035, 7}, {34173, 7}, {35523, 7}, {36108, 7}, {36830, 7}, {37254, 7}, 
    {37509, 7}, {37704, 7}, {37793, 7}, {38104, 7}, {38279, 7}, {38414, 7}, {38421, 7}, {39032, 7}, {39157, 7}, 
    {39731, 7}, {40393, 7}, {40742, 7}, {40886, 7}, {41528, 7}, {42005, 7}, {42694, 7}, {42735, 7}, {43450, 7}, 
    {44236, 7}, {44348, 7}, {44828, 7}, {45432, 7}, {45443, 7}, {45613, 7}, {46046, 7}, {46123, 7}, {46231, 7}, 
    {46681, 7}, {47365, 7}, {47805, 7}, {48474, 7}, {48669, 7}, {48911, 7}, {49718, 7}, {51270, 7}, {52813, 7}, 
    {53005, 7}, {53074, 7}, {53209, 7}, {54334, 7}, {54729, 7}, {55623, 7}, {55640, 7}, {57221, 7}, {58020, 7}, 
    {58395, 7}, {59105, 7}, {59735, 7}, {59808, 7}, {60063, 7}, {61366, 7}, {62411, 7}, {62750, 7}, {62916, 7}, 
    {63081, 7}, {64109, 7}, {64851, 7}, {64951, 7}, {65044, 7}, {66377, 7}, {66585, 7}, {68390, 7}, {68529, 7}, 
    {68558, 7}, {69402, 7}, {69511, 7}, {70250, 7}, {70883, 7}, {71235, 7}, {71724, 7}, {72232, 7}, {72308, 7}, 
    {72561, 7}, {73671, 7}, {74208, 7}, {74298, 7}, {74566, 7}, {74601, 7}, {74804, 7}, {75478, 7}, {75807, 7}, 
    {76047, 7}, {76509, 7}, {76628, 7}, {77638, 7}, {78010, 7}, {78236, 7}, {78790, 7}, {79776, 7}, {80530, 7}, 
    {81172, 7}, {82152, 7}, {82843, 7}, {83006, 7}, {83553, 7}, {84217, 7}, {84785, 7}, {84824, 7}, {84942, 7}, 
    {85225, 7}, {85790, 7}, {88172, 7}, {88242, 7}, {88498, 7}, {88768, 7}, {89025, 7}, {89181, 7}, {89514, 7}, 
    {89646, 7}, {89796, 7}, {90002, 7}, {90061, 7}, {90818, 7}, {91226, 7}, {91446, 7}, {92117, 7}, {92840, 7}, 
    {94537, 7}, {95409, 10}, {95549, 7}, {96389, 7}, {96724, 7}, {96998, 7}, {98760, 7}, {99048, 7}, {99792, 7}, 
    {btree.INF, btree.INF}, {3602, 7}, {4337, 7}, {5520, 7}, {8590, 7}, {10051, 7}, {13003, 7}, {15251, 7}, 
    {16250, 7}, {16828, 7}, {19752, 7}, {20551, 7}, {21996, 7}, {25342, 7}, {26783, 7}, {27811, 7}, {31887, 7}, 
    {32564, 7}, {34173, 7}, {37704, 7}, {38279, 7}, {39032, 7}, {41528, 7}, {42735, 7}, {44348, 7}, {46123, 7}, 
    {47365, 7}, {48669, 7}, {53074, 7}, {54729, 7}, {57221, 7}, {60063, 7}, {62750, 7}, {64109, 7}, {68390, 7}, 
    {69402, 7}, {70883, 7}, {73671, 7}, {74566, 7}, {75478, 7}, {78010, 7}, {79776, 7}, {82152, 7}, {84824, 7}, 
    {85790, 7}, {88498, 7}, {89796, 7}, {90818, 7}, {92117, 7}, {96724, 7}, {99048, 7}, {btree.INF, btree.INF}, 
    {7306, 7}, {14413, 7}, {18292, 7}, {29460, 7}, {36830, 7}, {40393, 7}, {51270, 7}, {59105, 7}, {65044, 7}, 
    {76509, 7}, {83553, 7}, {89181, 7}, {btree.INF, btree.INF}, {btree.INF, btree.INF}, {btree.INF, btree.INF}, 
    {24053, 7}, {45443, 7}, {72232, 7}, {btree.INF, btree.INF}, {btree.INF, btree.INF}, {btree.INF, btree.INF}
};

  // Mock the leaf pages for the merged BTree. Also assert that the leaf pages are written.
  for (int i = 0; i < ceil(static_cast<double>(expected_b_tree3.size()) / 3);
       ++i) {
    vector<pair<int, int>> data_slice(3);
    for (int j = 0; j < 3; j++) {
      data_slice[j] = i * 3 + j < expected_b_tree3.size()
                          ? expected_b_tree3[i * 3 + j]
                          : make_pair(btree.INF, btree.INF);
    }

    if (i < 201.0/3){

      for (int j = 0; j < 3; j++) {
        // When calling write, we do not pad with infinities, we let the io handler pad.
        // so we need to remove those values from the data slice.
        if (data_slice[j].first == btree.INF) {
          data_slice.resize(j);
          break;
        }
      }

      EXPECT_CALL(*mockIOHandler, writeToFile(filename3, data_slice, i + 1))
        .Times(1);
    }
   
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename3, i + 1))
        .Times(AnyNumber())
        .WillRepeatedly(Return(data_slice));
    data_slice.clear();
  }

  // Expect one write to file call for the internal nodes
  // leaf nodes take up 67 pages, so we write internal at 68.
  EXPECT_CALL(*mockIOHandler, writeToFile(filename3, _, 68))
      .Times(1);

  // Expect the metadata to be written
  EXPECT_CALL(*mockIOHandler, writeToFile(filename3, _, 92))
      .Times(1);

  vector<pair<int, int>> expected_internal_nodes(72);
  for (int i = 0; i < 72; i++) {
    expected_internal_nodes[i] = expected_b_tree3[201 + i];
  }
  BTree btree_merged = BTree(filename3, "test_db", mockIOHandler, buffer, true);
  btree_merged.mergeBTrees(filename, filename2);
  
  vector<pair<int, int>> internal_nodes = btree_merged.contructInternalNodes();

  
  for (int i = 0; i < internal_nodes.size(); i++) {
    EXPECT_EQ(internal_nodes[i].first, expected_internal_nodes[i].first);
    EXPECT_EQ(internal_nodes[i].second, expected_internal_nodes[i].second);
  }
}