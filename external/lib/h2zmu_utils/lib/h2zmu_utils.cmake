cmake_minimum_required(VERSION 3.13)

file(GLOB_RECURSE UTILS_SRC_FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
        )

file(GLOB_RECURSE UTILS_HDR_FILES
        "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
        )

add_library(utils SHARED
        ${UTILS_SRC_FILES}
        ${UTILS_HDR_FILES}
        )

add_library(utils_static STATIC
        ${UTILS_SRC_FILES}
        ${UTILS_HDR_FILES}
        )

set(LIBUTILS_INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/include/"
        )

target_include_directories(utils PUBLIC ${LIBUTILS_INCLUDE_DIRS})
target_include_directories(utils_static PUBLIC ${LIBUTILS_INCLUDE_DIRS})
