
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


## for Profiler
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif ()

#set(CMAKE_EXE_LINKER_FLAGS
#        ${CMAKE_EXE_LINKER_FLAGS}
#        "-ldl"
#    )

file(GLOB_RECURSE H2ZMU_SRC_FILES
        ${H2ZMU_BASE_DIR}/src/Core.cpp
        ${H2ZMU_BASE_DIR}/src/*/*.cpp
        )

file(GLOB_RECURSE H2ZMU_HDR_FILES
        ${H2ZMU_BASE_DIR}/include/*.h
        )
