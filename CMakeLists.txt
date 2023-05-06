cmake_minimum_required(VERSION 3.18)
project(kstd-platform LANGUAGES C CXX)

option(KSTD_PLATFORM_BUILD_TESTS "Build unit tests for kstd-platform" ON)
option(KSTD_PLATFORM_EMBEDDED "Look up CMake infra in parent directory instead of the project directory" OFF)
set(KSTD_PLATFORM_MODULE ON) # Let other modules know we are present!
add_compile_definitions(KSTD_PLATFORM_MODULE)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    message(STATUS "No C++ version specified, defaulting to C++20")
    set(CMAKE_CXX_STANDARD 20)
endif()

if(${KSTD_PLATFORM_EMBEDDED})
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake;")
else()
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;")
endif()

include(LibraryProject)
lib_define_targets()

target_include_fmt(${LIB_STATIC_TARGET})
target_include_fmt(${LIB_SHARED_TARGET})
target_include_fmt(${LIB_TEST_TARGET})

lib_include_directories("${CMAKE_SOURCE_DIR}/kstd-concepts/include")
lib_include_directories("${CMAKE_SOURCE_DIR}/kstd-types/include")