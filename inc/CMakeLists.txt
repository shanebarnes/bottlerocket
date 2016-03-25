cmake_minimum_required(VERSION 2.8)

set(BR_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/cli_options.h
    ${CMAKE_CURRENT_SOURCE_DIR}/input_if_instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/input_if_std.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io_event_instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io_event_poll.h
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mutex_instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/output_if_instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/output_if_std.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rwlock_instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sock_obj.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sock_tcp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sock_udp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/system_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util_date.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util_ioctl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util_sysctl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util_unit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h
    PARENT_SCOPE
)
