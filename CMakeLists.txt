cmake_minimum_required(VERSION 3.6.0)
project(
    Test
    VERSION 0.1.0
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

# Path to the dotnet CLI to invoke for building the managed library.
set(SCRIPT_DOTNET_SDK dotnet)

add_subdirectory(Lib)
add_subdirectory(Native/CoreCLR)

add_library(NativeLib SHARED Native/SharedLib.cpp)

add_executable(NativeExec Native/main.cpp)
target_link_libraries(NativeExec PUBLIC CoreCLR NativeLib)
add_dependencies(NativeExec ManagedLib)
