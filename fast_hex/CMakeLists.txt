cmake_minimum_required(VERSION 4.0)
project(fast_hex C)

set(CMAKE_C_STANDARD 11)

add_executable(
        fast_hex
        main.c
        fast_hex.h
        fast_hex.c
)
