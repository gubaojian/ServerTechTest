cmake_minimum_required(VERSION 3.31)
project(protocol)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/Turbo-Base64-master)

set(YYJSON_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/yyjson-master)

include_directories(${PROJECT_SOURCE_DIR}/lib/yyjson-master/src)
include_directories(${PROJECT_SOURCE_DIR}/lib/Turbo-Base64-master)

add_executable(protocol main.cpp)

target_link_libraries(
        protocol
        yyjson
        base64
)
