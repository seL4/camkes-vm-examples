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

include(application_settings)

find_package(camkes-vm REQUIRED)
include(${CAMKES_VM_SETTINGS_PATH})

if(NOT CAMKES_VM_APP)
    set(CAMKES_VM_APP "optiplex9020" CACHE INTERNAL "")
endif()

include("${repo_dir}/${CAMKES_VM_APP}/app_settings.cmake")

find_package(seL4 REQUIRED)
sel4_configure_platform_settings()

if(SIMULATION)
    ApplyCommonSimulationSettings(${KernelArch})
    # Force IOMMU back on after CommonSimulationSettings disabled it
    set(KernelIOMMU ON CACHE BOOL "" FORCE)
endif()
