enable_testing()
add_library(sorted_list sorted_list.cpp)
target_link_libraries(sorted_list io_handler)

add_executable(sorted_list_test sorted_list.test.cpp)
target_link_libraries(sorted_list_test PRIVATE sorted_list GTest::gtest_main gmock io_handler)

include(GoogleTest)
gtest_discover_tests(sorted_list_test)