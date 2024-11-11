#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <optional>
#include <filesystem>
#include <fstream>

#define private public
#include "io_handler.h"
using namespace std;

class IOHandlerTest : public ::testing::Test {
protected:
    IOHandler* ioHandler;
    string dbname = "testdb";
    string filename = "testfile";

    void SetUp() override {
        ioHandler = new IOHandler(dbname);
        filesystem::remove_all("data/" + dbname);
    }

    void TearDown() override {
        delete ioHandler;
        filesystem::remove_all("data/" + dbname);
    }
};

TEST_F(IOHandlerTest, TestGetPageSize) {
    EXPECT_EQ(ioHandler->getPageSize(), 4096);
}

TEST_F(IOHandlerTest, TestGetPairSize) {
    EXPECT_EQ(ioHandler->getPairSize(), sizeof(int) * 2);
}

TEST_F(IOHandlerTest, TestConstructFilePath) {
    EXPECT_EQ(ioHandler->constructFilePath(), "data/" + dbname + "/");
}

TEST_F(IOHandlerTest, TestConstructFileName) {
    EXPECT_EQ(ioHandler->constructFileName(filename), filename + ".bin");
}

TEST_F(IOHandlerTest, TestWriteToFile) {
    vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};
    EXPECT_EQ(ioHandler->writeToFile(filename, data), 1);

    string fullPath = ioHandler->constructFilePath() + ioHandler->constructFileName(filename);
    EXPECT_TRUE(filesystem::exists(fullPath));
}

TEST_F(IOHandlerTest, TestWriteToFileAtPage) {
    vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};
    EXPECT_EQ(ioHandler->writeToFileAtPage(filename, data, 1), 1);

    string fullPath = ioHandler->constructFilePath() + ioHandler->constructFileName(filename);
    EXPECT_TRUE(filesystem::exists(fullPath));
}

TEST_F(IOHandlerTest, TestReadPageFromFile) {
    vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};
    ioHandler->writeToFile(filename, data);

    optional<vector<pair<int, int>>> readData = ioHandler->readPageFromFile(filename, 1);
    ASSERT_TRUE(readData.has_value());
    EXPECT_EQ(readData.value(), data);
}

TEST_F(IOHandlerTest, TestGetDbFilenames) {
    vector<pair<int, int>> data = {{1, 2}, {3, 4}, {5, 6}};
    ioHandler->writeToFile(filename, data);

    vector<string> filenames = ioHandler->getDbFilenames();
    EXPECT_EQ(filenames.size(), 1);
    EXPECT_EQ(filenames[0], filename + ".bin");
}

TEST_F(IOHandlerTest, TestNumPagesInFile) {
    vector<pair<int, int>> data;
    for (int i = 0; i < ioHandler->getPageSize() / sizeof(pair<int, int>); ++i) {
        data.push_back({i, i});
    }

    for (int i = 1; i < 6; ++i) {
        ioHandler->writeToFileAtPage(filename, data, i);
    }

    EXPECT_EQ(ioHandler->numPagesInFile(filename), 5);
}
TEST_F(IOHandlerTest, TestWriteAndReadFile) {
    vector<pair<int, int>> data;
    for (int i = 0; i < ioHandler->getPageSize() / sizeof(pair<int, int>); ++i) {
        data.push_back({i, i});
    }

    for (int i = 1; i < 6; ++i) {
        ioHandler->writeToFileAtPage(filename, data, i);
    }

    // Read the data back and make sure it is correct.
    for (int i = 1; i < 6; ++i) {
        optional<vector<pair<int, int>>> readData = ioHandler->readPageFromFile(filename, i);
        ASSERT_TRUE(readData.has_value());
        EXPECT_EQ(readData.value(), data);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}