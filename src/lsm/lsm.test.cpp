#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "lsm.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

// #define private public

using namespace std;

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

class MockBuffer : public Buffer
{
public:
    MOCK_METHOD((optional<vector<pair<int, int>>>), getPage, (const string filename, int pageNumber));
    MOCK_METHOD(void, putPage, (const string filename, int pageNumber, (vector<pair<int, int>> data), bool dirty));
    MOCK_METHOD(void, updateMaximumSizeInPages, (int newMaximumSizeInPages));
    MOCK_METHOD(void, del, (string filename, int pagenumber, bool delete_file));
    MOCK_METHOD(int, rename, (string old_name, string new_name, int pageNumber));
};

class MockBTree : public BTree
{
public:
    MockBTree(string filename, string dbname, shared_ptr<IO> io_handler, shared_ptr<Buffer> buffer, bool use_binary_search = false)
        : BTree(filename, dbname, io_handler, buffer, use_binary_search) {}
    MOCK_METHOD(void, rename, (string filename));
    MOCK_METHOD(int, write, ((const vector<pair<int, int>> &data)));
    MOCK_METHOD(optional<int>, get, (int key));
    MOCK_METHOD(int, mergeBTrees, (string filename1, string filename2));
};

class MockBTreeFactory : public BTreeFactory
{
public:
    MOCK_METHOD(shared_ptr<BTree>, create, (string name, string dbname, shared_ptr<IO> io, shared_ptr<Buffer> buffer, shared_ptr<Filter> filter));
};

// TEST(LsmTest, PutInsertsSuccessfullyIntoMemtable) {
//     auto mockMemtable = make_shared<MockMemtable>();
//     auto mockIO = make_shared<MockIO>();
//     auto mockBuffer = make_shared<MockBuffer>(); // Assume Buffer is not mocked.
//     auto mockFactory = make_shared<MockBTreeFactory>();

//     EXPECT_CALL(*mockMemtable, put(1, 100)).WillOnce(Return(EXIT_SUCCESS)); // Simulate successful insertion.

//     Lsm lsm("test_db", mockBuffer, mockMemtable, mockIO, mockFactory);

//     EXPECT_NO_THROW(lsm.put(1, 100)); // Verify no exceptions are thrown.
// }

// TEST(LsmTest, PutFlushesMemtableAndCreatesBTree) {
//     auto mockMemtable = make_shared<MockMemtable>();
//     auto mockIO = make_shared<MockIO>();
//     auto mockBuffer = make_shared<MockBuffer>();
//     auto mockFactory = make_shared<MockBTreeFactory>();

//     // Simulate memtable overflow.
//     EXPECT_CALL(*mockMemtable, put(1, 100))
//         .WillOnce(Return(EXIT_FAILURE)).WillOnce(Return(EXIT_SUCCESS));
//     EXPECT_CALL(*mockMemtable, inOrder())
//         .WillOnce(Return(vector<pair<int, int>>{{1, 100}}));
//     EXPECT_CALL(*mockMemtable, clear())
//         .Times(1);

//     EXPECT_CALL(*mockIO, numPagesInFile("temp"))
//         .WillRepeatedly(Return(-1));
//     EXPECT_CALL(*mockIO, getPageSize())
//         .WillRepeatedly(Return(4096));
//     EXPECT_CALL(*mockIO, getPairSize())
//         .WillRepeatedly(Return(16));

//     // Simulate BTree creation and data write.
//     auto mockBTree = make_shared<MockBTree>("temp", "test_db", mockIO, mockBuffer);
//     EXPECT_CALL(*mockFactory, create("temp", "test_db", static_pointer_cast<IO>(mockIO), static_pointer_cast<Buffer>(mockBuffer)))
//         .WillOnce(Return(mockBTree));
//     EXPECT_CALL(*mockBTree, write(vector<pair<int, int>>{{1, 100}}))
//         .Times(1);
//     EXPECT_CALL(*mockBTree, rename("L1"))
//         .Times(1);

//     Lsm lsm("test_db", mockBuffer, mockMemtable, mockIO, mockFactory);

//     EXPECT_NO_THROW(lsm.put(1, 100));
// }

