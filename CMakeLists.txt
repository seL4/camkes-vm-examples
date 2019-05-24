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
    GLOBAL_COMPONENTS_PATH global-components.cmake
    PATHS ${CMAKE_SOURCE_DIR}/projects/global-components/
    CMAKE_FIND_ROOT_PATH_BOTH
)
mark_as_advanced(FORCE GLOBAL_COMPONENTS_PATH)
if("${GLOBAL_COMPONENTS_PATH}" STREQUAL "GLOBAL_COMPONENTS_PATH-NOTFOUND")
    message(
        FATAL_ERROR
            "Failed to find global-components.cmake. Consider cmake -DGLOBAL_COMPONENTS_PATH=/path/to/global-components.cmake"
    )
endif()
include(${GLOBAL_COMPONENTS_PATH})

find_file(
    CAMKES_VM_HELPERS_PATH camkes_vm_helpers.cmake
    PATHS ${CMAKE_SOURCE_DIR}/projects/camkes-vm/
    CMAKE_FIND_ROOT_PATH_BOTH
)
mark_as_advanced(FORCE CAMKES_VM_HELPERS_PATH)
if("${CAMKES_VM_HELPERS_PATH}" STREQUAL "CAMKES_VM_HELPERS_PATH-NOTFOUND")
    message(
        FATAL_ERROR
            "Failed to find camkes_vm_helpers.cmake. Consider cmake -DCAMKES_VM_HELPERS_PATH=/path/to/camkes_vm_helpers.cmake"
    )
endif()
include(${CAMKES_VM_HELPERS_PATH})

# Common build definitions
CAmkESAddImportPath(components)
CAmkESAddImportPath(interfaces)

# Add libraries
add_subdirectory(libs/libvirtio)

# Add VM application
add_subdirectory("apps/${CAMKES_VM_APP}")

# VM components
add_subdirectory(components/Vchan)
add_subdirectory(components/VM)
