cmake_minimum_required(VERSION 3.5)
project(ftp_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")

set(SERVEUR_SOURCE_FILES
        src/mysocket.cpp
        src/mysocket.h
        src/serveur.h
        src/serveur.cpp)

set(CLIENT_SOURCE_FILES
        src/mysocket.cpp
        src/mysocket.h
        src/Client.cpp
        src/Client.h
        src/Config.h)

add_executable(server ${SERVEUR_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})
add_executable(srv-test src/main.cpp)