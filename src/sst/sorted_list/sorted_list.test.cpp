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
class MockIOHandler : public IO {
public:
    MOCK_METHOD(int, writeToFile, (const string filename, (const vector<pair<int, int>> &data)));
    MOCK_METHOD((optional<vector<pair<int, int>>>), readPageFromFile, (const string filename, int pageNumber));
    MOCK_METHOD(int, numPagesInFile, (const string filename));
    MOCK_METHOD(vector<string>, getDbFilenames, ());
};

// Tests for sst.cpp
TEST(SstTest, WriteTest) {
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