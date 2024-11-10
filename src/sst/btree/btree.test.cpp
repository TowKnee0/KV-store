#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../io/io.h"

#define private public
#include "btree.h"

using namespace testing;

// Create a mock for IOHandler
class MockIOHandler : public IO {
 public:
  MOCK_METHOD(int, writeToFile,
              (const string filename, (const vector<pair<int, int>> &data)));
  MOCK_METHOD((optional<vector<pair<int, int>>>), readPageFromFile,
              (const string filename, int pageNumber));
  MOCK_METHOD(int, numPagesInFile, (const string filename));
  MOCK_METHOD(vector<string>, getDbFilenames, ());
  MOCK_METHOD(int, getPageSize, ());
  MOCK_METHOD(int, getPairSize, ());
  MOCK_METHOD(int, writeToFileAtPage,
              (const string filename, (const vector<pair<int, int>> &data),
               int pageNumber));
};

TEST(BTreeTest, WriteTest) {
  // Create a mock IOHandler
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  string filename = "test_btree";

  // Set the getPageSize to be 24, 3 pairs.
  EXPECT_CALL(*mockIOHandler, getPageSize())
      .WillRepeatedly(Return(24));  // Page size which has 3 values each
  // Set the pair size to be the size of 2 integers.
  EXPECT_CALL(*mockIOHandler, getPairSize()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .WillRepeatedly(Return(-1));

  BTree btree(filename, "test_db", mockIOHandler, false);

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
    data.emplace_back(value, 7);
  }

  // Expect the writeToFile method to be called once
  EXPECT_CALL(*mockIOHandler, writeToFile(filename, data))
      .Times(1)
      .WillOnce(Return(1));
  Sequence s;
  EXPECT_CALL(*mockIOHandler, numPagesInFile(filename))
      .InSequence(s)
      .WillOnce(Return(ceil(100.0 / 3)))
      .WillOnce(Return(47));

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

  vector<pair<int, int>> expected_b_tree = {{1928, 7},
                                            {4337, 7},
                                            {4711, 7},
                                            {4907, 7},
                                            {5729, 7},
                                            {5901, 7},
                                            {7306, 7},
                                            {8445, 7},
                                            {8692, 7},
                                            {13003, 7},
                                            {13153, 7},
                                            {14787, 7},
                                            {15251, 7},
                                            {15668, 7},
                                            {16201, 7},
                                            {16250, 7},
                                            {18292, 7},
                                            {19592, 7},
                                            {19762, 7},
                                            {20404, 7},
                                            {21336, 7},
                                            {21996, 7},
                                            {23069, 7},
                                            {23227, 7},
                                            {24053, 7},
                                            {24855, 7},
                                            {25342, 7},
                                            {26097, 7},
                                            {27340, 7},
                                            {27744, 7},
                                            {27811, 7},
                                            {28740, 7},
                                            {30116, 7},
                                            {32497, 7},
                                            {32952, 7},
                                            {34035, 7},
                                            {35523, 7},
                                            {36108, 7},
                                            {36830, 7},
                                            {37254, 7},
                                            {37509, 7},
                                            {37704, 7},
                                            {38104, 7},
                                            {39032, 7},
                                            {39731, 7},
                                            {40393, 7},
                                            {40886, 7},
                                            {42694, 7},
                                            {43450, 7},
                                            {44348, 7},
                                            {45443, 7},
                                            {45613, 7},
                                            {48474, 7},
                                            {48669, 7},
                                            {48911, 7},
                                            {51270, 7},
                                            {53209, 7},
                                            {54334, 7},
                                            {55640, 7},
                                            {59105, 7},
                                            {59808, 7},
                                            {61366, 7},
                                            {62411, 7},
                                            {63081, 7},
                                            {64851, 7},
                                            {68529, 7},
                                            {68558, 7},
                                            {70250, 7},
                                            {70883, 7},
                                            {71235, 7},
                                            {72561, 7},
                                            {73671, 7},
                                            {74208, 7},
                                            {74298, 7},
                                            {75478, 7},
                                            {76509, 7},
                                            {76628, 7},
                                            {79776, 7},
                                            {82843, 7},
                                            {83553, 7},
                                            {84785, 7},
                                            {84824, 7},
                                            {84942, 7},
                                            {85225, 7},
                                            {85790, 7},
                                            {88498, 7},
                                            {89181, 7},
                                            {89514, 7},
                                            {89796, 7},
                                            {90002, 7},
                                            {90061, 7},
                                            {90818, 7},
                                            {91226, 7},
                                            {95409, 7},
                                            {95549, 7},
                                            {96389, 7},
                                            {96724, 7},
                                            {96998, 7},
                                            {99048, 7},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {4907, 7},
                                            {7306, 7},
                                            {13003, 7},
                                            {16250, 7},
                                            {19762, 7},
                                            {21996, 7},
                                            {26097, 7},
                                            {27811, 7},
                                            {32497, 7},
                                            {37254, 7},
                                            {38104, 7},
                                            {40393, 7},
                                            {45613, 7},
                                            {48911, 7},
                                            {54334, 7},
                                            {63081, 7},
                                            {68558, 7},
                                            {71235, 7},
                                            {76509, 7},
                                            {82843, 7},
                                            {84824, 7},
                                            {89514, 7},
                                            {90061, 7},
                                            {95409, 7},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {15251, 7},
                                            {24053, 7},
                                            {35523, 7},
                                            {59808, 7},
                                            {74208, 7},
                                            {85790, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {43450, 7},
                                            {96724, 7},
                                            {btree.INF, btree.INF}};

  vector<pair<int, int>> expected_internal_nodes(41);
  for (int i = 0; i < 39; i++) {
    expected_internal_nodes[i] = expected_b_tree[102 + i];
  }

  // Expect the writeToFileAtPage method to be called to write the internal
  // nodes
  EXPECT_CALL(*mockIOHandler, writeToFileAtPage(filename, _, 35))
      .Times(1)
      .WillOnce(Return(0));

  // Expect the writeToFileAtPage method to be called to write metdata
  EXPECT_CALL(*mockIOHandler, writeToFileAtPage(filename, _, 48))
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

// TEST(BTreeTest, GetTest)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Create some data
//     vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};

//     // Expect the getNumPagesInFile method to be called once, returning 1
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).WillRepeatedly(Return(1));

//     // Expect the readPageFromFile method to be called once
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     1)).WillRepeatedly(Return(data));

