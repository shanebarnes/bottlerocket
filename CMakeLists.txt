cmake_minimum_required(VERSION 2.8)

set(BR_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(BR_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(inc)
add_subdirectory(src)

add_subdirectory(app)
