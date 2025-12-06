cmake_minimum_required(VERSION 3.22)

project(krnl LANGUAGES CXX)

# Options ----------------------------------------------------------------------
option(KRNL_BUILD_SHARED "Build krnl as a shared library" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Library type -----------------------------------------------------------------
if (KRNL_BUILD_SHARED)
    add_library(krnl SHARED)
else()
    add_library(krnl STATIC)
endif()

# Sources ----------------------------------------------------------------------
file(GLOB_RECURSE KRNL_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/krnl/*.h"
)

file(GLOB_RECURSE KRNL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

target_sources(krnl PRIVATE ${KRNL_SOURCES} ${KRNL_HEADERS})

target_include_directories(krnl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# EMSCRIPTEN / Native Backend Selection ----------------------------------------
if (EMSCRIPTEN)
    message(STATUS "[krnl] Building for EMSCRIPTEN")

    target_link_libraries(krnl
        PUBLIC
            emdawnwebgpu_cpp
    )

    # Options required for async WebGPU init
    target_link_options(krnl PRIVATE
        "-sASYNCIFY=1"
        "-sUSE_GLFW=3"
    )

    # Add WebGPU include path (emscripten)
    target_include_directories(krnl PUBLIC
        ${DAWN_WEBGPU_INCLUDE_DIR}
    )

else()
    message(STATUS "[krnl] Building for native platform")

    target_link_libraries(krnl
        PUBLIC
            webgpu_dawn
            webgpu_glfw
    )

    # Add Dawn include
    target_include_directories(krnl PUBLIC
        ${DAWN_INCLUDE_DIR}
    )
endif()

# Compiler warnings ------------------------------------------------------------
if (MSVC)
    target_compile_options(krnl PRIVATE /W4)
else()
    target_compile_options(krnl PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install rule (optional) ------------------------------------------------------
install(TARGETS krnl)
install(DIRECTORY include/ DESTINATION include)
