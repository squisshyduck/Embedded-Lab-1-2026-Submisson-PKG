cmake_minimum_required(VERSION 3.16)
project(untitled C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(untitled STATIC library.c)
add_library(embedded_core STATIC src/common.c)
target_include_directories(embedded_core PUBLIC include)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(embedded_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(test_common tests/test_common.c)
    target_link_libraries(test_common PRIVATE embedded_core)

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(test_common PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    add_test(NAME common_tests COMMAND test_common)
    add_test(NAME deliberate_failure_demo COMMAND test_common --demo-failure)
    set_tests_properties(deliberate_failure_demo PROPERTIES WILL_FAIL TRUE)
endif()
