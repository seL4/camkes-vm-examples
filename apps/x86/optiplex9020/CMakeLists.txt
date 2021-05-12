#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(optiplex9020)

# Include CAmkES VM helper
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

# Retrieve Init0 extra src files
file(GLOB init0_extra src/*.c)
# Declare VM component: Init0
DeclareCAmkESVM(Init0 EXTRA_SOURCES ${init0_extra} EXTRA_LIBS ethdrivers)
# Declare VM component: Init1
DeclareCAmkESVM(Init1)

# Get Default Linux VM files
GetDefaultLinuxKernelFile(kernel_file)
GetDefaultLinuxRootfsFile(rootfs_file)

# Decompress Linux Kernel image
DecompressLinuxKernel(extract_linux_kernel decompressed_kernel ${kernel_file})

# Add Linux VM images into file server
AddToFileServer("bzimage" ${decompressed_kernel} DEPENDS extract_linux_kernel)
AddToFileServer("rootfs.cpio" ${rootfs_file})

# Initialise CAmkES Root Server with additional CPP includes
DeclareCAmkESVMRootServer(
    optiplex9020.camkes CPP_INCLUDES ${CAMKES_VM_DIR}/components/StringReverse/include
)
