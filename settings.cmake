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

set(project_dir "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(resolved_path ${CMAKE_CURRENT_LIST_FILE} REALPATH)
# repo_dir is distinct from project_dir as this file is symlinked.
# project_dir corresponds to the top level project directory, and
# repo_dir is the absolute path after following the symlink.
get_filename_component(repo_dir ${resolved_path} DIRECTORY)

include(${project_dir}/projects/seL4_tools/cmake-tool/helpers/application_settings.cmake)

find_file(
    VM_SETTINGS_PATH camkes_vm_settings.cmake
    PATHS ${project_dir}/projects/vm/
    CMAKE_FIND_ROOT_PATH_BOTH
)
mark_as_advanced(FORCE VM_SETTINGS_PATH)
if("${VM_SETTINGS_PATH}" STREQUAL "VM_SETTINGS_PATH-NOTFOUND")
    message(
        FATAL_ERROR
            "Failed to find camkes_vm_settings.cmake. Consider cmake -DVM_SETTINGS_PATH=/path/to/camkes_vm_settings.cmake"
    )
endif()
include(${VM_SETTINGS_PATH})

if(NOT CAMKES_VM_APP)
    set(CAMKES_VM_APP "optiplex9020" CACHE INTERNAL "")
endif()

include("${repo_dir}/${CAMKES_VM_APP}/app_settings.cmake")

include(${project_dir}/kernel/configs/seL4Config.cmake)

if(SIMULATION)
    ApplyCommonSimulationSettings(${KernelArch})
    # Force IOMMU back on after CommonSimulationSettings disabled it
    set(KernelIOMMU ON CACHE BOOL "" FORCE)
endif()
