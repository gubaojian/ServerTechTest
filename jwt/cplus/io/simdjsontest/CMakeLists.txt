cmake_minimum_required(VERSION 3.31)
project(simdjsontest)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(simdjson-master)


include_directories(simdjson-master/singleheader/)

add_executable(simdjsontest main.cpp)

target_link_libraries(
    simdjsontest
    simdjson
)
