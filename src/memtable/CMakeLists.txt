add_library(memtable bst.cpp)
add_executable(memtable_test bst.test.cpp)
target_link_libraries(memtable_test PRIVATE memtable Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(memtable_test)