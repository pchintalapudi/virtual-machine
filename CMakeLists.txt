cmake_minimum_required(VERSION 3.17.2)
project(oops VERSION 0.1.0)
include(CTest)
enable_testing()

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED true)

add_executable(oops oops.cpp)

add_subdirectory(classes)
add_subdirectory(classloader)
add_subdirectory(core)
add_subdirectory(gc)
add_subdirectory(globals)
add_subdirectory(logues)
add_subdirectory(instructions)
add_subdirectory(memory)
add_subdirectory(methods)
add_subdirectory(native)
add_subdirectory(platform)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -Wextra -Wall -Winit-self -Wold-style-cast -Woverloaded-virtual -Wuninitialized -Winit-self -Wno-unknown-pragmas -fsanitize=undefined -fsanitize-undefined-trap-on-error")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -Wextra -Wall -Winit-self -Wold-style-cast -Woverloaded-virtual -Wuninitialized -Winit-self -Wno-unknown-pragmas")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -Wextra -Wall -Winit-self -Wold-style-cast -Woverloaded-virtual -Wuninitialized -Winit-self -Wno-unknown-pragmas -g -S -fverbose-asm -march=native")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
