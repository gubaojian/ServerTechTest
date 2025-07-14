cmake_minimum_required(VERSION 3.31)
project(protocol)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 ")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 ")
    MESSAGE("-----BUILD------ DEBUG OPTION")
endif()

add_subdirectory(lib/Turbo-Base64-master)

set(YYJSON_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/yyjson-master)

include_directories(${PROJECT_SOURCE_DIR}/lib/yyjson-master/src)
include_directories(${PROJECT_SOURCE_DIR}/lib/Turbo-Base64-master)
include_directories(${PROJECT_SOURCE_DIR}/lib/thread-pool-5.0.0/include)

add_executable(protocol main.cpp)

target_link_libraries(
        protocol
        yyjson
        base64
)
