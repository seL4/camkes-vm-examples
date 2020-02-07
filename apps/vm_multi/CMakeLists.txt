#
# Copyright 2020, Data61
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

project(vm-multi C)

include(../../arm_vm_helpers.cmake)
include(../../../camkes-vm-linux/vm-linux-helpers.cmake)

# Create our CPP Flags based on ARM VM config variables
set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")

AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/exynos5422/linux")

# Generate our overlayed rootfs images
set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/exynos5422/rootfs.cpio.gz")

# Setup our passthrough vm overlay
AddFileToOverlayDir(
    "inittab"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/inittab_hvc0
    "etc"
    overlay_vm0
)
AddFileToOverlayDir(
    "S90bridge_setup"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/vm0_bridge_setup.sh
    "etc/init.d"
    overlay_vm0
)
AddOverlayDirToRootfs(
    overlay_vm0
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    vm0_output_overlayed_rootfs_location
    rootfs_target_vm0
    GZIP
)

# Setup our client vm overlays
AddFileToOverlayDir(
    "inittab"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/inittab_hvc0
    "etc"
    overlay_client_vm
)
AddFileToOverlayDir(
    "S90client_net_vm"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/vm_client_net_setup.sh
    "etc/init.d"
    overlay_client_vm
)
AddOverlayDirToRootfs(
    overlay_client_vm
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    client_output_overlayed_rootfs_location
    rootfs_target_client_vm
    GZIP
)

AddToFileServer(
    "linux-initrd-vm0"
    "${vm0_output_overlayed_rootfs_location}"
    DEPENDS
    rootfs_target_vm0
)
AddToFileServer(
    "linux-initrd-vm-client"
    "${client_output_overlayed_rootfs_location}"
    DEPENDS
    rootfs_target_client_vm
)

AddCamkesCPPFlag(cpp_flags CONFIG_VARS VmEmmc2NoDMA VmVUSB VmVchan Tk1DeviceFwd Tk1Insecure)

DefineCAmkESVMFileServer()

CAmkESAddImportPath(${KernelARMPlatform})

# Declare root server
DeclareCAmkESRootserver(
    vm_multi.camkes
    CPP_FLAGS
    ${cpp_flags}
    CPP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/../../components/VM
)
