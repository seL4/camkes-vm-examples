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

find_file(
    VM_SETTINGS_PATH camkes_vm_settings.cmake
    PATHS ${CMAKE_SOURCE_DIR}/projects/vm/
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

include("${CMAKE_CURRENT_LIST_DIR}/${CAMKES_VM_APP}/app_settings.cmake")

if(SIMULATION)
    ApplyCommonSimulationSettings(${KernelArch})
    # Force IOMMU back on after CommonSimulationSettings disabled it
    set(KernelIOMMU ON CACHE BOOL "" FORCE)
endif()
