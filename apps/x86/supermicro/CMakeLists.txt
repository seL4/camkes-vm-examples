

#
# Copyright 2022, UNSW (ABN 57 195 873 179)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(supermicro)

# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

set(NumberVMs 4)
MATH(EXPR NumberVMsLess1 "${NumberVMs}-1")

# Declare VM component: Init
foreach(i RANGE ${NumberVMsLess1})
    file(GLOB "init${i}_extra" "src/cross_vm_connections_vm${i}.c")
    DeclareCAmkESVM("Init${i}" EXTRA_SOURCES "${init${i}_extra}")
endforeach()

DeclareCamkesComponent(virtqueueinit)

# Get 32-bit Linux VM files
GetSupermicroLinuxKernelFile(kernel_file)
GetArchDefaultLinuxRootfsFile("32" rootfs_file)

# Decompress Linux Kernel image
DecompressLinuxKernel(extract_linux_kernel decompressed_kernel ${kernel_file})

# Add Linux VM images into file server
AddToFileServer("bzimage" ${decompressed_kernel} DEPENDS extract_linux_kernel)
AddToFileServer("rootfs.cpio" ${rootfs_file})

# Initialise CAmkES Root Server with addition CPP includes
DeclareCAmkESVMRootServer(supermicro.camkes CPP_INCLUDES configurations)

if(SIMULATION)
    include(simulation)
    GenerateSimulateScript()
endif()
