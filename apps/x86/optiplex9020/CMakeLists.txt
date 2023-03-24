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

# Get 32-bit Linux VM files
GetArchDefaultLinuxKernelFile("32" kernel_file)
GetArchDefaultLinuxRootfsFile("32" rootfs_file)

# Decompress Linux Kernel image
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
DeclareCAmkESRootserver(
    "optiplex9020.camkes"
    CPP_INCLUDES
    "${VM_PROJECT_DIR}/components/VM"
    "${CAMKES_VM_DIR}/components/StringReverse/include"
)
