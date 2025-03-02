cmake_minimum_required(VERSION 3.5)

project(dir_view LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${GTK_INCLUDE_DIRS})

add_executable(dir_view src/main.c
    src/dir_view.h
    src/dir_view.c)

target_include_directories(dir_view PRIVATE ${GTK_INCLUDE_DIRS})
target_link_libraries(dir_view ${GTK_LIBRARIES})
