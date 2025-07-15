cmake_minimum_required(VERSION 3.31)
project(tasktest)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/libuv-1.51.0)

include_directories(lib/libuv-1.51.0/include)

add_executable(tasktest main.cpp)

target_link_libraries(
        tasktest
        uv_a
)
