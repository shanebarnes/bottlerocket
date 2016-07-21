cmake_minimum_required(VERSION 2.8)

project(brlib NONE)

include_directories(${BRLIB_INCLUDE_DIR})

link_directories(${BRLIB_INCLUDE_DIR})
link_libraries(pthread)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    link_libraries(rt)
endif ()

add_library(${PROJECT_NAME} STATIC ${BRLIB_HEADERS} ${BRLIB_SOURCES})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
