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

project(cma34cr_ubuntu)

# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

# Declare VM component: Init0
DeclareCAmkESVM(Init0)

# Declare C162 Kernel
GetDefaultLinuxKernelFile(kernel_file)
DecompressLinuxKernel(extract_linux_kernel decompressed_kernel ${kernel_file})
AddToFileServer("bzimage" ${decompressed_kernel} DEPENDS extract_linux_kernel)

# Declare C162 Rootfs
AddToFileServer("rootfs.cpio" ${CMAKE_CURRENT_SOURCE_DIR}/ubuntu_linux/rootfs.cpio)

# Initialise CAmkES Root Server with addition CPP includes
DeclareCAmkESVMRootServer(cma34cr_ubuntu.camkes)
