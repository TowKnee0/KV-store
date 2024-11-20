enable_testing()
add_library(db2 db2.cpp)
target_link_libraries(db2 PUBLIC btree bst ext_hash io_handler)

add_executable(db2_test db2.test.cpp)
target_link_libraries(db2_test PRIVATE db2 GTest::gtest_main gmock io_handler ext_hash)

add_executable(exp2 exp2.cpp)
target_link_libraries(exp2 db2)

