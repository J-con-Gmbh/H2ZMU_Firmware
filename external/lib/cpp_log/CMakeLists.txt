cmake_minimum_required(VERSION 3.13)
project(log)

set(CMAKE_CXX_STANDARD 14)

include(lib/include_log.cmake)

add_executable(log_main main.cpp)

include_directories(
#        "${LIBLOG_INCLUDE_DIRS}"
)

target_link_libraries(log_main log)