#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(minimal_64)

# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})
# message(WARNING "${CAMKES_VM_HELPERS_PATH}")

# message(FATAL_ERROR "${CAMKES_VM_LINUX_HELPERS_PATH}")

# Declare VM component: Init0
DeclareCAmkESVM(Init0)

# Get Default Linux VM files
GetArchDefaultLinuxKernelFile("64" kernel_file)
GetArchDefaultLinuxRootfsFile("64" rootfs_file)

# Decompress Linux Kernel image and add to file server
DecompressLinuxKernel(extract_linux_kernel decompressed_kernel ${kernel_file})
AddToFileServer("bzimage" ${decompressed_kernel} DEPENDS extract_linux_kernel)

# Add rootfs images into file server
AddToFileServer("rootfs.cpio" ${rootfs_file})

# Initialise CAmkES Root Server with addition CPP includes
DeclareCAmkESVMRootServer(minimal_64.camkes)

if(SIMULATION)
	include(simulation)
    GenerateSimulateScript()
endif()
