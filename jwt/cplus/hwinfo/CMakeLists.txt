cmake_minimum_required(VERSION 3.31)
project(hwinfo_test)

set(CMAKE_CXX_STANDARD 20)

if(APPLE)
    add_definitions(-DDARWIN)
elseif(WIN32)

elseif(UNIX)

endif()

include_directories(${CMAKE_SOURCE_DIR}/hwinfo/include)
include_directories(${CMAKE_SOURCE_DIR}/machineid/include)

add_subdirectory(hwinfo)
add_subdirectory(machineid)



add_executable(hwinfo_test main.cpp)

target_link_libraries(hwinfo_test
        PUBLIC
        lfreist-hwinfo::hwinfo
        machineid
)

## https://github.com/lfreist/hwinfo/blob/main/examples/CMakeLists.txt


