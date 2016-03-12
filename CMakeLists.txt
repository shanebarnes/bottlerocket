set (CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 2.8)

project(bottlerocket)

if (${UNIX})
    add_definitions("-Wall")
    add_definitions("-Werror")
    add_definitions("-Wextra")
    add_definitions("-O3")
    if (CMAKE_COMPILER_IS_GNUCC)
        add_definitions("-D_FILE_OFFSET_BITS=64")
        add_definitions("-D_GNU_SOURCE")
    endif (CMAKE_COMPILER_IS_GNUCC)
endif(${UNIX})

include_directories(${bottlerocket_SOURCE_DIR}/inc)

link_directories(${bottlerocket_SOURCE_DIR}/src)
link_libraries(pthread)

set(bottlerocket_HEADERS
${bottlerocket_SOURCE_DIR}/inc/cli_options.h
${bottlerocket_SOURCE_DIR}/inc/logger.h
${bottlerocket_SOURCE_DIR}/inc/manip_date.h
${bottlerocket_SOURCE_DIR}/inc/manip_string.h
${bottlerocket_SOURCE_DIR}/inc/manip_unit.h
${bottlerocket_SOURCE_DIR}/inc/mutex_instance.h
${bottlerocket_SOURCE_DIR}/inc/rwlock_instance.h
${bottlerocket_SOURCE_DIR}/inc/socket_instance.h
${bottlerocket_SOURCE_DIR}/inc/socket_api.h
${bottlerocket_SOURCE_DIR}/inc/socket_tcp.h
${bottlerocket_SOURCE_DIR}/inc/system_types.h
${bottlerocket_SOURCE_DIR}/inc/thread_instance.h
${bottlerocket_SOURCE_DIR}/inc/util_sysctl.h
${bottlerocket_SOURCE_DIR}/inc/version.h
)

set(bottlerocket_SOURCES
${bottlerocket_SOURCE_DIR}/src/bottlerocket.c
${bottlerocket_SOURCE_DIR}/src/cli_options.c
${bottlerocket_SOURCE_DIR}/src/logger.c
${bottlerocket_SOURCE_DIR}/src/manip_date.c
${bottlerocket_SOURCE_DIR}/src/manip_string.c
${bottlerocket_SOURCE_DIR}/src/manip_unit.c
${bottlerocket_SOURCE_DIR}/src/mutex_instance.c
${bottlerocket_SOURCE_DIR}/src/rwlock_instance.c
${bottlerocket_SOURCE_DIR}/src/socket_instance.c
${bottlerocket_SOURCE_DIR}/src/socket_tcp.c
${bottlerocket_SOURCE_DIR}/src/thread_instance.c
${bottlerocket_SOURCE_DIR}/src/util_sysctl.c
${bottlerocket_SOURCE_DIR}/src/version.c
)

add_executable(bottlerocket ${bottlerocket_HEADERS} ${bottlerocket_SOURCES})

install(TARGETS bottlerocket DESTINATION bin)
