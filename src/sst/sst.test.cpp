#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#define private public
#include "sst.h"
#include <vector>
#include <optional>
#include <utility>
#include <string>
#include <iostream>

// Mocking the IO functions
int writeToFileMock(const string &dbname, const string &filename, const vector<pair<int, int>> &data)
{
    return 0; // Assume success
}

int numPagesInFileMock(const string &dbname, const string &filename)
{
    return 3; // Assume there are 3 pages
}

optional<vector<pair<int, int>>> readPageFromFileMock(const string &dbname, const string &filename, int page)
{
    cout << "Testing" << endl;

    if (page == 1)
    {
        return vector<pair<int, int>>{{1, 10}, {2, 20}, {3, 30}};
    }
    else if (page == 2)
    {
        cout << "Reading page 2" << endl;
        return vector<pair<int, int>>{{4, 40}, {5, 50}, {6, 60}};
    }
    else if (page == 3)
    {
        return vector<pair<int, int>>{{7, 70}, {8, 80}, {9, 90}};
    }
    return nullopt; // Error case
}

TEST_CASE("Sst::get", "[get]")
{
    Sst sst("testfile", "testdb");

    SECTION("Key exists in the first page")
    {
        auto result = sst.get(2);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 20);
    }

    SECTION("Key exists in the second page")
    {
        auto result = sst.get(5);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 50);
    }

    SECTION("Key exists in the third page")
    {
        auto result = sst.get(8);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 80);
    }

    SECTION("Key does not exist")
    {
        auto result = sst.get(10);
        REQUIRE(!result.has_value());
    }
}

TEST_CASE("Sst::scan", "[scan]")
{
    Sst sst("testfile", "testdb");

    SECTION("Range within a single page")
    {
        auto result = sst.scan(2, 4);
        REQUIRE(result != nullptr);
        REQUIRE(result->size() == 2);
        REQUIRE(result->at(0).key == 3);
        REQUIRE(result->at(0).value == 30);
        delete result;
    }

    SECTION("Range across multiple pages")
    {
        auto result = sst.scan(3, 7);
        REQUIRE(result != nullptr);
        REQUIRE(result->size() == 4);
        REQUIRE(result->at(0).key == 4);
        REQUIRE(result->at(0).value == 40);
        REQUIRE(result->at(1).key == 5);
        REQUIRE(result->at(1).value == 50);
        REQUIRE(result->at(2).key == 6);
        REQUIRE(result->at(2).value == 60);
        delete result;
    }

    SECTION("Range exceeds available keys")
    {
        auto result = sst.scan(8, 12);
        REQUIRE(result != nullptr);
        REQUIRE(result->size() == 2);
        REQUIRE(result->at(0).key == 9);
        REQUIRE(result->at(0).value == 90);
        delete result;
    }

    SECTION("Range with no keys")
    {
        auto result = sst.scan(10, 12);
        REQUIRE(result != nullptr);
        REQUIRE(result->empty());
        delete result;
    }
}