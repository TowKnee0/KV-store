#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#define private public
#include "../../io/io.h"
#include "ext_hash.h"
using namespace testing;

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
  MOCK_METHOD(void, del, (string filename));
  MOCK_METHOD(int, renameFile, (string source_filename, string dest_filename));
};

TEST(ExtHashTest, PutAndGetPage) {
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  ExtHash hashTable(4, 16, 0.75, 2, mockIOHandler);

  string filename = "testfile";
  int pageNumber = 1;
  vector<std::pair<int, int>> data = {{1, 100}, {2, 200}};

  hashTable.putPage(filename, pageNumber, data);

  auto retrievedData = hashTable.getPage(filename, pageNumber);

  ASSERT_TRUE(retrievedData.has_value());
  EXPECT_EQ(retrievedData.value(), data);
}

TEST(ExtHashTest, GetNonExistentPage) {
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  ExtHash hashTable(4, 16, 0.75, 2, mockIOHandler);

  string filename = "nonexistentfile";
  int pageNumber = 1;
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, pageNumber))
      .WillOnce(Return(nullopt));

  optional<vector<pair<int, int>>> retrievedData =
      hashTable.getPage(filename, pageNumber);

  ASSERT_FALSE(retrievedData.has_value());
}

TEST(ExtHashTest, GetNonExistentPageAndAdd) {
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  ExtHash hashTable(4, 16, 0.75, 2, mockIOHandler);

  string filename = "nonexistentfile";
  int pageNumber = 1;

  vector<pair<int, int>> data = {{1, 100}, {2, 200}};
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, pageNumber))
      .WillOnce(Return(data));

  // Got page from storage and put it into the table
  optional<vector<pair<int, int>>> retrievedData =
      hashTable.getPage(filename, pageNumber);

  ASSERT_TRUE(retrievedData.has_value());
  EXPECT_EQ(retrievedData.value(), data);
  // Should get straight from the table
  optional<vector<pair<int, int>>> retrievedData2 =
      hashTable.getPage(filename, pageNumber);

  ASSERT_TRUE(retrievedData2.has_value());
  EXPECT_EQ(retrievedData2.value(), data);
}

TEST(ExtHashTest, ExpandHappens) {
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  ExtHash hashTable(1, 16, .8, 2, mockIOHandler);

  // Table should expand when it reaches 80% full of the current size, where the
  // current_size is directory size * frame size.
  string filename = "testfile";
  int pageNumber = 1;
  vector<pair<int, int>> data = {{1, 100}, {2, 200}};
  // Starts off with 1 bit, which has 2 buckets.
  ASSERT_EQ(hashTable.directory.size(), 2);

  hashTable.putPage(filename, pageNumber, data);

  // Table has 1 item, 2 * 2 * .8 = 3.2 , so it should not expand yet.
  ASSERT_EQ(hashTable.directory.size(), 2);

  hashTable.putPage(filename, pageNumber + 1, data);
  // Table has 2 items, 2 * 2 * .8 = 3.2, so it should not expand.
  ASSERT_EQ(hashTable.directory.size(), 2);

  hashTable.putPage(filename, pageNumber + 2, data);
  // Table has 3 items, 2 * 2 * .8 = 3.2, so it should not expand.
  ASSERT_EQ(hashTable.directory.size(), 2);

  hashTable.putPage(filename, pageNumber + 3, data);
  // Table has 4 items, 2 * 2 * .8 = 3.2, so it should expand.
  ASSERT_EQ(hashTable.directory.size(), 4);
}


