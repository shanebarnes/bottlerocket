cmake_minimum_required(VERSION 2.8)

project(bottlerocket NONE)

include_directories(${BRLIB_INCLUDE_DIR})

link_libraries(brlib)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
