add_library(memtable bst.cpp)
add_executable(memtable_test bst.test.cpp)
target_link_libraries(memtable_test PRIVATE memtable GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(memtable_test)