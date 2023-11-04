cmake_minimum_required(VERSION 3.6.0)

add_custom_target(
    ManagedLib ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${SCRIPT_DOTNET_SDK} build --configuration Release)
