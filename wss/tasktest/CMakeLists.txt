cmake_minimum_required(VERSION 3.31)
project(tasktest)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/libuv-1.51.0)
add_subdirectory(lib/concurrentqueue-master)

include_directories(lib/libuv-1.51.0/include)
include_directories(lib/concurrentqueue-master)

add_executable(tasktest main.cpp
        FastSwapQueue.h)

target_link_libraries(
        tasktest
        uv_a
)
