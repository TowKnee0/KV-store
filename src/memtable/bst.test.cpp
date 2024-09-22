#include <catch2/catch_test_macros.hpp>

// This is only done because we need to verify the Bst balance property
#define private public
#include "bst.h"

TEST_CASE("Bst initialization") {
  Bst b(5);
  REQUIRE(b.max_size == 5);
  REQUIRE(b.c_size == 0);
}

TEST_CASE("Bst put") {
  Bst b(3);
  REQUIRE(b.put(1, 1) == EXIT_SUCCESS);
  REQUIRE(b.c_size == 1);

  REQUIRE(b.put(2, 1) == EXIT_SUCCESS);
  REQUIRE(b.c_size == 2);

  REQUIRE(b.put(3, 1) == EXIT_SUCCESS);
  REQUIRE(b.c_size == 3);

  REQUIRE(b.put(4, 1) == EXIT_FAILURE);
  REQUIRE(b.c_size == 3);

  REQUIRE(b.put(5, 1) == EXIT_FAILURE);
  REQUIRE(b.c_size == 3);
}

TEST_CASE("Bst get") {
  Bst b(3);
  REQUIRE(b.get(1) == nullopt);

  b.put(1, 2);
  REQUIRE(b.get(1) == 2);

  b.put(2, 3);
  REQUIRE(b.get(2) == 3);

  b.put(3, 4);
  REQUIRE(b.get(3) == 4);

  b.put(4, 5);
  REQUIRE(b.get(4) == nullopt);
}

TEST_CASE("Bst inOrder") {
  Bst b(5);
  b.put(1, 1);
  b.put(5, 5);
  b.put(2, 2);
  b.put(4, 4);
  b.put(3, 3);
  vector<pair<int, int>> vec = {
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
  };
  REQUIRE(b.inOrder() == vec);
}
