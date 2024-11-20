cmake_minimum_required(VERSION 3.16)
project(utils_main)

set(CMAKE_CXX_STANDARD 14)

include( lib/h2zmu_utils.cmake )

add_executable(utils_main main.cpp)

target_link_libraries(utils_main utils_static)
