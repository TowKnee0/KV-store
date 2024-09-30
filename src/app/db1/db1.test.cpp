
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
};

// Mock for IO
class MockIO : public IO
{
public:
    MOCK_METHOD(int, writeToFile, (const string filename, (const vector<pair<int, int>> &data)));
    MOCK_METHOD((optional<vector<pair<int, int>>>), readPageFromFile, (const string filename, int pageNumber));
    MOCK_METHOD(int, numPagesInFile, (const string filename));
    MOCK_METHOD((vector<string>), getDbFilenames, ());
};

// Mock for SSTs
class MockSST : public Sst
{
public:
    MOCK_METHOD(int, write, ((const vector<pair<int, int>>) &data));
    MOCK_METHOD((optional<int>), get, (int key));
    MOCK_METHOD((vector<pair<int,int>>), scan, (int rangeStart, int rangeEnd));
};

// Test getting when the data is in the Memtable
TEST(Db1Test, GetFromMemtable)
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

