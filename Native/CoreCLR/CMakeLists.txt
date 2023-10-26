cmake_minimum_required(VERSION 3.5.0)

add_library(CoreCLR INTERFACE)
target_include_directories(CoreCLR INTERFACE include)
target_compile_definitions(CoreCLR INTERFACE NETHOST_USE_AS_STATIC)
target_link_libraries(CoreCLR
                      INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libnethost.a)