TEST(LsmTest, PutHandlesBTreeMerge)
{
    auto mockMemtable = make_shared<MockMemtable>();
    auto mockIO = make_shared<MockIO>();
    auto mockBuffer = make_shared<MockBuffer>();
    auto mockFactory = make_shared<MockBTreeFactory>();

    // Simulate memtable overflow.
    EXPECT_CALL(*mockMemtable, put(1, 100))
        .WillOnce(Return(EXIT_FAILURE))
        .WillOnce(Return(EXIT_SUCCESS));
    EXPECT_CALL(*mockMemtable, inOrder())
        .WillOnce(Return(vector<pair<int, int>>{{1, 100}}));
    EXPECT_CALL(*mockMemtable, clear())
        .Times(1);

    EXPECT_CALL(*mockIO, numPagesInFile(::testing::_))
        .WillRepeatedly(Return(-1));
    EXPECT_CALL(*mockIO, getPageSize())
        .WillRepeatedly(Return(4096));
    EXPECT_CALL(*mockIO, getPairSize())
        .WillRepeatedly(Return(16));

    // Simulate BTree creation and merge logic.
    auto mockBTree1 = make_shared<MockBTree>("L1", "test_db", mockIO, mockBuffer);
    auto mockBTree2 = make_shared<MockBTree>("temp", "test_db", mockIO, mockBuffer);
    auto mockBTree3 = make_shared<MockBTree>("merged", "test_db", mockIO, mockBuffer);

    EXPECT_CALL(*mockFactory, create("temp", "test_db", static_pointer_cast<IO>(mockIO), static_pointer_cast<Buffer>(mockBuffer), _))
        .WillOnce(Return(mockBTree2));
    EXPECT_CALL(*mockBTree2, write(vector<pair<int, int>>{{1, 100}}))
        .Times(1);
    EXPECT_CALL(*mockFactory, create("merged", "test_db", static_pointer_cast<IO>(mockIO), static_pointer_cast<Buffer>(mockBuffer), _))
        .WillOnce(Return(mockBTree3));

    EXPECT_CALL(*mockBTree3, mergeBTrees("L1", "temp"))
        .WillOnce(Return(0));
    EXPECT_CALL(*mockIO, del("temp"))
        .Times(1);
    EXPECT_CALL(*mockIO, del("L1"))
        .Times(1);
    EXPECT_CALL(*mockBTree3, rename("temp"))
        .WillOnce([mockBTree3](string name)
                  { mockBTree3->filename = name; });
    EXPECT_CALL(*mockBTree3, rename("L2"))
        .WillOnce([mockBTree3](string name)
                  { mockBTree3->filename = name; });

    Lsm lsm("test_db", mockBuffer, mockMemtable, mockIO, mockFactory, 10);
    lsm.trees.push_back(mockBTree1); // Simulate an existing BTree in level 1.

    EXPECT_NO_THROW(lsm.put(1, 100));
    ASSERT_EQ(2, lsm.trees.size());
    ASSERT_EQ(lsm.trees[0], nullptr);
    ASSERT_NE(lsm.trees[1], nullptr);
    ASSERT_EQ(lsm.trees[1]->filename, "L2");
}

TEST(LsmTest, PutfreeSpot)
{
    auto mockMemtable = make_shared<MockMemtable>();
    auto mockIO = make_shared<MockIO>();
    auto mockBuffer = make_shared<MockBuffer>();
    auto mockFactory = make_shared<MockBTreeFactory>();

    // Simulate memtable overflow.
    EXPECT_CALL(*mockMemtable, put(1, 100))
        .WillOnce(Return(EXIT_FAILURE))
        .WillOnce(Return(EXIT_SUCCESS));
    EXPECT_CALL(*mockMemtable, inOrder())
        .WillOnce(Return(vector<pair<int, int>>{{1, 100}}));
    EXPECT_CALL(*mockMemtable, clear())
        .Times(1);

    EXPECT_CALL(*mockIO, numPagesInFile(::testing::_))
        .WillRepeatedly(Return(-1));
    EXPECT_CALL(*mockIO, getPageSize())
        .WillRepeatedly(Return(4096));
    EXPECT_CALL(*mockIO, getPairSize())
        .WillRepeatedly(Return(16));

    // Simulate BTree creation and merge logic.
    auto mockBTree1 = make_shared<MockBTree>("L1", "test_db", mockIO, mockBuffer);
    auto mockBTree2 = make_shared<MockBTree>("temp", "test_db", mockIO, mockBuffer);
    auto mockBTree3 = make_shared<MockBTree>("merged", "test_db", mockIO, mockBuffer);
    auto mockBTree4 = make_shared<MockBTree>("L3", "test_db", mockIO, mockBuffer);

    EXPECT_CALL(*mockFactory, create("temp", "test_db", static_pointer_cast<IO>(mockIO), static_pointer_cast<Buffer>(mockBuffer), _))
        .WillOnce(Return(mockBTree2));
    EXPECT_CALL(*mockBTree2, write(vector<pair<int, int>>{{1, 100}}))
        .Times(1);
    EXPECT_CALL(*mockFactory, create("merged", "test_db", static_pointer_cast<IO>(mockIO), static_pointer_cast<Buffer>(mockBuffer), _))
        .WillOnce(Return(mockBTree3));

    EXPECT_CALL(*mockBTree3, mergeBTrees("L1", "temp"))
        .WillOnce(Return(0));
    EXPECT_CALL(*mockIO, del("temp"))
        .Times(1);
    EXPECT_CALL(*mockIO, del("L1"))
        .Times(1);
    EXPECT_CALL(*mockBTree3, rename("temp"))
        .WillOnce([mockBTree3](string name)
                  { mockBTree3->filename = name; });
    EXPECT_CALL(*mockBTree3, rename("L2"))
        .WillOnce([mockBTree3](string name)
                  { mockBTree3->filename = name; });

    Lsm lsm("test_db", mockBuffer, mockMemtable, mockIO, mockFactory, 10);
    lsm.trees.push_back(mockBTree1); // Simulate an existing BTree in level 1.
    lsm.trees.push_back(nullptr);
    lsm.trees.push_back(mockBTree4);

    EXPECT_NO_THROW(lsm.put(1, 100));
    ASSERT_EQ(3, lsm.trees.size());

    ASSERT_EQ(lsm.trees[0], nullptr);
    ASSERT_NE(lsm.trees[1], nullptr);
    ASSERT_NE(lsm.trees[2], nullptr);

    ASSERT_EQ(lsm.trees[1]->filename, "L2");
    ASSERT_EQ(lsm.trees[2]->filename, "L3");
}