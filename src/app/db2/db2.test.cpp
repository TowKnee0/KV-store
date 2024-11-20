
#include "../../io/io.h"
#include "../../memtable/memtable.h"
#include "../../sst/sst.h"
#include "../../buffer/ext_hash/ext_hash.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <optional>
#define private public
#include "db2.h"
using namespace std;
using namespace testing;

// Mock for Memtable
class MockMemtable : public Memtable
{
public:
    MOCK_METHOD(int, put, (int key, int value));
    MOCK_METHOD((optional<int>), get, (int key));
    MOCK_METHOD((vector<pair<int, int>>), inOrder, ());
    MOCK_METHOD(int, count, ());
    MOCK_METHOD(void, clear, ());
    MOCK_METHOD((vector<pair<int, int>>), scan, (int rangeStart, int rangeEnd));
};

// Mock for IO
class MockIO : public IO
{
public:
    MOCK_METHOD(int, writeToFile, (const string filename, (const vector<pair<int, int>> &data), int pageOffset));
    MOCK_METHOD((optional<vector<pair<int, int>>>), readPageFromFile, (const string filename, int pageNumber));
    MOCK_METHOD(int, numPagesInFile, (const string filename));
    MOCK_METHOD((vector<string>), getDbFilenames, ());
    MOCK_METHOD(int, getPageSize, ());
    MOCK_METHOD(int, getPairSize, ());
    MOCK_METHOD(void, del, (string filename));
    MOCK_METHOD(int, renameFile, (string source_filename, string dest_filename));
};

// Mock for SSTs
class MockSST : public Sst
{
public:
    MOCK_METHOD(int, write, ((const vector<pair<int, int>>)&data));
    MOCK_METHOD((optional<int>), get, (int key));
    MOCK_METHOD((vector<int>), getKeyData, (int key));
    MOCK_METHOD(string, getFilename, ());
    MOCK_METHOD(int, getNumDataPages, ());
};

// Test getting when the data is in the Memtable
TEST(Db2Test, GetFromMemtable)
{
    // Create a mock Memtable
    unique_ptr<MockMemtable> mockMemtable = make_unique<MockMemtable>();

    // Create a Db2 instance
    Db2 db2(10, 1, 10, 1.5, 2, false);

    // Set the key
    int key = 1;

    // Set the value
    optional<int> value = 2;

    // Expect the get method to be called once, returning the value
    EXPECT_CALL(*mockMemtable, get(key)).Times(1).WillOnce(Return(value));

    db2.memtable = move(mockMemtable);

    // Call the get method
    optional<int> result = db2.get(key);
    EXPECT_EQ(result.value(), value.value());
}

// Test getting when the data is not in the Memtable, but rather the 3rd sst
TEST(Db2Test, GetFromThirdSST)
{
    // Create a mock Memtable
    unique_ptr<MockMemtable> mockMemtable = make_unique<MockMemtable>();

    // Create a mock IO
    shared_ptr<MockIO> mockIO = make_shared<MockIO>();

    // Create a Db2 instance
    Db2 db2(10, 1, 10, 1.5, 2, false);

    // Set the io_handler to the mock
    db2.io_handler = mockIO;

    // Set the key
    int key = 1;

    // Set the value
    optional<int> value = 2;

    // Expect the get method to be called once, returning nullopt
    EXPECT_CALL(*mockMemtable, get(key)).Times(1).WillOnce(Return(nullopt));

    db2.memtable = move(mockMemtable);

    // Create mock SSTs
    vector<unique_ptr<MockSST>> mockSSTs;
    for (int i = 0; i < 3; ++i)
    {
        mockSSTs.push_back(make_unique<MockSST>());
    }

    // Expect the get method to be called on the first two SSTs, returning nullopt
    EXPECT_CALL(*mockSSTs[2], get(key)).Times(1).WillOnce(Return(nullopt));
    EXPECT_CALL(*mockSSTs[1], get(key)).Times(1).WillOnce(Return(nullopt));

    // Expect the get method to be called on the third SST, returning the value
    EXPECT_CALL(*mockSSTs[0], get(key)).Times(1).WillOnce(Return(value));

    // Move the mock SSTs into the db2 instance
    for (auto &mockSST : mockSSTs)
    {
        db2.sst.push_back(move(mockSST));
    }

    // Call the get method
    optional<int> result = db2.get(key);
    EXPECT_EQ(result.value(), value.value());
}

