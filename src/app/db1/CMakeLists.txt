add_library(db1 db1.cpp)
target_link_libraries(db1 PUBLIC sorted_list memtable)

add_executable(exp1 exp1.cpp)
target_link_libraries(exp1 db1)

