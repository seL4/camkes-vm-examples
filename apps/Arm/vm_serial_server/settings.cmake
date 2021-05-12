#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set(supported "exynos5422")
if(NOT "${PLATFORM}" IN_LIST supported)
    message(FATAL_ERROR "PLATFORM: ${PLATFORM} not supported.
         Supported: ${supported}")
endif()
set(KernelARMPlatform "${PLATFORM}" CACHE STRING "" FORCE)
set(LibUSB OFF CACHE BOOL "" FORCE)
set(VmPCISupport ON CACHE BOOL "" FORCE)
set(VmVirtioConsole ON CACHE BOOL "" FORCE)
set(VmInitRdFile ON CACHE BOOL "" FORCE)
set(VmDtbFile ON CACHE BOOL "provide dtb" FORCE)