//     // Call the get method
//     EXPECT_EQ(btree.get(1), 2);
// }

// TEST(BTreeTest, GetNonExistentKeyTest)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Create some data
//     vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};

//     // Expect the getNumPagesInFile method to be called once, returning 1
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).WillRepeatedly(Return(1));

//     // Expect the readPageFromFile method to be called once
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     1)).WillRepeatedly(Return(data));

//     // Call the get method with a non-existent key
//     EXPECT_EQ(btree.get(4), nullopt);
// }

// TEST(BTreeTest, ScanTest)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Create some data
//     vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
//     vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
//     vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
//     vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

//     // Expect the getNumPagesInFile method to be called once, returning 4
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).WillRepeatedly(Return(4));

//     // Expect the readPageFromFile method to be called four times
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     1)).Times(AtLeast(1)).WillRepeatedly(Return(data1));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

//     // Call the scan method
//     vector<pair<int, int>> result = btree.scan(1, 20);

//     // Check the result
//     ASSERT_EQ(result.size(), 9);
//     EXPECT_EQ(result.at(0).first, 3);
//     EXPECT_EQ(result.at(0).second, 4);
//     EXPECT_EQ(result.at(1).first, 5);
//     EXPECT_EQ(result.at(1).second, 6);
//     EXPECT_EQ(result.at(2).first, 7);
//     EXPECT_EQ(result.at(2).second, 8);
//     EXPECT_EQ(result.at(3).first, 9);
//     EXPECT_EQ(result.at(3).second, 10);
//     EXPECT_EQ(result.at(4).first, 11);
//     EXPECT_EQ(result.at(4).second, 12);
//     EXPECT_EQ(result.at(5).first, 13);
//     EXPECT_EQ(result.at(5).second, 14);
//     EXPECT_EQ(result.at(6).first, 15);
//     EXPECT_EQ(result.at(6).second, 16);
//     EXPECT_EQ(result.at(7).first, 17);
//     EXPECT_EQ(result.at(7).second, 18);
//     EXPECT_EQ(result.at(8).first, 19);
//     EXPECT_EQ(result.at(8).second, 20);
// }

