
if(NOT DEFINED H2ZMU_BASE_DIR)
    message(FATAL_ERROR "Please define H2ZMU_BASE_DIR variable")
endif()


include(${H2ZMU_BASE_DIR}/cmake/set_cmake_variables.cmake)

include(${H2ZMU_BASE_DIR}/lib/sqlite/sqlite.cmake)
include(${H2ZMU_BASE_DIR}/lib/cpp_log/lib/include_log.cmake)
include(${H2ZMU_BASE_DIR}/lib/h2zmu_utils/lib/h2zmu_utils.cmake)
include(${H2ZMU_BASE_DIR}/lib/modbus_interface/lib/modbus_interface.cmake)
include(${H2ZMU_BASE_DIR}/lib/uart_comm/lib/uart_communication.cmake)

include_directories(${H2ZMU_BASE_DIR}/lib/sqlite/include)
include_directories(${H2ZMU_BASE_DIR}/lib/cpp_log/lib/include)
include_directories(${H2ZMU_BASE_DIR}/lib/h2zmu_utils/lib/include)
include_directories(${H2ZMU_BASE_DIR}/lib/modbus_interface/lib/include)
include_directories(${H2ZMU_BASE_DIR}/lib/uart_comm/lib/include)

include_directories(${H2ZMU_BASE_DIR}/include)


set(CUSTOM_LIBS_TO_LINK
        log_static
        modbus_interface_static
        utils_static
        uart_communication_static
        sqlite_static
)


include(${H2ZMU_BASE_DIR}/lib/sqlite/sqlite.cmake)
