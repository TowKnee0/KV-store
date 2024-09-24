add_library(io io.cpp)
add_executable(io_test io.test.cpp)
target_link_libraries(io_test PRIVATE io GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(io_test)