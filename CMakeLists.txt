cmake_minimum_required(VERSION 3.23)
project(
    benchamel
    LANGUAGES C CXX)


set(CMAKE_EXECUTABLE_SUFFIX ".x")

add_subdirectory(ConVul)
add_subdirectory(libvsync)
