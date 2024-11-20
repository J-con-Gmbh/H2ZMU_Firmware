
include(FetchContent)

FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(my_tests ${GTEST_BASE_DIR}/testsuite.cpp ${SRC_FILES} ${HDR_FILES})

target_link_libraries(my_tests
        ${CUSTOM_LIBS_TO_LINK}
        ${ALL_LIBS_TO_LINK}
        ${CMAKE_DL_LIBS}
        GTest::gtest_main
        )   # Wichtig! Sqlite muss vor der dl lib angegeben werden: ${SQLITE} ${CMAKE_DL_LIBS}

include(GoogleTest)

gtest_discover_tests(my_tests)
