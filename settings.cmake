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

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/apps/Arm/${CAMKES_VM_APP}")
    set(AppArch "Arm" CACHE STRING "" FORCE)
elseif (EXISTS "${CMAKE_CURRENT_LIST_DIR}/apps/x86/${CAMKES_VM_APP}")
    set(AppArch "x86" CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "App does not exist for supported architecture")
endif()

if (AppArch STREQUAL "Arm")
    set(CAMKES_ARM_LINUX_DIR "${CMAKE_CURRENT_LIST_DIR}/linux" CACHE STRING "")

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
    # message(FATAL_ERROR "release is   ${RELEASE}")
    ApplyCommonReleaseVerificationSettings(${RELEASE} FALSE)

    if(NOT CAMKES_VM_APP)
        message(
            FATAL_ERROR
                "CAMKES_VM_APP is not defined. Pass CAMKES_VM_APP to specify the VM application to build e.g. vm_minimal, odroid_vm"
        )
    endif()

    # Add VM application
    include("${CMAKE_CURRENT_LIST_DIR}/apps/Arm/${CAMKES_VM_APP}/settings.cmake")

    correct_platform_strings()

    find_package(seL4 REQUIRED)
    sel4_configure_platform_settings()

    ApplyData61ElfLoaderSettings(${KernelARMPlatform} ${KernelSel4Arch})

    if(NUM_NODES MATCHES "^[0-9]+$")
        set(KernelMaxNumNodes ${NUM_NODES} CACHE STRING "" FORCE)
    else()
        set(KernelMaxNumNodes 1 CACHE STRING "" FORCE)
    endif()

    # We dont support SMP configurations on the exynos5422, exynos5410 or TK1
    if(
        ("${KernelARMPlatform}" STREQUAL "exynos5422"
         OR "${KernelARMPlatform}" STREQUAL "exynos5410"
         OR "${KernelARMPlatform}" STREQUAL "tk1"
         )
        AND (${KernelMaxNumNodes} GREATER 1)
    )
        message(FATAL_ERROR "${KernelARMPlatform} does not support SMP VMs")
    endif()

elseif (AppArch STREQUAL "x86")

    set(project_dir "${CMAKE_CURRENT_LIST_DIR}")
    get_filename_component(resolved_path ${CMAKE_CURRENT_LIST_FILE} REALPATH)
    # repo_dir is distinct from project_dir as this file is symlinked.
    # project_dir corresponds to the top level project directory, and
    # repo_dir is the absolute path after following the symlink.
    get_filename_component(repo_dir ${resolved_path} DIRECTORY)
    set(project_dir "${CMAKE_CURRENT_LIST_DIR}/../../")
    file(GLOB project_modules ${project_dir}/projects/*)
    list(
        APPEND
            CMAKE_MODULE_PATH
            ${project_dir}/kernel
            ${project_dir}/projects/seL4_tools/cmake-tool/helpers/
            ${project_dir}/projects/seL4_tools/elfloader-tool/
            ${project_dir}/tools/seL4/cmake-tool/helpers/
            ${project_dir}/tools/seL4/elfloader-tool/
            ${project_modules}
    )

    set(LIBZMQ_PATH ${project_dir}/projects/libzmq CACHE INTERNAL "")

    include(application_settings)

    # message(FATAL_ERROR "${CAMKES_VM_SETTINGS_PATH}")

    find_package(camkes-vm REQUIRED)
    include(${CAMKES_VM_SETTINGS_PATH})

    if(NOT CAMKES_VM_APP)
        set(CAMKES_VM_APP "optiplex9020" CACHE INTERNAL "")
    endif()

    # message(WARNING "project dir is ${repo_dir}")
    # message(FATAL_ERROR ${repo_dir}/apps/${CAMKES_VM_APP}/app_settings.cmake"")
    include("${repo_dir}/apps/x86/${CAMKES_VM_APP}/app_settings.cmake")

    find_package(seL4 REQUIRED)
    sel4_configure_platform_settings()

    if(SIMULATION)
        ApplyCommonSimulationSettings(${KernelSel4Arch})
        # Force IOMMU back on after CommonSimulationSettings disabled it
        set(KernelIOMMU ON CACHE BOOL "" FORCE)
    endif()

else()
    message(FATAL_ERROR "Unsupported Setting")
endif()
