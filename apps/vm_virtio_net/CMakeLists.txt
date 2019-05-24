#
# Copyright 2019, Data61
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

project(camkes-vm-virtio-net C)

include(../../arm_vm_helpers.cmake)
include(../../../camkes-vm-linux/vm-linux-helpers.cmake)

# Create our CPP Flags based on ARM VM config variables
set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")

AddToFileServer("linux" "${CMAKE_SOURCE_DIR}/projects/camkes-vm-images/exynos5422/linux")
# Generate overlayed rootfs
set(rootfs_file "${CMAKE_SOURCE_DIR}/projects/camkes-vm-images/exynos5422/rootfs.cpio.gz")
AddFileToOverlayDir(
    "interfaces"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/network_interfaces/interfaces
    "etc/network"
    overlay
)

if(VIRTIO_NET_PING)
    AddFileToOverlayDir(
        "S90ping_test"
        ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/ping_test
        "etc/init.d"
        overlay
    )
else()
    AddFileToOverlayDir(
        "S90arping_test"
        ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/arping_test
        "etc/init.d"
        overlay
    )
endif()

AddOverlayDirToRootfs(
    overlay
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    output_overlayed_rootfs_location
    rootfs_target
    GZIP
)
AddToFileServer(
    "linux-initrd"
    ${output_overlayed_rootfs_location}
    DEPENDS
    rootfs_target
)
# Updated dtb based on generated initrd
UpdateDtbFromInitrd(
    "${CMAKE_SOURCE_DIR}/projects/camkes-vm-images/exynos5422/linux-dtb"
    ${output_overlayed_rootfs_location}
    "0x4d700000"
    dtb_gen_target
    output_dtb_location
    DEPENDS
    rootfs_target
)
AddToFileServer(
    "linux-dtb"
    "${output_dtb_location}"
    DEPENDS
    dtb_gen_target
)

AddCamkesCPPFlag(
    cpp_flags
    CONFIG_VARS
    VmEmmc2NoDMA
    VmVUSB
    VmVchan
    Tk1DeviceFwd
    Tk1Insecure
    VmVirtioNetVirtqueue
)

DefineCAmkESVMFileServer()

CAmkESAddImportPath(exynos5422)

# Declare root server
if(VIRTIO_NET_PING)
    DeclareCAmkESComponent(
        PingClient
        SOURCES
        components/PingClient/ping_client.c
        LIBS
        virtqueue
    )

    DeclareCAmkESRootserver(vm_echo_ping.camkes CPP_FLAGS ${cpp_flags})
else()
    DeclareCAmkESRootserver(vm_virtio_net.camkes CPP_FLAGS ${cpp_flags})
endif()
