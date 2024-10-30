#include <iostream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define private public
#include "ext_hash.h"
using namespace testing;

TEST(ExtHashTest, PutAndGetPage)
{
    ExtHash hashTable(4, 16, 0.75, 2);

    std::string filename = "testfile";
    int pageNumber = 1;
    std::vector<std::pair<int, int>> data = {{1, 100}, {2, 200}};

    hashTable.putPage(filename, pageNumber, data);

    auto retrievedData = hashTable.getPage(filename, pageNumber);

    ASSERT_TRUE(retrievedData.has_value());
    EXPECT_EQ(retrievedData.value(), data);
}

TEST(ExtHashTest, GetNonExistentPage)
{
    ExtHash hashTable(4, 16, 0.75, 2);

    std::string filename = "nonexistentfile";
    int pageNumber = 1;

    auto retrievedData = hashTable.getPage(filename, pageNumber);

    ASSERT_FALSE(retrievedData.has_value());
}

TEST(ExtHashTest, HandleOverflow)
{
    // Using a small max page size, we end up getting overflow values which require
    // evicting pages, so we set it high.
    ExtHash hashTable(2, 32, 0.75, 2);

    std::string filename1 = "file1";
    int pageNumber1 = 1;
    std::vector<std::pair<int, int>> data1 = {{1, 100}, {2, 200}};

    std::string filename2 = "file2";
    int pageNumber2 = 2;
    std::vector<std::pair<int, int>> data2 = {{3, 300}, {4, 400}};

    std::string filename3 = "file3";
    int pageNumber3 = 3;
    std::vector<std::pair<int, int>> data3 = {{5, 500}, {6, 600}};

    std::string filename4 = "file4";
    int pageNumber4 = 4;
    std::vector<std::pair<int, int>> data4 = {{7, 700}, {8, 800}};

    hashTable.putPage(filename1, pageNumber1, data1);
    hashTable.putPage(filename2, pageNumber2, data2);
    hashTable.putPage(filename3, pageNumber3, data3);
    hashTable.putPage(filename4, pageNumber4, data4);

    auto retrievedData1 = hashTable.getPage(filename1, pageNumber1);
    auto retrievedData2 = hashTable.getPage(filename2, pageNumber2);
    auto retrievedData3 = hashTable.getPage(filename3, pageNumber3);
    auto retrievedData4 = hashTable.getPage(filename4, pageNumber4);

    ASSERT_TRUE(retrievedData1.has_value());
    EXPECT_EQ(retrievedData1.value(), data1);

    ASSERT_TRUE(retrievedData2.has_value());
    EXPECT_EQ(retrievedData2.value(), data2);

    ASSERT_TRUE(retrievedData3.has_value());
    EXPECT_EQ(retrievedData3.value(), data3);

    ASSERT_TRUE(retrievedData4.has_value());
    EXPECT_EQ(retrievedData4.value(), data4);
}

TEST(ExtHashTest, EvictPage)
{
    ExtHash hashTable(2, 2, 0.75, 2);

    string first_index_0 = "none";
    string first_index_1 = "none";

    std::string filename1 = "file1";
    int pageNumber1 = 1;
    std::vector<std::pair<int, int>> data1 = {{1, 100}, {2, 200}};
    hashTable.putPage(filename1, pageNumber1, data1);

    if (hashTable.directory[0]->frame_in_use == true)
    {
        first_index_0 = hashTable.directory[0]->filename;
    }
    else if (hashTable.directory[1]->frame_in_use == true)
    {
        first_index_1 = hashTable.directory[1]->filename;
    }

    std::string filename2 = "file2";
    int pageNumber2 = 2;
    std::vector<std::pair<int, int>> data2 = {{3, 300}, {4, 400}};
    hashTable.putPage(filename2, pageNumber2, data2);

    if (hashTable.directory[0]->frame_in_use == true && first_index_0 == "none")
    {
        first_index_0 = hashTable.directory[0]->filename;
    }
    else if (hashTable.directory[1]->frame_in_use == true && first_index_1 == "none")
    {
        first_index_1 = hashTable.directory[1]->filename;
    }

    std::string filename3 = "file3";
    int pageNumber3 = 3;
    std::vector<std::pair<int, int>> data3 = {{5, 500}, {6, 600}};
    hashTable.putPage(filename3, pageNumber3, data3);

    if (hashTable.directory[0]->frame_in_use == true && first_index_0 == "none")
    {
        first_index_0 = hashTable.directory[0]->filename;
    }
    else if (hashTable.directory[1]->frame_in_use == true && first_index_1 == "none")
    {
        first_index_1 = hashTable.directory[1]->filename;
    }

    std::string filename4 = "file4";
    int pageNumber4 = 4;
    std::vector<std::pair<int, int>> data4 = {{5, 500}, {6, 600}};
    hashTable.putPage(filename4, pageNumber4, data4);

    if (hashTable.directory[0]->frame_in_use == true && first_index_0 == "none")
    {
        first_index_0 = hashTable.directory[0]->filename;
    }
    else if (hashTable.directory[1]->frame_in_use == true && first_index_1 == "none")
    {
        first_index_1 = hashTable.directory[1]->filename;
    }

    std::string filename5 = "file5";
    int pageNumber5 = 5;
    std::vector<std::pair<int, int>> data5 = {{5, 500}, {6, 600}};
    hashTable.putPage(filename5, pageNumber5, data5);

    if (hashTable.directory[0]->frame_in_use == true && first_index_0 == "none")
    {
        first_index_0 = hashTable.directory[0]->filename;
    }
    else if (hashTable.directory[1]->frame_in_use == true && first_index_1 == "none")
    {
        first_index_1 = hashTable.directory[1]->filename;
    }

    bool index_0_same = first_index_0 == hashTable.directory[0]->filename;
    bool index_1_same = first_index_1 == hashTable.directory[1]->filename;
    bool atleast_not_one_same = !(index_0_same && index_1_same);
    ASSERT_TRUE(atleast_not_one_same);
}