TEST(ExtHashTest, EvictPage) {
  shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
  string filename1 = "file1";
  int pageNumber1 = 1;
  vector<pair<int, int>> data1 = {{1, 100}};

  string filename2 = "file2";
  int pageNumber2 = 2;
  vector<pair<int, int>> data2 = {{2, 200}};

  string filename3 = "file3";
  int pageNumber3 = 3;
  vector<pair<int, int>> data3 = {{3, 300}};

  string filename4 = "file4";
  int pageNumber4 = 4;
  vector<pair<int, int>> data4 = {{4, 400}};

  string filename5 = "file5";
  int pageNumber5 = 5;
  vector<pair<int, int>> data5 = {{5, 500}};

  // We evict these pages, so when we try to get them, the IO handler will try to 
  // read them from storage.
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename1, pageNumber1)).Times(1);
  EXPECT_CALL(*mockIOHandler, readPageFromFile(filename3, pageNumber3)).Times(1);


  ExtHash hashTable(3, 3, 0.75, 1, mockIOHandler);

  hashTable.putPage(filename1, pageNumber1, data1);

  hashTable.putPage(filename2, pageNumber2, data2);

  hashTable.putPage(filename3, pageNumber3, data3);

  hashTable.putPage(filename4, pageNumber4, data4);

  hashTable.getPage(filename2, pageNumber2);

  hashTable.putPage(filename5, pageNumber5, data5);

  auto retrievedData1 = hashTable.getPage(filename1, pageNumber1);
  auto retrievedData2 = hashTable.getPage(filename2, pageNumber2);
  auto retrievedData3 = hashTable.getPage(filename3, pageNumber3);
  auto retrievedData4 = hashTable.getPage(filename4, pageNumber4);
  auto retrievedData5 = hashTable.getPage(filename5, pageNumber5);

  ASSERT_FALSE(retrievedData1.has_value());
  ASSERT_FALSE(retrievedData3.has_value());

  ASSERT_TRUE(retrievedData2.has_value());
  ASSERT_TRUE(retrievedData4.has_value());
  ASSERT_TRUE(retrievedData5.has_value());
  EXPECT_EQ(retrievedData2.value(), data2);
  EXPECT_EQ(retrievedData4.value(), data4);
  EXPECT_EQ(retrievedData5.value(), data5);
}

TEST(ExtHashTest, ShrinkDirectory){
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    ExtHash hashTable(1, 100, 0.8, 1, mockIOHandler);

    // Insert 100 pages into the hash table
    for (int i = 1; i <= 100; i++)
    {
        string filename = "file";
        int pageNumber = i;
        vector<std::pair<int, int>> data = {{i, i * 100}};
        hashTable.putPage(filename, pageNumber, data);
    }

    EXPECT_EQ(hashTable.item_count, 100);
    EXPECT_EQ(hashTable.directory.size(), 128);
    EXPECT_EQ(hashTable.directory_bit_size, 7);

    // Shrink the directory to only allow 17 pages
    hashTable.updateMaximumSizeInPages(17);

    EXPECT_LT(hashTable.item_count, 18); // 17 or less
    EXPECT_EQ(hashTable.directory.size(), 32);
    EXPECT_EQ(hashTable.directory_bit_size, 5);

}

TEST(ExtHashTest, DeletePage){
    shared_ptr<MockIOHandler> mockIOHandler = make_shared<MockIOHandler>();
    ExtHash hashTable(1, 100, 0.8, 1, mockIOHandler);

    string filename = "file";
    int pageNumber = 1;
    vector<pair<int, int>> data = {{1, 100}};

    // after we get it, it will try to get from storage.
    EXPECT_CALL(*mockIOHandler, readPageFromFile(filename, pageNumber)).WillOnce(Return(nullopt));
    EXPECT_CALL(*mockIOHandler, numPagesInFile(filename)).WillOnce(Return(1));

    hashTable.putPage(filename, pageNumber, data);

    EXPECT_EQ(hashTable.item_count, 1);

    hashTable.del(filename);

    hashTable.getPage(filename, pageNumber);
    
    for (int i = 1; i <= 100; i++)
    {
        hashTable.evict();
    }

    for (int i = 0; i < hashTable.clock.size(); i++)
    {
        ASSERT_EQ(hashTable.clock[i], nullptr);
    }
    

    EXPECT_EQ(hashTable.item_count, 0);
    EXPECT_EQ(hashTable.directory[0], nullptr);
}