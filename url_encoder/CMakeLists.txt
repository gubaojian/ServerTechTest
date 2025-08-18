cmake_minimum_required(VERSION 4.0)
project(url_encoder C)

set(CMAKE_C_STANDARD 99)

add_executable(url_encoder
        main.c
        url_encode.c
        url_encode.h)
