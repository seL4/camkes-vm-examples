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

cmake_minimum_required(VERSION 3.8.2)

if (NOT ${PLATFORM} IN_LIST KernelARMPlatform_all_strings)
    message(FATAL_ERROR "Unknown ARM PLATFORM")
endif()

# Kernel settings
set(KernelArch "arm" CACHE STRING "" FORCE)
set(KernelArmSel4Arch "arm_hyp" CACHE STRING "" FORCE)
set(KernelARMPlatform ${PLATFORM} CACHE STRING "" FORCE)
set(KernelArmHypervisorSupport ON CACHE BOOL "" FORCE)
set(KernelArmExportPCNTUser ON CACHE BOOL "" FORCE)
set(KernelArmExportVCNTUser ON CACHE BOOL "" FORCE)
set(KernelRootCNodeSizeBits 19 CACHE STRING "" FORCE)
set(KernelMaxNumBootinfoUntypedCaps 230 CACHE STRING "")
set(KernelIRQReporting OFF CACHE BOOL "" FORCE)
if(${KernelARMPlatform} STREQUAL "tk1")
    set(KernelArmSMMU ON CACHE BOOL "" FORCE)
    set(KernelARMSMMUInterruptEnable ON CACHE BOOL "" FORCE)
endif()

# capDL settings
set(CapDLLoaderMaxObjects 90000 CACHE STRING "" FORCE)

# Elf Loader settings
set(ElfloaderMode "hypervisor" CACHE STRING "" FORCE)
set(ElfloaderMonitorHook ON CACHE STRING "" FORCE)

# CAmkES Settings
set(CAmkESCPP ON CACHE BOOL "" FORCE)

# Release settings
set(RELEASE OFF CACHE BOOL "Performance optimized build")
ApplyCommonReleaseVerificationSettings(${RELEASE} FALSE)

# Common build definitions
CAmkESAddImportPath(components)
CAmkESAddImportPath(interfaces)

# VM components
add_subdirectory(components/Vchan)
add_subdirectory(components/VM)

# Add VM application
add_subdirectory(apps/vm)
