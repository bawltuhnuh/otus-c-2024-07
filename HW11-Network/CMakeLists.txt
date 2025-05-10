cmake_minimum_required(VERSION 3.5)

project(font_convertor LANGUAGES C)

add_executable(font_convertor src/main.c
    src/convertor.h
    src/convertor.c)
