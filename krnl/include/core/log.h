#pragma once

#include <iostream>
#include <sstream>

#include <dawn/webgpu_cpp_print.h>

// Detect build mode
#if !defined(NDEBUG)
#define KRNL_DEBUG 1
#else
#define KRNL_DEBUG 0
#endif

// Platform-specific color support
#if defined(__EMSCRIPTEN__)
#define KRNL_COLOR_RESET   ""
#define KRNL_COLOR_BLUE    ""
#define KRNL_COLOR_GREEN   ""
#define KRNL_COLOR_YELLOW  ""
#define KRNL_COLOR_RED     ""
#else
#define KRNL_COLOR_RESET   "\033[0m"
#define KRNL_COLOR_BLUE    "\033[34m"
#define KRNL_COLOR_GREEN   "\033[32m"
#define KRNL_COLOR_YELLOW  "\033[33m"
#define KRNL_COLOR_RED     "\033[31m"
#endif

#if KRNL_DEBUG

#define KRNL_LOG(msg) \
        do { \
            std::cout << KRNL_COLOR_GREEN \
                      << "[KRNL] " << __FILE__ << ":" << __LINE__ << ": " \
                      << msg \
                      << KRNL_COLOR_RESET << std::endl; \
        } while (0)

#define KRNL_WARN(msg) \
        do { \
            std::cout << KRNL_COLOR_YELLOW \
                      << "[KRNL-WARN] " << __FILE__ << ":" << __LINE__ << ": " \
                      << msg \
                      << KRNL_COLOR_RESET << std::endl; \
        } while (0)

#define KRNL_ERROR(msg) \
        do { \
            std::cout << KRNL_COLOR_RED \
                      << "[KRNL-ERROR] " << __FILE__ << ":" << __LINE__ << ": " \
                      << msg \
                      << KRNL_COLOR_RESET << std::endl; \
        } while (0)

#else

#define KRNL_LOG(msg)   do {} while (0)
#define KRNL_WARN(msg)  do {} while (0)
#define KRNL_ERROR(msg) do {} while (0)

#endif
