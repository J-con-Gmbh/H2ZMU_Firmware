cmake_minimum_required(VERSION 3.13)
project(h2zmu-axcf C CXX)

## modified
set(H2ZMU_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/project")
include(project/cmake/include_h2zmu.cmake)

list(APPEND CMAKE_PROGRAM_PATH
        "${CMAKE_CURRENT_LIST_DIR}/external/build/cmake/external/grpc"
        )

include_directories(src)
##

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
file(GLOB_RECURSE sources src/*.cpp src/*.hpp

        ## modified
        project/src/*
        ##
)

## modified
list(FILTER sources EXCLUDE REGEX "h2zmu.cpp")
##

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PlcnextGrpc REQUIRED)
add_executable(${PROJECT_NAME} ${sources} ${H2ZMU_SRC_FILES} ${libs_src_files}) #${H2ZMU_HDR_FILES})
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++ PlcnextGrpc::PlcnextGrpc

        ## modified
        ${CUSTOM_LIBS_TO_LINK}
        ${CMAKE_CURRENT_LIST_DIR}/external/build/cmake/external/libmodbus/lib/libmodbus.so.5.1.0
        ##
        )

install(TARGETS ${PROJECT_NAME})