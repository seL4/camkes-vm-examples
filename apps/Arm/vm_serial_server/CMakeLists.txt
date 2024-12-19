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
    rootfs_target # helper CMake target to be created, not used
    GZIP
)

# Updated dtb based on generated initrd
UpdateDtbFromInitrd(
    "${CAMKES_VM_IMAGES_DIR}/exynos5422/linux-dtb-virtcon"
    ${output_overlayed_rootfs_location}
    "0x4d700000"
    dtb_gen_target # helper CMake target to be created, not used
    output_dtb_location
)

AddCamkesCPPFlag(cpp_flags CONFIG_VARS VmEmmc2NoDMA VmVUSB Tk1DeviceFwd Tk1Insecure)

# Create file server with VM images. There is an implicit dependency on the
# input files, so the rules above will be invoked to generate/update the files.
# There is no need to specify an explicit dependency on the helper targets
# 'rootfs_target' and 'dtb_gen_target'
DefineCAmkESVMFileServer(
    FILES
    "linux:${CAMKES_VM_IMAGES_DIR}/exynos5422/linux"
    "linux-initrd:${output_overlayed_rootfs_location}"
    "linux-dtb:${output_dtb_location}"
)

CAmkESAddImportPath(exynos5422)

# Declare root server
DeclareCAmkESRootserver(
    vm_serial_server.camkes
    CPP_FLAGS
    ${cpp_flags}
    CPP_INCLUDES
    ${CAMKES_VM_DIR}/components/VM_Arm
)
