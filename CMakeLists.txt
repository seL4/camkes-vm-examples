#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)
include(settings.cmake)
project(camkes-vm C CXX ASM)

if(AppArch STREQUAL "Arm")
	find_package(camkes-arm-vm REQUIRED)
	camkes_arm_vm_setup_arm_vm_environment()
elseif (AppArch STREQUAL "x86")
	camkes_x86_vm_setup_x86_vm_environment()
else()
	message(FATAL_ERROR "Unsupported")
endif()
# Add VM application
add_subdirectory("apps/${AppArch}/${CAMKES_VM_APP}")
# Should be done adding targets, can now generate the root server and the global configuration
GenerateCAmkESRootserver()
