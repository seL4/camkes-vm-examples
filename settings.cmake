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

# Kernel settings
set(KernelArch "arm" CACHE STRING "" FORCE)
set(KernelArmSel4Arch "arm_hyp" CACHE STRING "" FORCE)
set(KernelArmHypervisorSupport ON CACHE BOOL "" FORCE)
set(KernelArmExportPCNTUser ON CACHE BOOL "" FORCE)
set(KernelArmExportVCNTUser ON CACHE BOOL "" FORCE)
set(KernelRootCNodeSizeBits 19 CACHE STRING "" FORCE)
set(KernelMaxNumBootinfoUntypedCaps 230 CACHE STRING "")
set(KernelIRQReporting OFF CACHE BOOL "" FORCE)

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

if(NOT CAMKES_VM_APP)
    message(FATAL_ERROR "CAMKES_VM_APP is not defined. Pass CAMKES_VM_APP to specify the VM application to build e.g. tk1_vm, odroid_vm")
endif()

# Make CAMKES_VM_APP available to change in the ccmake interface
set(CAMKES_VM_APP "${CAMKES_VM_APP}" CACHE STRING "CAmkES VM application to build")

# Add VM application
include("${CMAKE_CURRENT_LIST_DIR}/apps/${CAMKES_VM_APP}/settings.cmake")
