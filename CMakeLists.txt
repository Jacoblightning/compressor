cmake_minimum_required(VERSION 3.28)
project(compressor C)

set(CMAKE_C_STANDARD 23)

add_executable(compressor main.c
        helpers.c
        helpers.h)

target_link_libraries(compressor m)