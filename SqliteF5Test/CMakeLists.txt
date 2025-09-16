cmake_minimum_required(VERSION 4.0)
project(SqliteF5Test)

set(CMAKE_CXX_STANDARD 20)

include(cmake/cdeps.cmake)
include(cmake/sqlite.cmake)

include_directories(${CDEPS_INCLUDE_DIRS})


add_executable(SqliteF5Test main.cpp)


target_link_libraries(SqliteF5Test PRIVATE
        ${CDEPS_LIBS_DIRS}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_include_directories(SqliteF5Test
            PRIVATE
            $<$<CONFIG:Debug>:${CDEPS_SOURCE_DIRS}>
    )
endif()
