cmake_minimum_required(VERSION 3.15.0)
project(RenderEngine LANGUAGES CXX)

# CPM dependency management
include(cmake/CPM.cmake)

# Depdendencies
include(Dependencies.cmake)

add_subdirectory(lib)
add_subdirectory(exe)
