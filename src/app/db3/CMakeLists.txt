enable_testing()
add_library(db3 db3.cpp)
target_link_libraries(db3 PUBLIC btree bst ext_hash io_handler lsm)

add_executable(db3_test db3.test.cpp)
target_link_libraries(db3_test PRIVATE db3 GTest::gtest_main gmock io_handler ext_hash)

add_executable(exp3 exp3.cpp)
target_link_libraries(exp3 db3)

