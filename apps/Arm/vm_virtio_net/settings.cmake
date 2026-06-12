#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set(supported "exynos5422;tx2;zcu102")
if(NOT "${PLATFORM}" IN_LIST supported)
    message(FATAL_ERROR "PLATFORM: ${PLATFORM} not supported.
         Supported: ${supported}")
endif()

set(LibUSB OFF CACHE BOOL "" FORCE)
set(VmPCISupport ON CACHE BOOL "" FORCE)
if(VIRTIO_NET_PING)
    set(VmVirtioNetVirtqueue ON CACHE BOOL "" FORCE)
else()
    set(VmVirtioNetArping ON CACHE BOOL "" FORCE)
endif()
set(VmInitRdFile ON CACHE BOOL "" FORCE)
set(VmDtbFile ON CACHE BOOL "provide dtb" FORCE)

if("${PLATFORM}" STREQUAL "exynos5422")
    # nothing special here
elseif("${PLATFORM}" STREQUAL "tx2")
    # nothing special here
elseif(${PLATFORM} STREQUAL "zcu102")
    set(AARCH64 ON CACHE BOOL "" FORCE)
    set(KernelAllowSMCCalls ON CACHE BOOL "" FORCE)
    set(VmZynqmpPetalinuxVersion 2022_1 CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "unsupported PLATFORM: ${PLATFORM}")
endif()
