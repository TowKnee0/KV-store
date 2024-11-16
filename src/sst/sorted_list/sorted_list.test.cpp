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

TEST(SstTest, ScanTest)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
    vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
    vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
    vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

    // Expect the getNumPagesInFile method to be called once, returning 4
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(4));

    // Expect the readPageFromFile method to be called four times
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).Times(AtLeast(1)).WillRepeatedly(Return(data1));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

    // Call the scan method
    vector<pair<int, int>> result = sortedList.scan(1, 20);

    // Check the result
    ASSERT_EQ(result.size(), 9);
    EXPECT_EQ(result.at(0).first, 3);
    EXPECT_EQ(result.at(0).second, 4);
    EXPECT_EQ(result.at(1).first, 5);
    EXPECT_EQ(result.at(1).second, 6);
    EXPECT_EQ(result.at(2).first, 7);
    EXPECT_EQ(result.at(2).second, 8);
    EXPECT_EQ(result.at(3).first, 9);
    EXPECT_EQ(result.at(3).second, 10);
    EXPECT_EQ(result.at(4).first, 11);
    EXPECT_EQ(result.at(4).second, 12);
    EXPECT_EQ(result.at(5).first, 13);
    EXPECT_EQ(result.at(5).second, 14);
    EXPECT_EQ(result.at(6).first, 15);
    EXPECT_EQ(result.at(6).second, 16);
    EXPECT_EQ(result.at(7).first, 17);
    EXPECT_EQ(result.at(7).second, 18);
    EXPECT_EQ(result.at(8).first, 19);
    EXPECT_EQ(result.at(8).second, 20);
}

TEST(SstTest, ScanTest2)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
    vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
    vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
    vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

    // Expect the getNumPagesInFile method to be called once, returning 4
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(4));

    // Expect the readPageFromFile method to be called four times
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).Times(AtLeast(1)).WillRepeatedly(Return(data1));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

    // Call the scan method
    vector<pair<int, int>> result = sortedList.scan(6, 20);

    // Check the result
    ASSERT_EQ(result.size(), 7);
    EXPECT_EQ(result.at(0).first, 7);
    EXPECT_EQ(result.at(0).second, 8);
    EXPECT_EQ(result.at(1).first, 9);
    EXPECT_EQ(result.at(1).second, 10);
    EXPECT_EQ(result.at(2).first, 11);
    EXPECT_EQ(result.at(2).second, 12);
    EXPECT_EQ(result.at(3).first, 13);
    EXPECT_EQ(result.at(3).second, 14);
    EXPECT_EQ(result.at(4).first, 15);
    EXPECT_EQ(result.at(4).second, 16);
    EXPECT_EQ(result.at(5).first, 17);
    EXPECT_EQ(result.at(5).second, 18);
    EXPECT_EQ(result.at(6).first, 19);
    EXPECT_EQ(result.at(6).second, 20);
}

TEST(SstTest, ScanTest3)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
    vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
    vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
    vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

    // Expect the getNumPagesInFile method to be called once, returning 4
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(4));

    // Expect the readPageFromFile method to be called four times
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

    // Call the scan method
    vector<pair<int, int>> result = sortedList.scan(8, 20);

    // Check the result
    ASSERT_EQ(result.size(), 6);
    EXPECT_EQ(result.at(0).first, 9);
    EXPECT_EQ(result.at(0).second, 10);
    EXPECT_EQ(result.at(1).first, 11);
    EXPECT_EQ(result.at(1).second, 12);
    EXPECT_EQ(result.at(2).first, 13);
    EXPECT_EQ(result.at(2).second, 14);
    EXPECT_EQ(result.at(3).first, 15);
    EXPECT_EQ(result.at(3).second, 16);
    EXPECT_EQ(result.at(4).first, 17);
    EXPECT_EQ(result.at(4).second, 18);
    EXPECT_EQ(result.at(5).first, 19);
    EXPECT_EQ(result.at(5).second, 20);
}

TEST(SstTest, ScanTest4)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
    vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
    vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
    vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

    // Expect the getNumPagesInFile method to be called once, returning 4
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(4));

    // Expect the readPageFromFile method to be called four times
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 3)).Times(AtLeast(1)).WillRepeatedly(Return(data3));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 4)).Times(AtLeast(1)).WillRepeatedly(Return(data4));

    // Call the scan method
    vector<pair<int, int>> result = sortedList.scan(19, 19);

    // Check the result
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result.at(0).first, 19);
    EXPECT_EQ(result.at(0).second, 20);
}

TEST(SstTest, ScanTest5)
{
    // Create a mock IOHandler
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    string filename = "test_sst";
    SortedList sortedList(filename, "test_db", mockIOHandler);

    // Create some data
    vector<pair<int, int>> data1 = {{3, 4}, {5, 6}};
    vector<pair<int, int>> data2 = {{7, 8}, {9, 10}, {11, 12}};
    vector<pair<int, int>> data3 = {{13, 14}, {15, 16}, {17, 18}};
    vector<pair<int, int>> data4 = {{19, 20}, {21, 22}, {23, 24}};

    // Expect the getNumPagesInFile method to be called once, returning 4
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillRepeatedly(Return(4));

    // Expect the readPageFromFile method to be called four times
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 1)).Times(AtLeast(1)).WillRepeatedly(Return(data1));
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, 2)).Times(AtLeast(1)).WillRepeatedly(Return(data2));

    // Call the scan method
    vector<pair<int, int>> result = sortedList.scan(1, 2);

    // Check the result
    ASSERT_EQ(result.size(), 0);
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
    vector<pair<int, int>> result = sortedList.scan(1, 6);

    // Check the result
    ASSERT_EQ(result.size(), 0);
}
