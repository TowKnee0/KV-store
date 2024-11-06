add_library(io_handler io_handler.cpp)
add_executable(io_handler_test io_handler.test.cpp)
target_link_libraries(io_handler_test PRIVATE io_handler GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(io_handler_test)