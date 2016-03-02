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

set(bottlerocket_HEADERS
${bottlerocket_SOURCE_DIR}/inc/socket_api.h
${bottlerocket_SOURCE_DIR}/inc/system_types.h
)

set(bottlerocket_SOURCES
${bottlerocket_SOURCE_DIR}/src/bottlerocket.c
)

add_executable(bottlerocket ${bottlerocket_HEADERS} ${bottlerocket_SOURCES})

install(TARGETS bottlerocket DESTINATION bin)
