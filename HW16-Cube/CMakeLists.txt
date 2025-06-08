cmake_minimum_required(VERSION 3.5)

project(cube LANGUAGES C)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic std=c11")

add_executable(cube src/main.c)

target_link_libraries(cube ${OPENGL_LIBRARIES} ${GLUT_glut_LIBRARY})
