cmake_minimum_required(VERSION 3.5)

project(file_server LANGUAGES C)

add_executable(file_server src/main.c
    src/server.h
    src/server.c)
