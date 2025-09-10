cmake_minimum_required(VERSION 4.0)
project(MongooseTest)

set(CMAKE_CXX_STANDARD 20)

add_executable(MongooseTest
        main.cpp
        mongoose.h
        mongoose.c
)
