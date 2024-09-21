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
  REQUIRE(b.put(1, 1) == SUCCESS);
  REQUIRE(b.put(2, 1) == SUCCESS);
  REQUIRE(b.put(3, 1) == SUCCESS);
  REQUIRE(b.put(4, 1) == TREE_FULL);
} 