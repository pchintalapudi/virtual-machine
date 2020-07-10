cmake_minimum_required(VERSION 3.17.2)
project(oops VERSION 0.1.0)
include(CTest)
enable_testing()

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED true)

add_executable(oops oops.cpp)

add_subdirectory(objects)
add_subdirectory(utils)
add_subdirectory(memory)
add_subdirectory(virtual_machine)
add_subdirectory(bytecode)
add_subdirectory(platform_specific)
add_subdirectory(interface)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wno-unknown-pragmas -fsanitize=undefined -fsanitize-undefined-trap-on-error")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
