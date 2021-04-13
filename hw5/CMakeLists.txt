cmake_minimum_required(VERSION 3.17)
project(slab)

set(CMAKE_CXX_STANDARD 17)

add_executable(slab main.cpp mmap_page_allocator.hpp mmap_page_allocator.inl page_allocator.hpp slab_allocator.hpp slab_allocator.cpp)