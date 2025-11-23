cmake_minimum_required(VERSION 3.22)
project(krnl_root)

set(CMAKE_CXX_STANDARD 20)

# Export compile commands for editors (e.g. clangd / VS Code C/C++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory(external/dawn EXCLUDE_FROM_ALL)

add_subdirectory(krnl)
add_subdirectory(sandbox)
add_subdirectory(samples)
