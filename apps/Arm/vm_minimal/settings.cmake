#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set(supported "tk1;tx1;tx2;exynos5422;qemu-arm-virt;odroidc2;zcu102")
if(NOT "${PLATFORM}" IN_LIST supported)
    message(FATAL_ERROR "PLATFORM: ${PLATFORM} not supported.
         Supported: ${supported}")
endif()

set(LibUSB OFF CACHE BOOL "" FORCE)

if(${PLATFORM} STREQUAL "tk1")
    set(KernelTk1SMMU ON CACHE BOOL "" FORCE)
    set(KernelTk1SMMUInterruptEnable ON CACHE BOOL "" FORCE)

elseif(${PLATFORM} STREQUAL "exynos5422")
    set(VmPCISupport ON CACHE BOOL "" FORCE)
    set(VmVirtioNet ON CACHE BOOL "" FORCE)
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
elseif(${PLATFORM} STREQUAL "tx1")
    # nothing here
elseif(${PLATFORM} STREQUAL "tx2")
    set(VmPCISupport ON CACHE BOOL "" FORCE)
    set(VmVirtioNet ON CACHE BOOL "" FORCE)
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
    set(VmDtbFile ON CACHE BOOL "" FORCE)
elseif(${PLATFORM} STREQUAL "odroidc2")
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
    set(VmDtbFile ON CACHE BOOL "" FORCE)
elseif(${PLATFORM} STREQUAL "qemu-arm-virt")
    # force cpu
    set(QEMU_MEMORY "2048")
    set(KernelArmCPU cortex-a53 CACHE STRING "" FORCE)
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
elseif(${PLATFORM} STREQUAL "zcu102")
    set(AARCH64 ON CACHE BOOL "" FORCE)
    set(KernelAllowSMCCalls ON CACHE BOOL "" FORCE)
else()
    message(FATAL_ERROR "unsupported PLATFORM: ${PLATFORM}")
endif()
