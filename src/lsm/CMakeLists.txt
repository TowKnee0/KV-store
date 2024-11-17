enable_testing()

add_library(lsm lsm.cpp)
target_link_libraries(lsm btree GTest::gtest GTest::gtest_main)

add_executable(lsm_test lsm.test.cpp)
target_link_libraries(lsm_test PRIVATE lsm GTest::gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(lsm_test)