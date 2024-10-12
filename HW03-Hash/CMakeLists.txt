cmake_minimum_required(VERSION 3.5)

project(hash LANGUAGES C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic std=c11")

add_executable(hash src/main.c
    src/hash.h
    src/hash.c)
