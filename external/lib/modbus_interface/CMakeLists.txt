cmake_minimum_required(VERSION 3.16)
project(modbus_interface_test)

include(lib/modbus_interface.cmake)

add_executable(mb_main main.cpp)

target_link_libraries(mb_main modbus_interface_static ${LIBMODBUS})