cmake_minimum_required(VERSION 3.10)
project(mpied)

set(CMAKE_CXX_STANDARD 11)
include_directories(/usr/include/openmpi/)

add_executable(mpied main.cpp)
target_link_libraries(mpied jsoncpp)