// TEST(BTreeTest, ScanTest2)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Create some data
//     vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
//     vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
//     vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
//     vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

//     // Expect the getNumPagesInFile method to be called once, returning 4
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).WillRepeatedly(Return(4));

//     // Expect the readPageFromFile method to be called four times
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     1)).Times(AtLeast(1)).WillRepeatedly(Return(data1));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

//     // Call the scan method
//     vector<pair<int, int>> result = btree.scan(6, 20);

//     // Check the result
//     ASSERT_EQ(result.size(), 7);
//     EXPECT_EQ(result.at(0).first, 7);
//     EXPECT_EQ(result.at(0).second, 8);
//     EXPECT_EQ(result.at(1).first, 9);
//     EXPECT_EQ(result.at(1).second, 10);
//     EXPECT_EQ(result.at(2).first, 11);
//     EXPECT_EQ(result.at(2).second, 12);
//     EXPECT_EQ(result.at(3).first, 13);
//     EXPECT_EQ(result.at(3).second, 14);
//     EXPECT_EQ(result.at(4).first, 15);
//     EXPECT_EQ(result.at(4).second, 16);
//     EXPECT_EQ(result.at(5).first, 17);
//     EXPECT_EQ(result.at(5).second, 18);
//     EXPECT_EQ(result.at(6).first, 19);
//     EXPECT_EQ(result.at(6).second, 20);
// }

// TEST(BTreeTest, ScanTest3)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Create some data
//     vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
//     vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
//     vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
//     vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

//     // Expect the getNumPagesInFile method to be called once, returning 4
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).WillRepeatedly(Return(4));

//     // Expect the readPageFromFile method to be called four times
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

//     // Call the scan method
//     vector<pair<int, int>> result = btree.scan(8, 20);

//     // Check the result
//     ASSERT_EQ(result.size(), 6);
//     EXPECT_EQ(result.at(0).first, 9);
//     EXPECT_EQ(result.at(0).second, 10);
//     EXPECT_EQ(result.at(1).first, 11);
//     EXPECT_EQ(result.at(1).second, 12);
//     EXPECT_EQ(result.at(2).first, 13);
//     EXPECT_EQ(result.at(2).second, 14);
//     EXPECT_EQ(result.at(3).first, 15);
//     EXPECT_EQ(result.at(3).second, 16);
//     EXPECT_EQ(result.at(4).first, 17);
//     EXPECT_EQ(result.at(4).second, 18);
//     EXPECT_EQ(result.at(5).first, 19);
//     EXPECT_EQ(result.at(5).second, 20);
// }

// TEST(BTreeTest, ScanTest4)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Create some data
//     vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
//     vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
//     vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
//     vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

//     // Expect the getNumPagesInFile method to be called once, returning 4
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).WillRepeatedly(Return(4));

//     // Expect the readPageFromFile method to be called four times
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

//     // Call the scan method
//     vector<pair<int, int>> result = btree.scan(19, 19);

//     // Check the result
//     ASSERT_EQ(result.size(), 1);
//     EXPECT_EQ(result.at(0).first, 19);
//     EXPECT_EQ(result.at(0).second, 20);
// }

// TEST(BTreeTest, ScanTest5)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Create some data
//     vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
//     vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
//     vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
//     vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

//     // Expect the getNumPagesInFile method to be called once, returning 4
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).WillRepeatedly(Return(4));

