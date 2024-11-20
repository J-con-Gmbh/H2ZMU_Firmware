cmake_minimum_required(VERSION 3.13)

file(GLOB_RECURSE SRC_FILES
        ${SRC_FILES}
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
        )
file(GLOB_RECURSE HDR_FILES
        ${HDR_FILES}
        "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
        )

add_library(uart_communication SHARED
        ${SRC_FILES}
        ${HDR_FILES}
        )

add_library(uart_communication_static STATIC
        ${SRC_FILES}
        ${HDR_FILES}
        )

set(LIBUART_INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/include/"
        )

target_include_directories(uart_communication PUBLIC ${LIBUART_INCLUDE_DIRS})
target_include_directories(uart_communication_static PUBLIC ${LIBUART_INCLUDE_DIRS})
