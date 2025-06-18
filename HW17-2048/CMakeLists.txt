cmake_minimum_required(VERSION 3.5)

project(game LANGUAGES C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic std=c11")

add_executable(game src/main.c)
