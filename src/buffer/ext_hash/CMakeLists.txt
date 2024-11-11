enable_testing()
add_library(ext_hash ext_hash.cpp)
target_link_libraries(ext_hash io_handler)


add_executable(ext_hash_test ext_hash.test.cpp)
target_link_libraries(ext_hash_test PRIVATE ext_hash GTest::gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(ext_hash_test)