cmake_minimum_required(VERSION 2.8)

project(gtest_br NONE)

include_directories(${BRLIB_INCLUDE_DIR}
                    ${BRLIB_SOURCE_DIR}
                    ${gtest_SOURCE_DIR}/include)

link_libraries(brlib gtest)

add_executable(gtest_brcon gtest_brcon.cpp)
add_executable(gtest_brutil gtest_brutil.cpp)

install(TARGETS gtest_brcon DESTINATION bin)
install(TARGETS gtest_brutil DESTINATION bin)
