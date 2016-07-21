cmake_minimum_required(VERSION 2.8)

project(gtest_brutil NONE)

include_directories(${BRLIB_INCLUDE_DIR}
                    ${BRLIB_SOURCE_DIR}
                    ${gtest_SOURCE_DIR}/include)

link_libraries(brlib gtest)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
