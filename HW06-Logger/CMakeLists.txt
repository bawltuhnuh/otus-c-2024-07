cmake_minimum_required(VERSION 3.5)

project(logger LANGUAGES C)

add_library(logger STATIC src/logger.h
    src/logger.c)

add_executable(logger_demo src/main.c)

target_link_libraries(logger_demo PRIVATE logger)
