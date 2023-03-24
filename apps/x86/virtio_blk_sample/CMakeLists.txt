#
# Copyright 2020, DornerWorks
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(virtio_blk_sample)

# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

# Declare VM components: Init0
DeclareCAmkESVM(Init0)

# Get Default Linux VM files
GetDefaultLinuxKernelFile(kernel_file)
GetDefaultLinuxRootfsFile(rootfs_file)

# Decompress Linux Kernel image and add to file server
DecompressLinuxKernel(
    extract_linux_kernel # helper CMake target to be created, not used
    decompressed_kernel
    ${kernel_file}
)

# Create file server with VM images. There is an implicit dependency on the
# input files, so the rules above will be invoked to generate/update the files.
# There is no need to specify an explicit dependency on the helper target
# 'extract_linux_kernel'.
DefineCAmkESVMFileServer(FILES "bzimage:${decompressed_kernel}" "rootfs.cpio:${rootfs_file}")

# Declare CAmkES Root Server
DeclareCAmkESRootserver("virtio_blk_sample.camkes")

if(SIMULATION)
    include(simulation)
    GenerateSimulateScript()
endif()
