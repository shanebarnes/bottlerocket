cmake_minimum_required(VERSION 2.8)
project(gtest_brutil NONE)

if (${UNIX})
    add_definitions("-Wall")
    add_definitions("-Werror")
    add_definitions("-Wextra")
    add_definitions("-O3")
    add_definitions("-D_GNU_SOURCE")
    if (CMAKE_COMPILER_IS_GNUCC)
        add_definitions("-D_FILE_OFFSET_BITS=64")
    endif (CMAKE_COMPILER_IS_GNUCC)
endif(${UNIX})

include_directories(${BR_LIB_INCLUDE_DIR}
                    ${BR_LIB_SOURCE_DIR}
                    ${gtest_SOURCE_DIR}/include)

link_directories(${BR_LIB_INCLUDE_DIR})
link_libraries(gtest)

add_executable(${PROJECT_NAME}
               ${PROJECT_NAME}.cpp
               ${BR_HEADERS}
               ${BR_SOURCES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
