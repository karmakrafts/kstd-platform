cmake_minimum_required(VERSION 3.18)
project(kstd-platform LANGUAGES C CXX)

option(KSTD_PLATFORM_BUILD_TESTS "Build unit tests for kstd-platform" OFF)
set(KSTD_PLATFORM_MODULE ON) # Let other modules know we are present!
add_compile_definitions(KSTD_PLATFORM_MODULE)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    message(STATUS "No C++ version specified, defaulting to C++20")
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;")
include(CommonLibraries)
include(Platform)
include(KstdCore)

file(GLOB_RECURSE KSTD_PLATFORM_SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Shared
add_library(kstd-platform SHARED ${KSTD_PLATFORM_SOURCE_FILES})
target_include_directories(kstd-platform PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_include_fmt(kstd-platform)
target_include_kstd_core(kstd-platform)

# Static
add_library(kstd-platform-static STATIC ${KSTD_PLATFORM_SOURCE_FILES})
target_include_directories(kstd-platform-static PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_include_fmt(kstd-platform-static)
target_include_kstd_core(kstd-platform-static)

# Tests
if(KSTD_PLATFORM_BUILD_TESTS)
    file(GLOB_RECURSE KSTD_PLATFORM_TEST_SOURCE_FILES "${CMAKE_SOURCE_DIR}/test/*.cpp")
    add_executable(kstd-platform-tests ${KSTD_PLATFORM_TEST_SOURCE_FILES})
    target_include_directories(kstd-platform-tests PUBLIC "${CMAKE_SOURCE_DIR}/include")
    target_include_fmt(kstd-platform-tests)
    target_include_kstd_core(kstd-platform-tests)
    target_include_gtest(kstd-platform-tests)
    target_link_libraries(kstd-platform-tests kstd-platform-static)
    add_dependencies(kstd-platform-tests kstd-platform-static)
endif()

if(PLATFORM_LINUX)
    target_link_libraries(kstd-platform -ldl -lrt)
    target_link_libraries(kstd-platform-static -ldl -lrt)
    if(KSTD_PLATFORM_BUILD_TESTS)
        target_link_libraries(kstd-platform-tests -ldl -lrt)
    endif()
endif()