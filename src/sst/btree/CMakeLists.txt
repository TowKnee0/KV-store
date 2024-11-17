enable_testing()
add_library(btree btree.cpp)
target_link_libraries(btree io_handler bloom_filter ext_hash)

add_executable(btree_test btree.test.cpp)
target_link_libraries(btree_test PRIVATE btree bloom_filter GTest::gtest_main gmock io_handler)

include(GoogleTest)
gtest_discover_tests(btree_test)