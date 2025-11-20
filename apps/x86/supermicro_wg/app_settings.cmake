#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

# Define kernel config options
set(KernelSel4Arch x86_64 CACHE STRING "" FORCE)
set(KernelMaxNumNodes 1 CACHE STRING "" FORCE)
set(RELEASE ON CACHE BOOL "" FORCE)

set(supported "qemu;e300-9d")
if(NOT "${PLATFORM}" IN_LIST supported)
    message(FATAL_ERROR "PLATFORM: ${PLATFORM} not supported.
         Supported: ${supported}")
endif()

if(${PLATFORM} STREQUAL "qemu")
    set(LibPlatSupportX86ConsoleDevice "com1" CACHE STRING "" FORCE)
endif()
if(${PLATFORM} STREQUAL "e300-9d")
    set(LibPlatSupportX86ConsoleDevice "com2" CACHE STRING "" FORCE)
endif()