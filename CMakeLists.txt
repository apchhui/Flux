cmake_minimum_required(VERSION 3.10)

project(DatabaseProject)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")

include_directories(include)


set(SOURCES
    src/Database.cpp
    include/Database.h
    main.cpp
)

add_executable(DatabaseApp ${SOURCES})
