cmake_minimum_required(VERSION 3.5)

project(zipjpeg LANGUAGES C)

add_executable(zipjpeg src/main.c
    src/zip.h
    src/zip.c
    src/detail.h
    src/detail.c)
