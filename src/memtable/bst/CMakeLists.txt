enable_testing()

add_library(bst bst.cpp)
add_executable(bst_test bst.test.cpp)
target_link_libraries(bst_test PRIVATE bst GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(bst_test)