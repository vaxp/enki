# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


cmake_minimum_required(VERSION 3.13...3.26)
project(anucore)
set(CMAKE_VERBOSE_MAKEFILE on)

if(TARGET anucore)
    return()
endif()

include(CheckIPOSupported)

set(ANU_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
include(${ANU_ROOT}/cmake/project-defaults.cmake)


file(GLOB SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)

add_library(anucore STATIC ${SOURCES})

# Anu conditionally uses <android/log> when building for Android
if (ANDROID)
    target_link_libraries(anucore log)
endif()

check_ipo_supported(RESULT result)
if(result)
  set_target_properties(anucore PROPERTIES
    CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
endif()

target_include_directories(anucore
    PUBLIC
    $<BUILD_INTERFACE:${ANU_ROOT}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/anu>)
