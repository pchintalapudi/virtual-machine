cmake_minimum_required(VERSION 3.17.2)
project(oops VERSION 0.1.0)
include(CTest)
enable_testing()

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED true)

add_executable(oops oops.cpp)

add_subdirectory(memory)
add_subdirectory(parser)
add_subdirectory(punning)
add_subdirectory(vm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unknown-pragmas")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
