cmake_minimum_required(VERSION 3.10)
project(Laba-1)

set(CMAKE_CXX_STANDARD 17)

add_executable(my_sort main.cpp src.cpp)
add_executable(std_sort std_sort.cpp src.cpp)
add_executable(make_test make_test.cpp)