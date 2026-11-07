cmake_minimum_required(VERSION 3.21)

project(vectorpie_screensavers LANGUAGES C CXX)

# Screensaver apps for VectorPie, each in its own folder: the cabinet runs
# one instead of its own screensaver and it draws to the cabinet's music
# (vendor/VectorPie/sdk/docs/09-screensavers.md). All of them share the
# SDK (vendor/VectorPie) and this build. Built like Vector Racer:
#   PC, natively (MSYS2 MinGW-w64 with SDL2):  see BUILD.md
#   Pi, with clang against a copy of the cabinet's libraries: cmake/toolchain-pi-clang.cmake
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(VPS_PI_BUILD)
    # cmake/toolchain-pi-clang.cmake: the cabinet's own SDL2, by its file.
    add_library(SDL2::SDL2 SHARED IMPORTED GLOBAL)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu/libSDL2-2.0.so.0"
        IMPORTED_SONAME "libSDL2-2.0.so.0"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SYSROOT}/usr/include/SDL2"
        INTERFACE_COMPILE_DEFINITIONS "_REENTRANT")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

set(VP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(VP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/VectorPie/sdk vectorpie)

# The screensavers: each builds into build/<pc|pi>/<folder>/.
add_subdirectory(neondrive)
add_subdirectory(digitalrain)
