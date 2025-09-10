cmake_minimum_required(VERSION 4.0)
project(httparser)

set(CMAKE_CXX_STANDARD 20)

add_executable(httparser
        main.cpp
        api.c
        http.c
        llhttp.h
        llhttp.c
)
