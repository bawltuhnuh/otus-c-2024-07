cmake_minimum_required(VERSION 3.5)

project(ipc LANGUAGES C)

add_executable(ipc src/main.c
    src/server.h
    src/lock.h
    src/lock.c)
