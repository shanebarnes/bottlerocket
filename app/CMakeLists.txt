set (CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 2.8)

project(bottlerocket NONE)

if (${UNIX})
    add_definitions("-Wall")
    add_definitions("-Werror")
    add_definitions("-Wextra")
    add_definitions("-O3")
    if (CMAKE_COMPILER_IS_GNUCC)
        add_definitions("-D_FILE_OFFSET_BITS=64")
        add_definitions("-D_GNU_SOURCE")
    endif (CMAKE_COMPILER_IS_GNUCC)
endif ()

include_directories(${BR_LIB_INCLUDE_DIR})

link_directories(${BR_LIB_INCLUDE_DIR})
link_libraries(pthread)

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    link_libraries(rt)
endif ()

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c ${BR_HEADERS} ${BR_SOURCES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
