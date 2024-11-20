#include "sorted_list.h"
#include <vector>
#include <optional>
#include <utility>
#include <string>
#include <iostream>
#include "../../io/io.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace testing;

// Create a mock for IOHandler
class MockIOHandler : public IO
{
public:
    MOCK_METHOD(int, writeToFile, (const string filename, (const vector<pair<int, int>> &data), int pageNumber));
    MOCK_METHOD((optional<vector<pair<int, int>>>), readPageFromFile, (const string filename, int pageNumber));
    MOCK_METHOD(int, numPagesInFile, (const string filename));
    MOCK_METHOD(vector<string>, getDbFilenames, ());
    MOCK_METHOD(int, getPageSize, ());
    MOCK_METHOD(int, getPairSize, ());
    MOCK_METHOD(void, del, (string filename));
    MOCK_METHOD(int, renameFile, (string source_filename, string dest_filename));
};

TEST(SstTest, WriteTest)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};

    // Expect the writeToFile method to be called once
    EXPECT_CALL(*mockIOHandler, writeToFile(filename, data, 0)).Times(1).WillOnce(Return(1));

    // Call the write method
    EXPECT_EQ(sortedList.write(data), 0);
}

TEST(SstTest, GetTest)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};

    // Expect the getNumPagesInFile method to be called once, returning 1
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(1));

    // Expect the readPageFromFile method to be called once
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).WillRepeatedly(Return(data));

    // Call the get method
    EXPECT_EQ(sortedList.get(1), 2);
}

// Test get when we have multiple pages worth of data
TEST(SstTest, GetTestLarge)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data1 = {{1, 2}, {3, 4}, {5, 6}};
    vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
    vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
    vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

    // Expect the getNumPagesInFile method to be called once, returning 4
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(4));

    // Expect the readPageFromFile method to be called four times
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 2)).WillRepeatedly(Return(data2));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).WillRepeatedly(Return(data1));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 3)).WillRepeatedly(Return(data3));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 4)).WillRepeatedly(Return(data4));

    // Call the get method
    EXPECT_EQ(sortedList.get(7), 8);
    EXPECT_EQ(sortedList.get(15), 16);
    EXPECT_EQ(sortedList.get(23), 24);

    // Test the getKeyData method
    vector<int> keyData = sortedList.getKeyData(15);
    EXPECT_EQ(keyData[0], 1);  // index in page
    EXPECT_EQ(keyData[1], 3);  // Page number
    EXPECT_EQ(keyData[2], 15); // key
    EXPECT_EQ(keyData[3], 16); // value

    // test not in the list at the beginning
    keyData = sortedList.getKeyData(0);
    EXPECT_EQ(keyData[0], 0); // index in page
    EXPECT_EQ(keyData[1], 1); // Page number
    EXPECT_EQ(keyData[2], 1); // key
    EXPECT_EQ(keyData[3], 2); // value

    // test not in the list in the middle
    keyData = sortedList.getKeyData(8);
    EXPECT_EQ(keyData[0], 1);  // index in page
    EXPECT_EQ(keyData[1], 2);  // Page number
    EXPECT_EQ(keyData[2], 9);  // key
    EXPECT_EQ(keyData[3], 10); // value

    // test not in the list at the end
    keyData = sortedList.getKeyData(24);
    EXPECT_EQ(keyData[0], -1); // index in page
    EXPECT_EQ(keyData[1], -1); // Page number
    EXPECT_EQ(keyData[2], -1); // key
    EXPECT_EQ(keyData[3], -1); // value

    // test exactly at the end
    keyData = sortedList.getKeyData(23);
    EXPECT_EQ(keyData[0], 2);  // index in page
    EXPECT_EQ(keyData[1], 4);  // Page number
    EXPECT_EQ(keyData[2], 23); // key
    EXPECT_EQ(keyData[3], 24); // value

    // test exactly at the start
    keyData = sortedList.getKeyData(1);
    EXPECT_EQ(keyData[0], 0); // index in page
    EXPECT_EQ(keyData[1], 1); // Page number
    EXPECT_EQ(keyData[2], 1); // key
    EXPECT_EQ(keyData[3], 2); // value
}

TEST(SstTest, GetNonExistentKeyTest)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};

    // Expect the getNumPagesInFile method to be called once, returning 1
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(1));

    // Expect the readPageFromFile method to be called once
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).WillRepeatedly(Return(data));

    // Call the get method with a non-existent key
    EXPECT_EQ(sortedList.get(4), nullopt);
}