//     // Expect the readPageFromFile method to be called four times
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     1)).Times(AtLeast(1)).WillRepeatedly(Return(data1));
//     EXPECT_CALL(*mockIOHandler, readPageFromFile(filename,
//     2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));

//     // Call the scan method
//     vector<pair<int, int>> result = btree.scan(1, 2);

//     // Check the result
//     ASSERT_EQ(result.size(), 0);
// }

// TEST(BTreeTest, ScanEmptyRangeTest)
// {
//     // Create a mock IOHandler
//     shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
//     string filename = "test_btree";
//     BTree btree(filename, "test_db", mockIOHandler);

//     // Expect the getNumPagesInFile method to be called once, returning 0
//     EXPECT_CALL(*mockIOHandler,
//     numPagesInFile(filename)).Times(1).WillOnce(Return(0));

//     // Call the scan method with an empty range
//     vector<pair<int, int>> result = btree.scan(1, 6);

//     // Check the result
//     ASSERT_EQ(result.size(), 0);
// }

TEST(BTreeTest, GetTest) {
  // Create a mock IOHandler
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
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

  BTree btree(filename, "test_db", mockIOHandler, false);

  vector<pair<int, int>> expected_b_tree = {{1928, 7},
                                            {4337, 7},
                                            {4711, 7},
                                            {4907, 7},
                                            {5729, 7},
                                            {5901, 7},
                                            {7306, 7},
                                            {8445, 7},
                                            {8692, 7},
                                            {13003, 7},
                                            {13153, 7},
                                            {14787, 7},
                                            {15251, 7},
                                            {15668, 7},
                                            {16201, 7},
                                            {16250, 7},
                                            {18292, 7},
                                            {19592, 7},
                                            {19762, 7},
                                            {20404, 7},
                                            {21336, 7},
                                            {21996, 7},
                                            {23069, 7},
                                            {23227, 7},
                                            {24053, 7},
                                            {24855, 7},
                                            {25342, 7},
                                            {26097, 7},
                                            {27340, 7},
                                            {27744, 7},
                                            {27811, 7},
                                            {28740, 7},
                                            {30116, 7},
                                            {32497, 7},
                                            {32952, 7},
                                            {34035, 7},
                                            {35523, 7},
                                            {36108, 7},
                                            {36830, 7},
                                            {37254, 7},
                                            {37509, 7},
                                            {37704, 7},
                                            {38104, 7},
                                            {39032, 7},
                                            {39731, 7},
                                            {40393, 7},
                                            {40886, 7},
                                            {42694, 7},
                                            {43450, 7},
                                            {44348, 7},
                                            {45443, 7},
                                            {45613, 7},
                                            {48474, 7},
                                            {48669, 7},
                                            {48911, 7},
                                            {51270, 7},
                                            {53209, 7},
                                            {54334, 7},
                                            {55640, 7},
                                            {59105, 7},
                                            {59808, 7},
                                            {61366, 7},
                                            {62411, 7},
                                            {63081, 7},
                                            {64851, 7},
                                            {68529, 7},
                                            {68558, 7},
                                            {70250, 7},
                                            {70883, 7},
                                            {71235, 7},
                                            {72561, 7},
                                            {73671, 7},
                                            {74208, 7},
                                            {74298, 7},
                                            {75478, 7},
                                            {76509, 7},
                                            {76628, 7},
                                            {79776, 7},
                                            {82843, 7},
                                            {83553, 7},
                                            {84785, 7},
                                            {84824, 7},
                                            {84942, 7},
                                            {85225, 7},
                                            {85790, 7},
                                            {88498, 7},
                                            {89181, 7},
                                            {89514, 7},
                                            {89796, 7},
                                            {90002, 7},
                                            {90061, 7},
                                            {90818, 7},
                                            {91226, 7},
                                            {95409, 10},
                                            {95549, 7},
                                            {96389, 7},
                                            {96724, 7},
                                            {96998, 7},
                                            {99048, 7},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {4907, 7},
                                            {7306, 7},
                                            {13003, 7},
                                            {16250, 7},
                                            {19762, 7},
                                            {21996, 7},
                                            {26097, 7},
                                            {27811, 7},
                                            {32497, 7},
                                            {37254, 7},
                                            {38104, 7},
                                            {40393, 7},
                                            {45613, 7},
                                            {48911, 7},
                                            {54334, 7},
                                            {63081, 7},
                                            {68558, 7},
                                            {71235, 7},
                                            {76509, 7},
                                            {82843, 7},
                                            {84824, 7},
                                            {89514, 7},
                                            {90061, 7},
                                            {95409, 10},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {15251, 7},
                                            {24053, 7},
                                            {35523, 7},
                                            {59808, 7},
                                            {74208, 7},
                                            {85790, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {43450, 7},
                                            {96724, 7},
                                            {btree.INF, btree.INF}};

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

  BTree btree(filename, "test_db", mockIOHandler, true);

  vector<pair<int, int>> expected_b_tree = {{1928, 7},
                                            {4337, 7},
                                            {4711, 7},
                                            {4907, 7},
                                            {5729, 7},
                                            {5901, 7},
                                            {7306, 7},
                                            {8445, 7},
                                            {8692, 7},
                                            {13003, 7},
                                            {13153, 7},
                                            {14787, 7},
                                            {15251, 7},
                                            {15668, 7},
                                            {16201, 7},
                                            {16250, 7},
                                            {18292, 7},
                                            {19592, 7},
                                            {19762, 7},
                                            {20404, 7},
                                            {21336, 7},
                                            {21996, 7},
                                            {23069, 7},
                                            {23227, 7},
                                            {24053, 7},
                                            {24855, 7},
                                            {25342, 7},
                                            {26097, 7},
                                            {27340, 7},
                                            {27744, 7},
                                            {27811, 7},
                                            {28740, 7},
                                            {30116, 7},
                                            {32497, 7},
                                            {32952, 7},
                                            {34035, 7},
                                            {35523, 7},
                                            {36108, 7},
                                            {36830, 7},
                                            {37254, 7},
                                            {37509, 7},
                                            {37704, 7},
                                            {38104, 7},
                                            {39032, 7},
                                            {39731, 7},
                                            {40393, 7},
                                            {40886, 7},
                                            {42694, 7},
                                            {43450, 7},
                                            {44348, 7},
                                            {45443, 7},
                                            {45613, 7},
                                            {48474, 7},
                                            {48669, 7},
                                            {48911, 7},
                                            {51270, 7},
                                            {53209, 7},
                                            {54334, 7},
                                            {55640, 7},
                                            {59105, 7},
                                            {59808, 7},
                                            {61366, 7},
                                            {62411, 7},
                                            {63081, 7},
                                            {64851, 7},
                                            {68529, 7},
                                            {68558, 7},
                                            {70250, 7},
                                            {70883, 7},
                                            {71235, 7},
                                            {72561, 7},
                                            {73671, 7},
                                            {74208, 7},
                                            {74298, 7},
                                            {75478, 7},
                                            {76509, 7},
                                            {76628, 7},
                                            {79776, 7},
                                            {82843, 7},
                                            {83553, 7},
                                            {84785, 7},
                                            {84824, 7},
                                            {84942, 7},
                                            {85225, 7},
                                            {85790, 7},
                                            {88498, 7},
                                            {89181, 7},
                                            {89514, 7},
                                            {89796, 7},
                                            {90002, 7},
                                            {90061, 7},
                                            {90818, 7},
                                            {91226, 7},
                                            {95409, 10},
                                            {95549, 7},
                                            {96389, 7},
                                            {96724, 7},
                                            {96998, 7},
                                            {99048, 7},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {4907, 7},
                                            {7306, 7},
                                            {13003, 7},
                                            {16250, 7},
                                            {19762, 7},
                                            {21996, 7},
                                            {26097, 7},
                                            {27811, 7},
                                            {32497, 7},
                                            {37254, 7},
                                            {38104, 7},
                                            {40393, 7},
                                            {45613, 7},
                                            {48911, 7},
                                            {54334, 7},
                                            {63081, 7},
                                            {68558, 7},
                                            {71235, 7},
                                            {76509, 7},
                                            {82843, 7},
                                            {84824, 7},
                                            {89514, 7},
                                            {90061, 7},
                                            {95409, 10},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {15251, 7},
                                            {24053, 7},
                                            {35523, 7},
                                            {59808, 7},
                                            {74208, 7},
                                            {85790, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {43450, 7},
                                            {96724, 7},
                                            {btree.INF, btree.INF}};

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

  BTree btree(filename, "test_db", mockIOHandler, false);

  vector<pair<int, int>> expected_b_tree = {{1928, 7},
                                            {4337, 7},
                                            {4711, 7},
                                            {4907, 7},
                                            {5729, 7},
                                            {5901, 7},
                                            {7306, 7},
                                            {8445, 7},
                                            {8692, 7},
                                            {13003, 7},
                                            {13153, 7},
                                            {14787, 7},
                                            {15251, 7},
                                            {15668, 7},
                                            {16201, 7},
                                            {16250, 7},
                                            {18292, 7},
                                            {19592, 7},
                                            {19762, 7},
                                            {20404, 7},
                                            {21336, 7},
                                            {21996, 7},
                                            {23069, 7},
                                            {23227, 7},
                                            {24053, 7},
                                            {24855, 7},
                                            {25342, 7},
                                            {26097, 7},
                                            {27340, 7},
                                            {27744, 7},
                                            {27811, 7},
                                            {28740, 7},
                                            {30116, 7},
                                            {32497, 7},
                                            {32952, 7},
                                            {34035, 7},
                                            {35523, 7},
                                            {36108, 7},
                                            {36830, 7},
                                            {37254, 7},
                                            {37509, 7},
                                            {37704, 7},
                                            {38104, 7},
                                            {39032, 7},
                                            {39731, 7},
                                            {40393, 7},
                                            {40886, 7},
                                            {42694, 7},
                                            {43450, 7},
                                            {44348, 7},
                                            {45443, 7},
                                            {45613, 7},
                                            {48474, 7},
                                            {48669, 7},
                                            {48911, 7},
                                            {51270, 7},
                                            {53209, 7},
                                            {54334, 7},
                                            {55640, 7},
                                            {59105, 7},
                                            {59808, 7},
                                            {61366, 7},
                                            {62411, 7},
                                            {63081, 7},
                                            {64851, 7},
                                            {68529, 7},
                                            {68558, 7},
                                            {70250, 7},
                                            {70883, 7},
                                            {71235, 7},
                                            {72561, 7},
                                            {73671, 7},
                                            {74208, 7},
                                            {74298, 7},
                                            {75478, 7},
                                            {76509, 7},
                                            {76628, 7},
                                            {79776, 7},
                                            {82843, 7},
                                            {83553, 7},
                                            {84785, 7},
                                            {84824, 7},
                                            {84942, 7},
                                            {85225, 7},
                                            {85790, 7},
                                            {88498, 7},
                                            {89181, 7},
                                            {89514, 7},
                                            {89796, 7},
                                            {90002, 7},
                                            {90061, 7},
                                            {90818, 7},
                                            {91226, 7},
                                            {95409, 10},
                                            {95549, 7},
                                            {96389, 7},
                                            {96724, 7},
                                            {96998, 7},
                                            {99048, 7},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {4907, 7},
                                            {7306, 7},
                                            {13003, 7},
                                            {16250, 7},
                                            {19762, 7},
                                            {21996, 7},
                                            {26097, 7},
                                            {27811, 7},
                                            {32497, 7},
                                            {37254, 7},
                                            {38104, 7},
                                            {40393, 7},
                                            {45613, 7},
                                            {48911, 7},
                                            {54334, 7},
                                            {63081, 7},
                                            {68558, 7},
                                            {71235, 7},
                                            {76509, 7},
                                            {82843, 7},
                                            {84824, 7},
                                            {89514, 7},
                                            {90061, 7},
                                            {95409, 10},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {15251, 7},
                                            {24053, 7},
                                            {35523, 7},
                                            {59808, 7},
                                            {74208, 7},
                                            {85790, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {43450, 7},
                                            {96724, 7},
                                            {btree.INF, btree.INF}};

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

  BTree btree(filename, "test_db", mockIOHandler, true);

  vector<pair<int, int>> expected_b_tree = {{1928, 7},
                                            {4337, 7},
                                            {4711, 7},
                                            {4907, 7},
                                            {5729, 7},
                                            {5901, 7},
                                            {7306, 7},
                                            {8445, 7},
                                            {8692, 7},
                                            {13003, 7},
                                            {13153, 7},
                                            {14787, 7},
                                            {15251, 7},
                                            {15668, 7},
                                            {16201, 7},
                                            {16250, 7},
                                            {18292, 7},
                                            {19592, 7},
                                            {19762, 7},
                                            {20404, 7},
                                            {21336, 7},
                                            {21996, 7},
                                            {23069, 7},
                                            {23227, 7},
                                            {24053, 7},
                                            {24855, 7},
                                            {25342, 7},
                                            {26097, 7},
                                            {27340, 7},
                                            {27744, 7},
                                            {27811, 7},
                                            {28740, 7},
                                            {30116, 7},
                                            {32497, 7},
                                            {32952, 7},
                                            {34035, 7},
                                            {35523, 7},
                                            {36108, 7},
                                            {36830, 7},
                                            {37254, 7},
                                            {37509, 7},
                                            {37704, 7},
                                            {38104, 7},
                                            {39032, 7},
                                            {39731, 7},
                                            {40393, 7},
                                            {40886, 7},
                                            {42694, 7},
                                            {43450, 7},
                                            {44348, 7},
                                            {45443, 7},
                                            {45613, 7},
                                            {48474, 7},
                                            {48669, 7},
                                            {48911, 7},
                                            {51270, 7},
                                            {53209, 7},
                                            {54334, 7},
                                            {55640, 7},
                                            {59105, 7},
                                            {59808, 7},
                                            {61366, 7},
                                            {62411, 7},
                                            {63081, 7},
                                            {64851, 7},
                                            {68529, 7},
                                            {68558, 7},
                                            {70250, 7},
                                            {70883, 7},
                                            {71235, 7},
                                            {72561, 7},
                                            {73671, 7},
                                            {74208, 7},
                                            {74298, 7},
                                            {75478, 7},
                                            {76509, 7},
                                            {76628, 7},
                                            {79776, 7},
                                            {82843, 7},
                                            {83553, 7},
                                            {84785, 7},
                                            {84824, 7},
                                            {84942, 7},
                                            {85225, 7},
                                            {85790, 7},
                                            {88498, 7},
                                            {89181, 7},
                                            {89514, 7},
                                            {89796, 7},
                                            {90002, 7},
                                            {90061, 7},
                                            {90818, 7},
                                            {91226, 7},
                                            {95409, 10},
                                            {95549, 7},
                                            {96389, 7},
                                            {96724, 7},
                                            {96998, 7},
                                            {99048, 7},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {4907, 7},
                                            {7306, 7},
                                            {13003, 7},
                                            {16250, 7},
                                            {19762, 7},
                                            {21996, 7},
                                            {26097, 7},
                                            {27811, 7},
                                            {32497, 7},
                                            {37254, 7},
                                            {38104, 7},
                                            {40393, 7},
                                            {45613, 7},
                                            {48911, 7},
                                            {54334, 7},
                                            {63081, 7},
                                            {68558, 7},
                                            {71235, 7},
                                            {76509, 7},
                                            {82843, 7},
                                            {84824, 7},
                                            {89514, 7},
                                            {90061, 7},
                                            {95409, 10},
                                            {99792, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {15251, 7},
                                            {24053, 7},
                                            {35523, 7},
                                            {59808, 7},
                                            {74208, 7},
                                            {85790, 7},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {btree.INF, btree.INF},
                                            {43450, 7},
                                            {96724, 7},
                                            {btree.INF, btree.INF}};

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