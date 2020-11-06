#
# Copyright 2018, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(DATA61_BSD)
#

set(supported "tk1;tx1;tx2;exynos5422;qemu-arm-virt;odroidc2")
if(NOT "${PLATFORM}" IN_LIST supported)
    message(FATAL_ERROR "PLATFORM: ${PLATFORM} not supported.
         Supported: ${supported}")
endif()
if(${PLATFORM} STREQUAL "tk1")
    set(KernelTk1SMMU ON CACHE BOOL "" FORCE)
    set(KernelTk1SMMUInterruptEnable ON CACHE BOOL "" FORCE)
endif()

set(LibUSB OFF CACHE BOOL "" FORCE)
if(${PLATFORM} STREQUAL "exynos5422")
    set(VmPCISupport ON CACHE BOOL "" FORCE)
    set(VmVirtioNet ON CACHE BOOL "" FORCE)
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
endif()
if(${PLATFORM} STREQUAL "tx2")
    set(VmPCISupport ON CACHE BOOL "" FORCE)
    set(VmVirtioNet ON CACHE BOOL "" FORCE)
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
    set(VmDtbFile ON CACHE BOOL "" FORCE)
endif()
if(${PLATFORM} STREQUAL "odroidc2")
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
    set(VmDtbFile ON CACHE BOOL "" FORCE)
endif()
if(${PLATFORM} STREQUAL "qemu-arm-virt")
    # force cpu
    set(QEMU_MEMORY "2048")
    set(KernelArmCPU cortex-a53 CACHE STRING "" FORCE)
    set(VmInitRdFile ON CACHE BOOL "" FORCE)
endif()
