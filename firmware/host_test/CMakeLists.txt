cmake_minimum_required(VERSION 3.16)
project(smart_intercom_host_tests C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(test_placeholder test_placeholder.c)
add_test(NAME placeholder COMMAND test_placeholder)
