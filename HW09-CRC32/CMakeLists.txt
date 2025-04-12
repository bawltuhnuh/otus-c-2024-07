cmake_minimum_required(VERSION 3.5)

project(crc LANGUAGES C)

add_executable(crc src/main.c
    src/crc32.h)
