cmake_minimum_required(VERSION 3.16)

project(
    benchamel
    LANGUAGES C CXX
    VERSION 0.1)

message("Building on ${CMAKE_SYSTEM_NAME}")

option(BENCHAMEL_BUILD_TSAN "Enable ThreadSanitizer" OFF)

add_subdirectory(ConVul)
add_subdirectory(libvsync)
add_subdirectory(sctbench)
