cmake_minimum_required(VERSION 3.18)
project(kstd-platform LANGUAGES C CXX)

option(KSTD_PLATFORM_BUILD_TESTS "Build unit tests for kstd-platform" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;")
include(cmx-bootstrap)
cmx_include_scripts()

# Library
cmx_add_library(kstd-platform SHARED "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(kstd-platform PUBLIC "${CMAKE_SOURCE_DIR}/include")
cmx_include_kstd_core(kstd-platform PUBLIC)
cmx_include_kstd_streams(kstd-platform PUBLIC)

cmx_add_library(kstd-platform-static STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(kstd-platform-static PUBLIC "${CMAKE_SOURCE_DIR}/include")
cmx_include_kstd_core(kstd-platform-static PUBLIC)
cmx_include_kstd_streams(kstd-platform-static PUBLIC)

if (CMX_PLATFORM_UNIX)
    target_link_libraries(kstd-platform PUBLIC -lresolv)
    target_link_libraries(kstd-platform-static PUBLIC -lresolv)
endif ()

if (CMX_PLATFORM_LINUX)
    # Check if all needed libnl modules are installed on the build system
    find_library(LIBNL_LIBRARY NAMES nl nl-3)
    find_library(LIBNL_ROUTE_LIBRARY NAMES nl-route nl-route-3)
    find_library(LIBNL_GENL_LIBRARY NAMES nl-genl nl-genl-3)
    if (NOT LIBNL_LIBRARY OR NOT LIBNL_ROUTE_LIBRARY OR NOT LIBNL_GENL_LIBRARY)
        message(FATAL_ERROR "There are missing some dependencies! Please run install_deps.sh!")
    endif ()

    # Find include path of libnl-3
    find_path(LIBNL_INCLUDE_DIR "netlink/netlink.h"
            "/usr/include"
            "/usr/include/libnl3"
            "/usr/local/include"
            "/usr/local/include/libnl3")

    # Include libnl include directories
    target_include_directories(kstd-platform PUBLIC ${LIBNL_INCLUDE_DIR})
    target_include_directories(kstd-platform-static PUBLIC ${LIBNL_INCLUDE_DIR})

    # Link libraries
    target_link_libraries(kstd-platform PUBLIC -ldl -lrt -lnl-3 -lnl-route-3 -lnl-nf-3 -lnl-genl-3)
    target_link_libraries(kstd-platform-static PUBLIC -ldl -lrt -lnl-3 -lnl-route-3 -lnl-nf-3 -lnl-genl-3)
endif()

# Tests
if (KSTD_PLATFORM_BUILD_TESTS)
    cmx_add_tests(kstd-platform-tests "${CMAKE_SOURCE_DIR}/test")
    target_link_libraries(kstd-platform-tests PRIVATE kstd-platform-static)
    add_dependencies(kstd-platform-tests kstd-platform-static)
endif ()
