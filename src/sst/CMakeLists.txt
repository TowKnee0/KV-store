enable_testing()
add_library(sst sst.cpp)
target_link_libraries(sst io)

add_executable(sst_test sst.test.cpp)
target_link_libraries(sst_test PRIVATE sst GTest::gtest_main gmock io)

include(GoogleTest)
gtest_discover_tests(sst_test)