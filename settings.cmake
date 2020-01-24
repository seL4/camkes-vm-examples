#
# Copyright 2019, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(DATA61_BSD)
#

cmake_minimum_required(VERSION 3.7.2)
set(project_dir "${CMAKE_CURRENT_LIST_DIR}/../../")
file(GLOB project_modules ${project_dir}/projects/*)
list(
    APPEND
        CMAKE_MODULE_PATH
        ${project_dir}/kernel
        ${project_dir}/tools/seL4/cmake-tool/helpers/
        ${project_dir}/tools/seL4/elfloader-tool/
        ${project_modules}
)
set(SEL4_CONFIG_DEFAULT_ADVANCED ON)
set(CAMKES_CONFIG_DEFAULT_ADVANCED ON)
mark_as_advanced(CMAKE_INSTALL_PREFIX)
include(application_settings)

include(${CMAKE_CURRENT_LIST_DIR}/easy-settings.cmake)

# Kernel settings
set(KernelArch "arm" CACHE STRING "" FORCE)
if(AARCH64)
    set(KernelSel4Arch "aarch64" CACHE STRING "" FORCE)
else()
    set(KernelSel4Arch "arm_hyp" CACHE STRING "" FORCE)
    set(ARM_HYP ON CACHE INTERNAL "" FORCE)
endif()
set(KernelArmHypervisorSupport ON CACHE BOOL "" FORCE)
set(KernelRootCNodeSizeBits 18 CACHE STRING "" FORCE)
set(KernelArmVtimerUpdateVOffset OFF CACHE BOOL "" FORCE)
set(KernelArmDisableWFIWFETraps ON CACHE BOOL "" FORCE)

# capDL settings
set(CapDLLoaderMaxObjects 90000 CACHE STRING "" FORCE)

# CAmkES Settings
set(CAmkESCPP ON CACHE BOOL "" FORCE)

# Release settings
ApplyCommonReleaseVerificationSettings(${RELEASE} FALSE)

if(NOT CAMKES_VM_APP)
    message(
        FATAL_ERROR
            "CAMKES_VM_APP is not defined. Pass CAMKES_VM_APP to specify the VM application to build e.g. vm_minimal, odroid_vm"
    )
endif()

# Add VM application
include("${CMAKE_CURRENT_LIST_DIR}/apps/${CAMKES_VM_APP}/settings.cmake")

correct_platform_strings()

find_package(seL4 REQUIRED)
sel4_configure_platform_settings()

ApplyData61ElfLoaderSettings(${KernelARMPlatform} ${KernelSel4Arch})
