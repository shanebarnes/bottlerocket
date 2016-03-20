cmake_minimum_required(VERSION 2.8)

set(BR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cli_options.c
    ${CMAKE_CURRENT_SOURCE_DIR}/io_event_poll.c
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mutex_instance.c
    ${CMAKE_CURRENT_SOURCE_DIR}/output_if_std.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rwlock_instance.c
    ${CMAKE_CURRENT_SOURCE_DIR}/socket_instance.c
    ${CMAKE_CURRENT_SOURCE_DIR}/socket_tcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_instance.c
    ${CMAKE_CURRENT_SOURCE_DIR}/util_date.c
    ${CMAKE_CURRENT_SOURCE_DIR}/util_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/util_sysctl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/util_unit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/version.c
    PARENT_SCOPE
)
