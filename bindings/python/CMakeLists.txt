cmake_minimum_required(VERSION 3.18)
project(krnl_python LANGUAGES CXX)

set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpmcache")
file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake
    ${CMAKE_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a
)
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage(
    NAME pybind11
    URL https://github.com/pybind/pybind11/archive/refs/tags/v3.0.1.tar.gz
)

pybind11_add_module(krnl_py MODULE module.cpp)

# Include krnl headers
target_include_directories(krnl_py PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../krnl/include
)

# print CMAKE_SOURCE_DIR
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

# Link against the krnl target already defined in root
target_link_libraries(krnl_py PRIVATE krnl)
