cmake_minimum_required(VERSION 3.16)
project(uart_comm)

set(CMAKE_CXX_STANDARD 14)

include(lib/uart_communication.cmake)

add_executable(uart main.cpp)

target_link_libraries(uart uart_communication)
