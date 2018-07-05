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

project(optiplex9020)

# Include CAmkES VM helper
include("../../vm/camkes_vm_helpers.cmake")
include("../../vm-linux/vm-linux-helpers.cmake")

# Define kernel config options
set(KernelX86Sel4Arch x86_64 CACHE STRING "" FORCE)
set(KernelMaxNumNodes 2 CACHE STRING "" FORCE)

# Retrieve Init0 extra src files
file(GLOB init0_extra src/*.c)
# Declare VM component: Init0
DeclareCAmkESVM(Init0
    EXTRA_SOURCES ${init0_extra}
    EXTRA_LIBS ethdrivers crossvm
)
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
DeclareCAmkESVMRootServer(optiplex9020.camkes CPP_INCLUDES ${CMAKE_SOURCE_DIR}/projects/vm/components/StringReverse/include)
