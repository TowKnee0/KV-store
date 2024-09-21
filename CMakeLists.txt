cmake_minimum_required(VERSION 3.5.0)
project(KV-store LANGUAGES CXX)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

add_executable(bst_tests src/memtable/bst.test.cpp src/memtable/bst.cpp)
target_link_libraries(bst_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(bst_tests)

add_executable(KV-store main.cpp)
