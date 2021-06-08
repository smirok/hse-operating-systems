cmake_minimum_required(VERSION 3.15)
project(lab-06)

include_directories(include)
set(CMAKE_CXX_STANDARD 17)

add_executable(lab-06
        main.cpp
        src/Archiver.cpp include/Archiver.hpp
        src/FileReader.cpp include/FileReader.hpp
        src/FileWriter.cpp include/FileWriter.hpp
        src/Creator.cpp include/Creator.hpp
        src/ArchiverException.cpp include/ArchiverException.hpp)