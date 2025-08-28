cmake_minimum_required(VERSION 3.31)
project(tasktest)

set(CMAKE_CXX_STANDARD 20)

# 设置 C 语言的 Release 模式编译选项
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

# 设置 C++ 语言的 Release 模式编译选项（如果项目包含 C++ 代码）
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

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
