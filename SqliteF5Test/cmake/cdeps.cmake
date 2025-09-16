
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

set(CDEPS_SOURCE_DIR "${USER_HOME_DIR}/cdeps/src")

set(DEBUG_SOURCE_DIRS "")
set(CDEPS_LIBS_DIRS "")
set(CDEPS_INCLUDE_DIRS "")
