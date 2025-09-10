cmake_minimum_required(VERSION 4.0)
project(uriparse C)

set(CMAKE_C_STANDARD 11)


include_directories(/usr/local/apr/include/apr-1)

add_executable(uriparse
        main.c
        urlparse.c
        urlparse.h
        url_encode.c
        url_encode.h
)

target_link_libraries(
        uriparse
        /usr/local/apr/lib/libapr-1.a
)
