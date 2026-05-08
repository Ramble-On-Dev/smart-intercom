cmake_minimum_required(VERSION 3.16)
project(smart_intercom_host_tests C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(unity)

set(COMPONENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../components)

find_package(Threads REQUIRED)

add_library(state_machine STATIC
    ${COMPONENTS_DIR}/state_machine/state_machine.c
)
target_include_directories(state_machine PUBLIC
    ${COMPONENTS_DIR}/state_machine/include
)

add_library(event_queue STATIC
    ${COMPONENTS_DIR}/event_queue/host/event_queue_host.c
)
target_include_directories(event_queue PUBLIC
    ${COMPONENTS_DIR}/event_queue/include
)
target_link_libraries(event_queue PUBLIC state_machine Threads::Threads)

add_executable(test_state_machine test_state_machine.c)
target_link_libraries(test_state_machine PRIVATE state_machine unity)
add_test(NAME state_machine COMMAND test_state_machine)

add_executable(test_event_queue test_event_queue.c)
target_link_libraries(test_event_queue PRIVATE event_queue unity)
add_test(NAME event_queue COMMAND test_event_queue)
