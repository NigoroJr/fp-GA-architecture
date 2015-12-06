cmake_minimum_required(VERSION 2.8)
project(fp-GA-architecture)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(src)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    enable_testing()
    add_subdirectory(test)
endif()
