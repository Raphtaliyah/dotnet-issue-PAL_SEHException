cmake_minimum_required(VERSION 3.6.0)
project(
    Test
    VERSION 0.1.0
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(Native/CoreCLR)

add_executable(Test Native/main.cpp)
target_link_libraries(Test PUBLIC CoreCLR)
