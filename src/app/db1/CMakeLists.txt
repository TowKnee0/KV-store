enable_testing()
add_library(db1 db1.cpp)
target_link_libraries(db1 PUBLIC sorted_list bst)

add_executable(db1_test db1.test.cpp)
target_link_libraries(db1_test PRIVATE db1 GTest::gtest_main gmock io_handler)


add_executable(exp1 exp1.cpp)
target_link_libraries(exp1 db1)

