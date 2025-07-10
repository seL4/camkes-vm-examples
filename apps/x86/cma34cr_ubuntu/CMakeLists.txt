#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(cma34cr_ubuntu)

CAmkESAddImportPath(components)

# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

# Declare VM component: Init0
DeclareCAmkESVM(Init0)

# Declare C162 Kernel
GetArchSpecificLinuxKernelFile("32" kernel_file)
DecompressLinuxKernel(
    extract_linux_kernel # helper CMake target to be created, not used
    decompressed_kernel
    ${kernel_file}
)

# Create file server with VM images. There is an implicit dependency on the
# input files, so the rules above will be invoked to generate/update the files.
# There is no need to specify an explicit dependency on the helper target
# 'extract_linux_kernel'.
DefineCAmkESVMFileServer(
    FILES "bzimage:${decompressed_kernel}"
    "rootfs.cpio:${CMAKE_CURRENT_SOURCE_DIR}/ubuntu_linux/rootfs.cpio"
)

# Declare CAmkES Root Server
DeclareCAmkESRootserver("cma34cr_centos.camkes")
