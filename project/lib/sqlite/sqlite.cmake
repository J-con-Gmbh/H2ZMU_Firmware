
if(NOT DEFINED SQLITE_LIB_INCLUDED)

    file(GLOB_RECURSE SQLITE_SRC_FILES
            "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
            )

    file(GLOB_RECURSE SQLITE_HDR_FILES
            "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
            )

    add_library(sqlite SHARED
            ${SQLITE_SRC_FILES}
            ${SQLITE_HDR_FILES}
            )

    add_library(sqlite_static STATIC
            ${SQLITE_SRC_FILES}
            ${SQLITE_HDR_FILES}
            )

    set(LIBSQLITE_INCLUDE_DIRS
            "${CMAKE_CURRENT_LIST_DIR}/include"
            )

    target_include_directories(sqlite PUBLIC ${LIBSQLITE_INCLUDE_DIRS})
    target_include_directories(sqlite_static PUBLIC ${LIBSQLITE_INCLUDE_DIRS})

    SET(SQLITE_LIB_INCLUDED
            TRUE)

endif()