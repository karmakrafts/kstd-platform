cmake_minimum_required(VERSION 3.18)
project(kstd-platform LANGUAGES C CXX)

option(KSTD_PLATFORM_BUILD_TESTS "Build unit tests for kstd-platform" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;")
include(cmx)
cmx_include_scripts()

# Library
cmx_add_library(kstd-platform SHARED "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(kstd-platform PUBLIC "${CMAKE_SOURCE_DIR}/include")
cmx_include_kstd_core(kstd-platform PUBLIC)

cmx_add_library(kstd-platform-static STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(kstd-platform-static PUBLIC "${CMAKE_SOURCE_DIR}/include")
cmx_include_kstd_core(kstd-platform-static PUBLIC)

if (CMX_PLATFORM_LINUX)
    target_link_libraries(kstd-platform PUBLIC -ldl -lrt)
    target_link_libraries(kstd-platform-static PUBLIC -ldl -lrt)
endif ()

if (CMX_PLATFORM_UNIX)
    target_link_libraries(kstd-platform PUBLIC -lresolv)
    target_link_libraries(kstd-platform-static PUBLIC -lresolv)
endif ()

# Tests
if (KSTD_PLATFORM_BUILD_TESTS)
    cmx_add_tests(kstd-platform-tests "${CMAKE_SOURCE_DIR}/test")
    cmx_include_gtest(kstd-platform-tests PRIVATE)
    target_link_libraries(kstd-platform-tests PRIVATE kstd-platform-static)
    add_dependencies(kstd-platform-tests kstd-platform-static)
endif ()