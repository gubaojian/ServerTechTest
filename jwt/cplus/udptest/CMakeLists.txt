cmake_minimum_required(VERSION 3.31)
project(udptest)

set(CMAKE_CXX_STANDARD 20)

include_directories(/opt/homebrew/Cellar/libuv/1.51.0/include/)

find_library(UVLIB
        NAMES uv       # 库名称
        PATHS /opt/homebrew/Cellar/libuv/1.51.0/lib  # 自定义搜索路径
        NO_DEFAULT_PATH   # 仅搜索指定路径（可选）
)

message("lib ${UVLIB}")

add_executable(udptest main.cpp
        udp_server.cpp
        udp_server.h
        udp_client.cpp
        udp_client.h)
target_link_libraries(
        udptest
        ${UVLIB}
)
