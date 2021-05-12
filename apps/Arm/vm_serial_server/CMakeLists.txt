#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(camkes-vm-virtio-net C)

include(${CAMKES_ARM_VM_HELPERS_PATH})

find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

# Create our CPP Flags based on ARM VM config variables
set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")

AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/exynos5422/linux")
# Generate overlayed rootfs
set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/exynos5422/rootfs.cpio.gz")
AddFileToOverlayDir(
    "inittab"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/inittab_hvc0
    "etc"
    overlay
)

AddOverlayDirToRootfs(
    overlay
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    output_overlayed_rootfs_location
    rootfs_target
    GZIP
)
AddToFileServer("linux-initrd" ${output_overlayed_rootfs_location} DEPENDS rootfs_target)
# Updated dtb based on generated initrd
UpdateDtbFromInitrd(
    "${CAMKES_VM_IMAGES_DIR}/exynos5422/linux-dtb-virtcon"
    ${output_overlayed_rootfs_location}
    "0x4d700000"
    dtb_gen_target
    output_dtb_location
    DEPENDS
    rootfs_target
)
AddToFileServer("linux-dtb" "${output_dtb_location}" DEPENDS dtb_gen_target)

AddCamkesCPPFlag(cpp_flags CONFIG_VARS VmEmmc2NoDMA VmVUSB Tk1DeviceFwd Tk1Insecure)

DefineCAmkESVMFileServer()

CAmkESAddImportPath(exynos5422)

# Declare root server
DeclareCAmkESRootserver(
    vm_serial_server.camkes
    CPP_FLAGS
    ${cpp_flags}
    CPP_INCLUDES
    ${CAMKES_VM_DIR}/components/VM_Arm
)
