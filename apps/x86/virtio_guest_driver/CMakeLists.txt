#
# Copyright 2020, DornerWorks
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(virtio_guest_driver)

include(ExternalProject)
include(external-project-helpers)
# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

# Declare VM components
DeclareCAmkESVM(Init0)
DeclareCAmkESVM(Init1)
DeclareCAmkESVM(Init2 EXTRA_SOURCES "src/cross_vm_connections.c")

# Complile CrossVM Dataport Apps
ExternalProject_Add(
    dataport-apps
    URL
    file:///${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-apps/camkes-connector-apps/pkgs/dataport
    BINARY_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/dataport_apps
    INSTALL_COMMAND
    ""
    BUILD_ALWAYS
    ON
    EXCLUDE_FROM_ALL
    CMAKE_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_TOOLCHAIN_FILE=${LINUX_32BIT_TOOLCHAIN}
    -DCMAKE_C_FLAGS="-m32"

)

AddExternalProjFilesToOverlay(
    dataport-apps
    ${CMAKE_CURRENT_BINARY_DIR}/dataport_apps
    overlay_vm2
    "usr/bin"
    FILES
    dataport_read
    dataport_write
    dataport_sata_backend
)

# Get Default Linux VM files
# GetSupermicroLinuxKernelFile(kernel_file)
GetArchDefaultLinuxKernelFile("32" kernel_file)
GetArchDefaultLinuxRootfsFile("32" rootfs_file)

# Add the buildroot overlay to our bare rootfs image
AddOverlayDirToRootfs(
    overlay_vm2
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    output_rootfs_location_vm2
    rootfs_target_vm2
)

# Decompress Linux Kernel image and add to file server
DecompressLinuxKernel(extract_linux_kernel decompressed_kernel ${kernel_file})
AddToFileServer("bzimage" ${decompressed_kernel} DEPENDS extract_linux_kernel)

AddToFileServer(
    "rootfs.cpio"
    "${output_rootfs_location_vm2}"
    DEPENDS
    rootfs_target_vm2
)

DeclareCAmkESVMRootServer(virtio_guest_driver.camkes CPP_INCLUDES configurations)

if(SIMULATION)
    include(simulation)
    GenerateSimulateScript()
endif()
