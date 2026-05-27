#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set(supported "exynos5422;qemu-arm-virt;stm32mp2")
if(NOT "${PLATFORM}" IN_LIST supported)
    message(FATAL_ERROR "PLATFORM: ${PLATFORM} not supported.
         Supported: ${supported}")
endif()
set(VmPCISupport ON CACHE BOOL "" FORCE)
set(LibUSB OFF CACHE BOOL "" FORCE)
set(VmInitRdFile ON CACHE BOOL "" FORCE)
if(${PLATFORM} STREQUAL "qemu-arm-virt")
    # force cpu
    set(QEMU_MEMORY "2048")
    set(KernelArmCPU cortex-a53 CACHE STRING "" FORCE)
endif()
if(${PLATFORM} STREQUAL "stm32mp2")
    set(AARCH64 ON CACHE BOOL "" FORCE)
    set(KernelArmCPU cortex-a35 CACHE STRING "" FORCE)
    set(KernelAllowSMCCalls ON CACHE BOOL "" FORCE)
    set(KernelMaxNumNodes 2 CACHE STRING "" FORCE)
endif()
