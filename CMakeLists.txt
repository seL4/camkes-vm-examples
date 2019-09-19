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
include(settings.cmake)

project(camkes-vm-examples C ASM)

camkes_x86_vm_setup_x86_vm_environment()

add_subdirectory("${CAMKES_VM_APP}")
GenerateCAmkESRootserver()

if(SIMULATION)
    GenerateSimulateScript()
endif()
