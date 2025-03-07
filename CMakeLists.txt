cmake_minimum_required(VERSION 3.16)

project(
    benchamel
    LANGUAGES C CXX
    VERSION 0.1)

message(INFO "Building on ${CMAKE_SYSTEM_NAME}")

add_subdirectory(ConVul)
add_subdirectory(libvsync)
add_subdirectory(sctbench)
