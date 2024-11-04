cmake_minimum_required(VERSION 3.5)

project(weather LANGUAGES C)

add_executable(weather src/main.c
    src/json_parser.h
    src/weather.h
    src/weather.c
    src/json_parser.c)

target_link_libraries(weather -lcurl -ljson-c)
