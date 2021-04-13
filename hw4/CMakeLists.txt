cmake_minimum_required(VERSION 3.16)
project(buddy)

set(CMAKE_CXX_STANDARD 17)

add_executable(buddy buddy_allocator.cpp buddy_allocator.hpp test.cpp)
