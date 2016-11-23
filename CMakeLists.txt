cmake_minimum_required(VERSION 3.6)
project(ftp_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/mysocket.cpp
        src/mysocket.h)

add_executable(ftp_project ${SOURCE_FILES})