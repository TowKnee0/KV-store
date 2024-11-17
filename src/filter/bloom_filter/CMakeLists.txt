enable_testing()
add_library(bloom_filter bloom_filter.cpp)
target_link_libraries(bloom_filter)


add_executable(bloom_filter_test bloom_filter.test.cpp)
target_link_libraries(bloom_filter_test PRIVATE bloom_filter GTest::gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(bloom_filter_test)