TEST(Db1Test, Scan)
{
    // Create a mock Memtable
    unique_ptr<MockMemtable> mockMemtable = make_unique<MockMemtable>();

    // Create a mock IO
    shared_ptr<MockIO> mockIO = make_shared<MockIO>();

    // Create mock IOs calls for the SSTs
    vector<pair<int, int>> mock_data1_1 = {{1, 2}, {2, 3}, {3, 4}, {5, 6}, {31, 21}};
    EXPECT_CALL(*mockIO, readPageFromFile("filename0", 1)).Times(1).WillOnce(Return(mock_data1_1));
    EXPECT_CALL(*mockIO, numPagesInFile("filename0")).WillRepeatedly(Return(1));

    vector<pair<int, int>> mock_data2_1 = {{4, 5}, {6, 7}, {7, 8}, {8, 9}, {9, 10}, {10, 11}};
    vector<pair<int, int>> mock_data2_2 = {{11, 12}, {15, 16}};
    EXPECT_CALL(*mockIO, readPageFromFile("filename1", 1)).Times(1).WillOnce(Return(mock_data2_1));
    EXPECT_CALL(*mockIO, readPageFromFile("filename1", 2)).Times(1).WillOnce(Return(mock_data2_2));
    EXPECT_CALL(*mockIO, numPagesInFile("filename1")).WillRepeatedly(Return(2));

    vector<pair<int, int>> mock_data3_1 = {{13, 14}, {17, 18}, {19, 20}, {21, 22}};
    vector<pair<int, int>> mock_data3_2 = {{23, 24}, {25, 26}, {27, 28}};
    EXPECT_CALL(*mockIO, readPageFromFile("filename2", 1)).Times(1).WillOnce(Return(mock_data3_1));
    EXPECT_CALL(*mockIO, readPageFromFile("filename2", 2)).Times(1).WillOnce(Return(mock_data3_2));
    EXPECT_CALL(*mockIO, numPagesInFile("filename2")).WillRepeatedly(Return(2));

    unique_ptr<MockSST> mockSST1 = make_unique<MockSST>();
    unique_ptr<MockSST> mockSST2 = make_unique<MockSST>();
    unique_ptr<MockSST> mockSST3 = make_unique<MockSST>();

    EXPECT_CALL(*mockSST1, getFilename()).WillRepeatedly(Return("filename0"));
    EXPECT_CALL(*mockSST1, getNumDataPages()).WillRepeatedly(Return(1));
    EXPECT_CALL(*mockSST2, getFilename()).WillRepeatedly(Return("filename1"));
    EXPECT_CALL(*mockSST2, getNumDataPages()).WillRepeatedly(Return(2));

    EXPECT_CALL(*mockSST3, getFilename()).WillRepeatedly(Return("filename2"));
    EXPECT_CALL(*mockSST3, getNumDataPages()).WillRepeatedly(Return(2));

    EXPECT_CALL(*mockSST1, getKeyData(2)).WillRepeatedly(Return(vector<int>{1, 1, 2, 3}));
    EXPECT_CALL(*mockSST2, getKeyData(2)).WillRepeatedly(Return(vector<int>{0, 1, 4, 5}));
    EXPECT_CALL(*mockSST3, getKeyData(2)).WillRepeatedly(Return(vector<int>{0, 1, 13, 14}));

    // Create a Db1 instance
    Db2 db2(10, 1, 10, 1.5, 2, false);

    // Set the io_handler to the mock
    db2.io_handler = mockIO;

    shared_ptr<ExtHash> buffer = make_shared<ExtHash>(5, 5, .8, 1, mockIO);
    db2.buffer = buffer;

    // Move the mock SSTs into the db1 instance

    db2.sst.push_back(move(mockSST1));
    db2.sst.push_back(move(mockSST2));
    db2.sst.push_back(move(mockSST3));

    // Set the key range
    int key1 = 2;
    int key2 = 30;

    // Expect the scan method to be called once, returning the value
    EXPECT_CALL(*mockMemtable, scan(key1, key2)).Times(1).WillOnce(Return(vector<pair<int, int>>{{14, 15}, {40, 41}}));

    db2.memtable = move(mockMemtable);

    // Call the scan method
    vector<pair<int, int>> result = db2.scan(key1, key2);
    vector<pair<int, int>> expected_result = {{2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 8}, {8, 9}, {9, 10}, {10, 11}, {11, 12}, {13, 14}, {14, 15}, {15, 16}, {17, 18}, {19, 20}, {21, 22}, {23, 24}, {25, 26}, {27, 28}};
    EXPECT_EQ(result, expected_result);
}
