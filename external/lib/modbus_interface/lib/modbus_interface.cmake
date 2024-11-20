cmake_minimum_required(VERSION 3.13)

include(
        ${CMAKE_CURRENT_LIST_DIR}/deps/modbus_pnp/lib/modbus_pnp.cmake
)

file(GLOB MODBUSINTERFACE_SRC_FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
        )
file(GLOB MODBUSINTERFACE_HDR_FILES
        "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
        )

add_library(modbus_interface_static STATIC
        ${MODBUSINTERFACE_SRC_FILES}
        )

set(LIBMODBUSINTERFACE_INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/include/"
        "${CMAKE_CURRENT_LIST_DIR}/deps"
        )

target_link_libraries(
        modbus_interface_static
        modbus_pnp_static
)

target_include_directories(
        modbus_interface_static
        PUBLIC
        ${LIBMODBUSINTERFACE_INCLUDE_DIRS}

        "${CMAKE_CURRENT_LIST_DIR}/deps"
)



if(DEFINED LIBS_BUILD_SHARED)

        target_link_libraries(
                modbus_interface
                "${CMAKE_CURRENT_LIST_DIR}/deps/libmodbus_pnp.so"
        )


        add_library(modbus_interface SHARED
                ${MODBUSINTERFACE_SRC_FILES}
        )

        target_include_directories(
                modbus_interface
                PUBLIC
                ${LIBMODBUSINTERFACE_INCLUDE_DIRS}

                "${CMAKE_CURRENT_LIST_DIR}/deps"
        )

endif ()