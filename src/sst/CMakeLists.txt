add_library(sst sst.cpp)
add_executable(sst_test sst.test.cpp)
target_link_libraries(sst_test PRIVATE sst Catch2::Catch2WithMain io)


include(Catch)
catch_discover_tests(sst_test)