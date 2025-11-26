cmake_minimum_required(VERSION 3.22)
project(krnl_root)

set(CMAKE_CXX_STANDARD 20)

# Export compile commands for editors (e.g. clangd / VS Code C/C++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpmcache")
file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake
    ${CMAKE_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a
)
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "" FORCE)
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME dawn
    URL https://github.com/google/dawn/archive/refs/tags/v20251030.221451.tar.gz
)

add_subdirectory(krnl)
add_subdirectory(sandbox)
add_subdirectory(samples)
