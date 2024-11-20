cmake_minimum_required(VERSION 3.13)

file(GLOB MODBUSPNP_SRC_FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)
file(GLOB MODBUSPNP_HDR_FILES
        "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
)

add_library(modbus_pnp_static STATIC
        ${MODBUSPNP_SRC_FILES}
)

set(LIBMODBUSPNP_INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/include/"
        "${CMAKE_CURRENT_LIST_DIR}/deps"
)

target_include_directories(
        modbus_pnp_static
        PUBLIC
        ${LIBMODBUSPNP_INCLUDE_DIRS}

        "${CMAKE_CURRENT_LIST_DIR}/deps"
)

if(DEFINED LIBS_BUILD_SHARED)
    add_library(modbus_pnp SHARED
            ${MODBUSPNP_SRC_FILES}
    )

    target_include_directories(
            modbus_pnp
            PUBLIC
            ${LIBMODBUSPNP_INCLUDE_DIRS}

            "${CMAKE_CURRENT_LIST_DIR}/deps"
    )
endif ()