TEST(ExtHashTest, ShrinkDirectory)
{
    ExtHash hashTable(4, 64, 0.75, 2);

    std::string filename1 = "file1";
    int pageNumber1 = 1;
    std::vector<std::pair<int, int>> data1 = {{1, 100}, {2, 200}};

    std::string filename2 = "file2";
    int pageNumber2 = 2;
    std::vector<std::pair<int, int>> data2 = {{3, 300}, {4, 400}};

    std::string filename3 = "file3";
    int pageNumber3 = 3;
    std::vector<std::pair<int, int>> data3 = {{5, 500}, {6, 600}};

    std::string filename4 = "file4";
    int pageNumber4 = 4;
    std::vector<std::pair<int, int>> data4 = {{7, 700}, {8, 800}};

    hashTable.putPage(filename1, pageNumber1, data1);
    hashTable.putPage(filename2, pageNumber2, data2);
    hashTable.putPage(filename3, pageNumber3, data3);
    hashTable.putPage(filename4, pageNumber4, data4);

    // Shrink the directory to a smaller size
    hashTable.updateMaximumSizeInPages(2);

    auto retrievedData1 = hashTable.getPage(filename1, pageNumber1);
    auto retrievedData2 = hashTable.getPage(filename2, pageNumber2);
    auto retrievedData3 = hashTable.getPage(filename3, pageNumber3);
    auto retrievedData4 = hashTable.getPage(filename4, pageNumber4);

    int new_directory_size = hashTable.directory.size();
    ASSERT_EQ(new_directory_size, 2);
}

TEST(ExtHashTest, ShrinkDirectoryBig)
{
    ExtHash hashTable(5, 64, 0.75, 1);

    // Insert 64 pages into the hash table
    for (int i = 1; i <= 64; i++)
    {
        std::string filename = "file";
        int pageNumber = i;
        std::vector<std::pair<int, int>> data = {{i, i * 100}};
        hashTable.putPage(filename, pageNumber, data);
    }

    // Shrink the directory to only allow 8 pages
    hashTable.updateMaximumSizeInPages(8);

    // with a maximum of 8 pages, and 1 per index, we should have 8 indices

    // Check that the directory size is now 8
    int new_directory_size = hashTable.directory.size();
    ASSERT_EQ(new_directory_size, 8);
}

TEST(ExtHashTest, MaximumPagesLimit)
{
    ExtHash hashTable(2, 2, 0.75, 2);

    // 2 maximum pages is 1 bit, which is 2 indices, which in theory can support 4 pages,
    // ensure that we only allow 2 pages to be stored.

    std::string filename1 = "file1";
    int pageNumber1 = 1;
    std::vector<std::pair<int, int>> data1 = {{1, 100}, {2, 200}};

    std::string filename2 = "file2";
    int pageNumber2 = 2;
    std::vector<std::pair<int, int>> data2 = {{3, 300}, {4, 400}};

    std::string filename3 = "file3";
    int pageNumber3 = 3;
    std::vector<std::pair<int, int>> data3 = {{5, 500}, {6, 600}};

    std::string filename4 = "file4";
    int pageNumber4 = 4;
    std::vector<std::pair<int, int>> data4 = {{7, 700}, {8, 800}};

    std::string filename5 = "file5";
    int pageNumber5 = 5;
    std::vector<std::pair<int, int>> data5 = {{9, 900}, {10, 1000}};

    hashTable.putPage(filename1, pageNumber1, data1);
    hashTable.putPage(filename2, pageNumber2, data2);
    hashTable.putPage(filename3, pageNumber3, data3);
    hashTable.putPage(filename4, pageNumber4, data4);
    hashTable.putPage(filename5, pageNumber5, data5);

    int directory_size = hashTable.directory.size();
    ASSERT_EQ(directory_size, 2);

    int non_empty_frame_count = 0;
    for (int i = 0; i < hashTable.directory.size(); i++)
    {
        auto current_frame = hashTable.directory[i];
        if (current_frame->frame_in_use == false)
        {
            continue;
        }
        while (current_frame != nullptr)
        {
            non_empty_frame_count++;
            current_frame = current_frame->next;
        }
    }
    ASSERT_EQ(non_empty_frame_count, 2);
}

TEST(ExtHashTest, UpdatePageSetsDirty)
{
    ExtHash hashTable(4, 16, 0.75, 2);

    std::string filename = "testfile";
    int pageNumber = 1;
    std::vector<std::pair<int, int>> data = {{1, 100}, {2, 200}};
    std::vector<std::pair<int, int>> newData = {{1, 150}, {2, 250}};

    // Insert the initial page
    hashTable.putPage(filename, pageNumber, data);

    // Update the page with new data
    hashTable.putPage(filename, pageNumber, newData);

    // Retrieve the updated page
    auto retrievedData = hashTable.getPage(filename, pageNumber);

    // Check that the data was updated
    ASSERT_TRUE(retrievedData.has_value());
    EXPECT_EQ(retrievedData.value(), newData);

    // Check that the frame is marked as dirty
    uint32_t index = hashTable.GetIndexFromHashAndBitSize(hashTable.getHash(filename, pageNumber), hashTable.directory_bit_size);
    shared_ptr<frame> current_frame = hashTable.directory[index];
    while (current_frame != nullptr)
    {
        if (current_frame->filename == filename && current_frame->pageNumber == pageNumber)
        {
            EXPECT_TRUE(current_frame->dirty);
            break;
        }
        current_frame = current_frame->next;
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
