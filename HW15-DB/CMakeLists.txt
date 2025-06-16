cmake_minimum_required(VERSION 3.5)

project(database LANGUAGES C)

find_package(SQLite3 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic std=c11")

add_executable(database
    src/main.c
)

target_link_libraries(database PRIVATE ${SQLite3_LIBRARY})
target_link_libraries(database PRIVATE m)

include_directories(${SQLite3_INCLUDE_DIRS})
