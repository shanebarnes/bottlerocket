cmake_minimum_required(VERSION 2.8)

project(bottlerocket NONE)

include_directories(${BRLIB_INCLUDE_DIR})

link_libraries(brlib)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# make package
find_program(BUILDDEB dpkg)
find_program(BUILDRPM rpmbuild)

if (BUILDDEB)
    if (CPACK_GENERATOR)
        set (CPACK_GENERATOR "${CPACK_GENERATOR};")
    endif()
    set (CPACK_GENERATOR "${CPACK_GENERATOR}DEB")
endif()

if (BUILDRPM)
    if (CPACK_GENERATOR)
        set (CPACK_GENERATOR "${CPACK_GENERATOR};")
    endif()
    set (CPACK_GENERATOR "${CPACK_GENERATOR}RPM")
endif()

# By default, do not build any packages.
if (NOT CPACK_GENERATOR)
    set (CPACK_GENERATOR "NULL")
endif()

set (CPACK_PACKAGE_NAME "bottlerocket")
set (CPACK_PACKAGE_VERSION "0.2.8")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "A multi-threaded network benchmarking tool")
set (CPACK_PACKAGE_DESCRIPTION "")
set (CPACK_PACKAGE_RELEASE 1)
set (CPACK_PACKAGE_CONTACT "Shane Barnes")
set (CPACK_PACKAGE_VENDOR "Shane Barnes")
set (CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
