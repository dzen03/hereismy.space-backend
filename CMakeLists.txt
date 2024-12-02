cmake_minimum_required(VERSION 3.27)
project(here_is_my_space)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_TESTING "Enable tests" OFF)
add_subdirectory(simple-http-server)

option(USE_CLANG_TIDY "Enable clang tidy" ON)
set(CLANG_TIDY_NAME "clang-tidy" CACHE STRING "clang-tidy name/verion")
if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY "${CLANG_TIDY_NAME}")
endif()

if(CLANG_TIDY)
    message(STATUS "Clang-tidy: ${CLANG_TIDY}")

    set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS clang-tidy-checks.txt
    )

    file(STRINGS clang-tidy-checks.txt clang_tidy_checks)
    list(JOIN clang_tidy_checks "," clang_tidy_checks_str)
    set(CMAKE_C_CLANG_TIDY
            ${CLANG_TIDY}
            -header-filter=${CMAKE_SOURCE_DIR}
            -checks=${clang_tidy_checks_str}
            -warnings-as-errors=*
    )
    set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY}
            -header-filter=${CMAKE_SOURCE_DIR}
            -checks=${clang_tidy_checks_str}
            -warnings-as-errors=*
    )
endif()

add_executable(here_is_my_space main.cpp)
target_link_libraries(here_is_my_space PRIVATE simple-http-server-sources)
