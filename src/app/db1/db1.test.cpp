
#include "../../io/io.h"
#include "../../memtable/memtable.h"
#include "../../sst/sst.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <optional>
#define private public
#include "db1.h"
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
TEST(Db1Test, GetFromMemtable)
{
    // Create a mock Memtable
    unique_ptr<MockMemtable> mockMemtable = make_unique<MockMemtable>();

    // Create a Db1 instance
    Db1 db1(10);

    // Set the key
    int key = 1;

    // Set the value
    optional<int> value = 2;

    // Expect the get method to be called once, returning the value
    EXPECT_CALL(*mockMemtable, get(key)).Times(1).WillOnce(Return(value));

    db1.memtable = move(mockMemtable);

    // Call the get method
    optional<int> result = db1.get(key);
    EXPECT_EQ(result.value(), value.value());
}

// Test getting when the data is not in the Memtable, but rather the 3rd sst
TEST(Db1Test, GetFromThirdSST)
{
    // Create a mock Memtable
    unique_ptr<MockMemtable> mockMemtable = make_unique<MockMemtable>();

    // Create a mock IO
    shared_ptr<MockIO> mockIO = make_shared<MockIO>();

    // Create a Db1 instance
    Db1 db1(10);

    // Set the io_handler to the mock
    db1.io_handler = mockIO;

    // Set the key
    int key = 1;

    // Set the value
    optional<int> value = 2;

    // Expect the get method to be called once, returning nullopt
    EXPECT_CALL(*mockMemtable, get(key)).Times(1).WillOnce(Return(nullopt));

    db1.memtable = move(mockMemtable);

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

    // Move the mock SSTs into the db1 instance
    for (auto &mockSST : mockSSTs)
    {
        db1.sst.push_back(move(mockSST));
    }

    // Call the get method
    optional<int> result = db1.get(key);
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

    vector<pair<int, int>> mock_data2_1 = {{4, 5}, {6, 7}, {7, 8}, {8, 9}, {9, 10}, {10, 11}};
    vector<pair<int, int>> mock_data2_2 = {{11, 12}, {15, 16}};
    EXPECT_CALL(*mockIO, readPageFromFile("filename1", 1)).Times(1).WillOnce(Return(mock_data2_1));
    EXPECT_CALL(*mockIO, readPageFromFile("filename1", 2)).Times(1).WillOnce(Return(mock_data2_2));

    vector<pair<int, int>> mock_data3_1 = {{13, 14}, {17, 18}, {19, 20}, {21, 22}};
    vector<pair<int, int>> mock_data3_2 = {{23, 24}, {25, 26}, {27, 28}};
    EXPECT_CALL(*mockIO, readPageFromFile("filename2", 1)).Times(1).WillOnce(Return(mock_data3_1));
    EXPECT_CALL(*mockIO, readPageFromFile("filename2", 2)).Times(1).WillOnce(Return(mock_data3_2));

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
    Db1 db1(10);

    // Set the io_handler to the mock
    db1.io_handler = mockIO;

    // Move the mock SSTs into the db1 instance

    db1.sst.push_back(move(mockSST1));
    db1.sst.push_back(move(mockSST2));
    db1.sst.push_back(move(mockSST3));

    // Set the key range
    int key1 = 2;
    int key2 = 30;

    // Expect the scan method to be called once, returning the value
    EXPECT_CALL(*mockMemtable, scan(key1, key2)).Times(1).WillOnce(Return(vector<pair<int, int>>{{14, 15}, {40, 41}}));

    db1.memtable = move(mockMemtable);

    // Call the scan method
    vector<pair<int, int>> result = db1.scan(key1, key2);
    vector<pair<int, int>> expected_result = {{2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 8}, {8, 9}, {9, 10}, {10, 11}, {11, 12}, {13, 14}, {14, 15}, {15, 16}, {17, 18}, {19, 20}, {21, 22}, {23, 24}, {25, 26}, {27, 28}};
    EXPECT_EQ(result, expected_result);
}

TEST(Db1Test, NoMockTests)
{
    // Create a Db1 instance
    Db1 db1(10);
    db1.open("no_mock_test");

    for (string filename : db1.io_handler->getDbFilenames())
    {
        db1.io_handler->del(filename);
    }


    // for (int i = 0; i < 1000; i++)
    // {
    //     db1.put(i, i + 1);
    // }

    // for (int i = 0; i < 100; i++)
    // {
    //     optional<int> result = db1.get(i);
    //     EXPECT_EQ(result.value(), i + 1);
    // }

    // // scan for 5 - 9
    // vector<pair<int, int>> result = db1.scan(5, 9);
    // vector<pair<int, int>> expected_result = {{5, 6}, {6, 7}, {7, 8}, {8, 9}, {9, 10}};
    // EXPECT_EQ(result, expected_result);

    // db1.close();
   
    
}