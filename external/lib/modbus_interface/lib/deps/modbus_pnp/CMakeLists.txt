cmake_minimum_required(VERSION 3.5)
project(test)

set(CMAKE_CXX_STANDARD 17)

include(lib/modbus_pnp.cmake)

add_executable(test
        main.cpp
)

target_link_libraries(
        test
        modbus_pnp_static
)