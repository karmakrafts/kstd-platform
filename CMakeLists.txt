cmake_minimum_required(VERSION 3.18)
project(kstd-platform LANGUAGES C CXX)

option(KSTD_PLATFORM_BUILD_TESTS "Build unit tests for kstd-platform" OFF)
set(KSTD_PLATFORM_MODULE ON) # Let other modules know we are present!
add_compile_definitions(KSTD_PLATFORM_MODULE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;")
include(cmx)
cmx_include_scripts()

# Library
cmx_add_library(kstd-platform "${CMAKE_SOURCE_DIR}/src")

target_include_directories(kstd-platform PUBLIC "${CMAKE_SOURCE_DIR}/include")
cmx_include_fmt(kstd-platform PUBLIC)
cmx_include_kstd_core(kstd-platform PUBLIC)

target_include_directories(kstd-platform-static PUBLIC "${CMAKE_SOURCE_DIR}/include")
cmx_include_fmt(kstd-platform-static PUBLIC)
cmx_include_kstd_core(kstd-platform-static PUBLIC)

# Tests
if(KSTD_PLATFORM_BUILD_TESTS)
	cmx_add_tests(kstd-platform-tests "${CMAKE_SOURCE_DIR}/test")
    target_include_directories(kstd-platform-tests PRIVATE "${CMAKE_SOURCE_DIR}/include")
    cmx_include_fmt(kstd-platform-tests PUBLIC)
    cmx_include_kstd_core(kstd-platform-tests PUBLIC)
    target_link_libraries(kstd-platform-tests PRIVATE kstd-platform-static)
    add_dependencies(kstd-platform-tests kstd-platform-static)
endif()

if(CMX_PLATFORM_LINUX)
    target_link_libraries(kstd-platform PUBLIC -ldl -lrt)
    target_link_libraries(kstd-platform-static PUBLIC -ldl -lrt)
    if(KSTD_PLATFORM_BUILD_TESTS)
        target_link_libraries(kstd-platform-tests PRIVATE -ldl -lrt)
    endif()
endif()
