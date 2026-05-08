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

add_library(config STATIC
    ${COMPONENTS_DIR}/config/host/config_host.c
)
target_include_directories(config PUBLIC
    ${COMPONENTS_DIR}/config/include
)

add_executable(test_config test_config.c)
target_link_libraries(test_config PRIVATE config unity)
add_test(NAME config COMMAND test_config)

add_library(tone_detect STATIC
    ${COMPONENTS_DIR}/tone_detect/tone_detect.c
)
target_include_directories(tone_detect PUBLIC
    ${COMPONENTS_DIR}/tone_detect/include
)
target_link_libraries(tone_detect PUBLIC m)

add_executable(test_tone_detect test_tone_detect.c)
target_link_libraries(test_tone_detect PRIVATE tone_detect unity m)
add_test(NAME tone_detect COMMAND test_tone_detect)

add_library(dispatcher STATIC
    ${COMPONENTS_DIR}/dispatcher/dispatcher.c
)
target_include_directories(dispatcher PUBLIC
    ${COMPONENTS_DIR}/dispatcher/include
)
target_link_libraries(dispatcher PUBLIC state_machine event_queue tone_detect)

add_executable(test_dispatcher test_dispatcher.c)
target_link_libraries(test_dispatcher PRIVATE dispatcher unity m)
add_test(NAME dispatcher COMMAND test_dispatcher)

add_library(rtsp_proto STATIC
    ${COMPONENTS_DIR}/rtsp_proto/rtsp_url.c
    ${COMPONENTS_DIR}/rtsp_proto/rtsp_request.c
)
target_include_directories(rtsp_proto PUBLIC
    ${COMPONENTS_DIR}/rtsp_proto/include
)

add_executable(test_rtsp_url test_rtsp_url.c)
target_link_libraries(test_rtsp_url PRIVATE rtsp_proto unity)
add_test(NAME rtsp_url COMMAND test_rtsp_url)

add_executable(test_rtsp_request test_rtsp_request.c)
target_link_libraries(test_rtsp_request PRIVATE rtsp_proto unity)
add_test(NAME rtsp_request COMMAND test_rtsp_request)
