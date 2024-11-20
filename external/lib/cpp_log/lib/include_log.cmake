cmake_minimum_required(VERSION 3.13)

file(GLOB_RECURSE LIBLOG_SRC_FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/Log.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/libutils/src/epoch_time.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ibutils/src/sys/dump_stacktrace.cpp"
        )

file(GLOB_RECURSE LIBLOG_HDR_FILES
        "${CMAKE_CURRENT_LIST_DIR}/include/Log.h"
        "${CMAKE_CURRENT_LIST_DIR}/libutils/include/epoch_time.h"
        "${CMAKE_CURRENT_LIST_DIR}/libutils/include/sys/dump_stacktrace.h"
        )

add_library(log SHARED
        ${LIBLOG_SRC_FILES}
        ${LIBLOG_HDR_FILES}
        )

add_library(log_static STATIC
        ${LIBLOG_SRC_FILES}
        ${LIBLOG_HDR_FILES}
        )

set(LIBLOG_INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/include/"
        )

target_include_directories(
        log
        PUBLIC
        ${LIBLOG_INCLUDE_DIRS}
)

target_include_directories(
        log_static
        PUBLIC
        ${LIBLOG_INCLUDE_DIRS}
)
