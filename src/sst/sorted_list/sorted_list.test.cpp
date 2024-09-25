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
    MOCK_METHOD(int, writeToFile, (const string filename, (const vector<pair<int, int>> &data)));
    MOCK_METHOD((optional<vector<pair<int, int>>>), readPageFromFile, (const string filename, int pageNumber));
    MOCK_METHOD(int, numPagesInFile, (const string filename));
    MOCK_METHOD(vector<string>, getDbFilenames, ());
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
    EXPECT_CALL(*mockIOHandler, writeToFile(filename, data)).Times(1).WillOnce(Return(1));

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
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).Times(1).WillOnce(Return(1));

    // Expect the readPageFromFile method to be called once
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).Times(1).WillOnce(Return(data));

    // Call the get method
    EXPECT_EQ(sortedList.get(1), 2);
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
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).Times(1).WillOnce(Return(1));

    // Expect the readPageFromFile method to be called once
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).Times(1).WillOnce(Return(data));

    // Call the get method with a non-existent key
    EXPECT_EQ(sortedList.get(10), nullopt);
}

TEST(SstTest, ScanTest)
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
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).Times(1).WillOnce(Return(4));

    // Expect the readPageFromFile method to be called four times
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).Times(1).WillOnce(Return(data1));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 2)).Times(1).WillOnce(Return(data2));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 3)).Times(1).WillOnce(Return(data3));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 4)).Times(1).WillOnce(Return(data4));

    // Call the scan method
    vector<entry> *result = sortedList.scan(1, 20);

    // Check the result
    ASSERT_EQ(result->size(), 10);
    EXPECT_EQ(result->at(0).key, 3);
    EXPECT_EQ(result->at(0).value, 4);
    EXPECT_EQ(result->at(1).key, 5);
    EXPECT_EQ(result->at(1).value, 6);
    EXPECT_EQ(result->at(2).key, 7);
    EXPECT_EQ(result->at(2).value, 8);
    EXPECT_EQ(result->at(3).key, 9);
    EXPECT_EQ(result->at(3).value, 10);
    EXPECT_EQ(result->at(4).key, 11);
    EXPECT_EQ(result->at(4).value, 12);
    EXPECT_EQ(result->at(5).key, 13);
    EXPECT_EQ(result->at(5).value, 14);
    EXPECT_EQ(result->at(6).key, 15);
    EXPECT_EQ(result->at(6).value, 16);
    EXPECT_EQ(result->at(7).key, 17);
    EXPECT_EQ(result->at(7).value, 18);
    EXPECT_EQ(result->at(8).key, 19);
    EXPECT_EQ(result->at(8).value, 20);
    EXPECT_EQ(result->at(9).key, 21);
    EXPECT_EQ(result->at(9).value, 22);

    delete result;
}

TEST(SstTest, ScanEmptyRangeTest)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Expect the getNumPagesInFile method to be called once, returning 0
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).Times(1).WillOnce(Return(0));

    // Call the scan method with an empty range
    vector<entry> *result = sortedList.scan(1, 6);

    // Check the result
    ASSERT_EQ(result->size(), 0);

    delete result;
}
