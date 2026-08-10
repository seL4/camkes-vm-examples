#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(supermicro_wg)

include(ExternalProject)
# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

if("${PLATFORM}" STREQUAL "qemu")
    set(APP_CAMKES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/qemu/passthrough.camkes)
elseif("${PLATFORM}" STREQUAL "e300-9d")
    set(APP_CAMKES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/e300-9d/passthrough.camkes)
else()
    message(FATAL_ERROR "Unsupported PLATFORM: ${PLATFORM}")
endif()

# Number of vms.
set(NumberVMs 3)

# CMake foreach RANGE loops are 0..num inclusive....
MATH(EXPR NumberVMsLess1 "${NumberVMs}-1")

# Declare VM component: Init
foreach(i RANGE ${NumberVMsLess1})
    DeclareCAmkESVM("Init${i}")
endforeach()

DeclareCamkesComponent(virtqueueinit)

# Get Linux VM files
set(kernel_file "${CMAKE_CURRENT_SOURCE_DIR}/images/bzImage")
set(rootfs_file "${CMAKE_CURRENT_SOURCE_DIR}/images/rootfs.cpio")
# If Linux files are compiled externally, use these template instead
#set(kernel_file "/host/linux-vm/linux-stable/arch/x86/boot/bzImage")
#set(rootfs_file "/host/linux-vm/buildroot/output/images/rootfs.cpio")

foreach(i RANGE ${NumberVMsLess1})
    if("${WG_DEMO}" STREQUAL "A" OR "${WG_DEMO}" STREQUAL "B")
        if(${i} EQUAL 1)
        message(STATUS "WG_DEMO " ${WG_DEMO})
            AddFileToOverlayDir(
                "wg0_${WG_DEMO}.conf"
                ${CMAKE_CURRENT_SOURCE_DIR}/scripts/wg_demo/configs_${WG_DEMO}/wg0_${WG_DEMO}.conf
                "etc/wireguard"
                overlay_vm${i}
            )
            AddFileToOverlayDir(
                "wg0_dockerdemo.conf"
                ${CMAKE_CURRENT_SOURCE_DIR}/scripts/wg_demo/wg0_dockerdemo.conf
                "etc/wireguard"
                overlay_vm${i}
            )
        endif()
        AddFileToOverlayDir(
            "interfaces"
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/wg_demo/configs_${WG_DEMO}/network_interface_${WG_DEMO}${i}
            "etc/network"
            overlay_vm${i}
        )
        AddFileToOverlayDir(
            "S91wg_demo"
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/wg_demo/configs_${WG_DEMO}/wg_demo_${WG_DEMO}${i}.sh
            "etc/init.d"
            overlay_vm${i}
        )
    else()
        AddFileToOverlayDir(
            "interfaces"
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/default_interfaces/network_interface_${i}
            "etc/network"
            overlay_vm${i}
        )
    endif()
    AddFileToOverlayDir(
        "rcS"
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/rcS.sh
        "etc/init.d"
        overlay_vm${i}
    )
endforeach()

foreach(i RANGE ${NumberVMsLess1})
    # Add the buildroot overlay to our bare rootfs image
    AddOverlayDirToRootfs(
        overlay_vm${i}
        ${rootfs_file}
        "buildroot"
        "rootfs_install"
        output_rootfs_location_vm${i}
        rootfs_target_vm${i}
    )
    AddToFileServer(
        "vm${i}_rootfs.cpio"
        "${output_rootfs_location_vm${i}}"
        DEPENDS
        rootfs_target_vm${i}
    )
endforeach()

# Decompress Linux Kernel image and add to file server
DecompressLinuxKernel(extract_linux_kernel decompressed_kernel ${kernel_file})
AddToFileServer("bzimage" ${decompressed_kernel} DEPENDS extract_linux_kernel)

# Initialise CAmkES Root Server with addition CPP includes
DeclareCAmkESVMRootServer(${APP_CAMKES_FILE})

