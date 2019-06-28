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

set(project_dir "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(resolved_path ${CMAKE_CURRENT_LIST_FILE} REALPATH)
# repo_dir is distinct from project_dir as this file is symlinked.
# project_dir corresponds to the top level project directory, and
# repo_dir is the absolute path after following the symlink.
get_filename_component(repo_dir ${resolved_path} DIRECTORY)

include(${project_dir}/projects/seL4_tools/cmake-tool/helpers/application_settings.cmake)

include(${repo_dir}/easy-settings.cmake)

# Kernel settings
set(KernelArch "arm" CACHE STRING "" FORCE)
if(AARCH64)
    set(KernelSel4Arch "aarch64" CACHE STRING "" FORCE)
else()
    set(KernelSel4Arch "arm_hyp" CACHE STRING "" FORCE)
    set(ARM_HYP ON CACHE INTERNAL "" FORCE)
endif()
set(KernelArmHypervisorSupport ON CACHE BOOL "" FORCE)
set(KernelArmExportPCNTUser ON CACHE BOOL "" FORCE)
set(KernelArmExportVCNTUser ON CACHE BOOL "" FORCE)
set(KernelRootCNodeSizeBits 18 CACHE STRING "" FORCE)
set(KernelMaxNumBootinfoUntypedCaps 230 CACHE STRING "")
set(KernelIRQReporting OFF CACHE BOOL "" FORCE)

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
include("${repo_dir}/apps/${CAMKES_VM_APP}/settings.cmake")

correct_platform_strings()

include(${project_dir}/kernel/configs/seL4Config.cmake)

ApplyData61ElfLoaderSettings(${KernelARMPlatform} ${KernelSel4Arch})
