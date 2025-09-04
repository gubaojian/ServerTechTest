cmake_minimum_required(VERSION 3.29)
project(tlsdemo)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED CMAKE_CURRENT_USER_DIR)  # 额外检查变量是否定义（避免极端环境问题）
    set(USER_HOME_DIR "${CMAKE_CURRENT_USER_DIR}")
else()
    # 降级处理：若变量未定义（理论上 3.14+ 不会出现），通过环境变量兼容
    if(WIN32)
        set(USER_HOME_DIR "$ENV{USERPROFILE}")
    else()
        set(USER_HOME_DIR "$ENV{HOME}")
    endif()
endif()

if(EXISTS "${USER_HOME_DIR}")
    message(STATUS "current user home dir: ${USER_HOME_DIR}")
else()
    message(WARNING "cann't found current user home dir: ${USER_HOME_DIR}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CDEPS_DIR "${USER_HOME_DIR}/cdeps/debug")
else ()
    set(CDEPS_DIR "${USER_HOME_DIR}/cdeps/release")
endif()


message("${USER_HOME_DIR}/cdeps_debug/openssl/lib/cmake/OpenSSL")

set(OPENSSL_SOURCE_DIR "/Users/baojian/cdeps_source")

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL 3.5.1 REQUIRED CONFIG
        PATHS "${CDEPS_DIR}/openssl/3.5.1/lib/cmake/OpenSSL"
        NO_DEFAULT_PATH)

if(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION} (${OPENSSL_INCLUDE_DIR})")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL or check the path.")
endif()

add_executable(tlsdemo main.cpp)

target_link_libraries(tlsdemo PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_SOURCE_DIRS
            "${OPENSSL_SOURCE_DIR}/crypto/"
            "${OPENSSL_SOURCE_DIR}/crypto/bio"
            "${OPENSSL_SOURCE_DIR}/ssl/"
    )
    target_include_directories(tlsdemo
            PRIVATE
            $<$<CONFIG:Debug>:${DEBUG_SOURCE_DIRS}>  # 仅 Debug 模式生效
    )
endif()

