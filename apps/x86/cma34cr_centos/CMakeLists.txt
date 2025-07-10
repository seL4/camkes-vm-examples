#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(cma34cr_centos)

# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})

# Retrieve Init0 extra src files
file(GLOB init0_extra src/*.c)
# Declare VM component: Init0
DeclareCAmkESVM(Init0 EXTRA_SOURCES ${init0_extra} EXTRA_LIBS ethdrivers)

# Declare C162 Kernel
DecompressLinuxKernel(
    decompress_linux # helper CMake target to be created, not used
    decompressed_kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/centos_linux/bzimage
)

# Provided by find_package(util_libs)
include(${LWIP_HELPERS})
# Declare lwipopts.h include
AddLWIPConfiguration(${CMAKE_CURRENT_SOURCE_DIR}/lwip_include)

# Create file server with VM images. There is an implicit dependency on the
# input files, so the rules above will be invoked to generate/update the files.
# There is no need to specify an explicit dependency on the helper target
# 'decompress_linux'.
DefineCAmkESVMFileServer(
    FILES "bzimage:${decompressed_kernel}"
    "rootfs.cpio:${CMAKE_CURRENT_SOURCE_DIR}/centos_linux/rootfs.cpio"
)

# Declare CAmkES Root Server
DeclareCAmkESRootserver(
    "cma34cr_centos.camkes"
    CPP_INCLUDES
    ${CAMKES_VM_DIR}/components/StringReverse/include
    ${CAMKES_VM_DIR}/components/Ethdriver/include
    configurations
)
