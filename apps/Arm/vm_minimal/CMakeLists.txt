#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(camkes-arm-tk1-vm C)

include(${CAMKES_ARM_VM_HELPERS_PATH})

set(cpp_includes "${CAMKES_VM_DIR}/components/VM_Arm")

# On some platforms, there is no dedicated rootfs, because it's part of the
# Linux kernel image.
set(rootfs_file "")

# Create our CPP Flags based on ARM VM config variables
if("${KernelARMPlatform}" STREQUAL "tk1")
    set(cpp_flags "-DKERNELARMPLATFORM_TK1")

    # Different device tree if compiling with Tk1Insecure
    if(Tk1Insecure)
        set(device_tree_src "${CAMKES_ARM_LINUX_DIR}/linux-tk1-nonsecured.dts")
    else()
        set(device_tree_src "${CAMKES_ARM_LINUX_DIR}/linux-tk1-secure.dts")
    endif()

    # kernel image contains rootfs
    if(VmTk1InitrdRootfs)
        set(linux_image "${CAMKES_ARM_LINUX_DIR}/linux-tk1-initrd")
    else()
        set(linux_image "${CAMKES_ARM_LINUX_DIR}/linux-tk1-debian")
    endif()

    set(dtb_file "${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb")
    add_custom_command(
        OUTPUT "${dtb_file}"
        COMMAND
            dtc -I dts -O dtb -o ${dtb_file} ${device_tree_src}
        VERBATIM
        DEPENDS ${device_tree_src}
    )
    # There is no need to create an explicit target there, any step that uses
    # 'dtb_file' and lists it properly as dependency will make CMake run the
    # command above to create (or update) it.

elseif("${KernelARMPlatform}" STREQUAL "tx1")
    set(cpp_flags "-DKERNELARMPLATFORM_TX1")

    # kernel image contains rootfs
    set(linux_image "${CAMKES_VM_IMAGES_DIR}/tx1/linux")
    set(dtb_file "${CAMKES_VM_IMAGES_DIR}/tx1/linux-dtb")

elseif("${KernelARMPlatform}" STREQUAL "exynos5422")
    find_package(camkes-vm-linux REQUIRED)
    include(${CAMKES_VM_LINUX_HELPERS_PATH})
    set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")

    set(linux_image "${CAMKES_VM_IMAGES_DIR}/exynos5422/linux")
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/exynos5422/rootfs.cpio.gz")

    # Updated dtb based on initrd
    UpdateDtbFromInitrd(
        "${CAMKES_VM_IMAGES_DIR}/exynos5422/linux-dtb"
        ${rootfs_file}
        "0x4d700000"
        target_gen_dtb # helper CMake target to be created, not used
        dtb_file
    )

elseif("${KernelARMPlatform}" STREQUAL "tx2")
    find_package(camkes-vm-linux REQUIRED)
    include(${CAMKES_VM_LINUX_HELPERS_PATH})
    set(cpp_flags "-DKERNELARMPLATFORM_TX2")

    set(linux_image "${CAMKES_VM_IMAGES_DIR}/tx2/linux")
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/tx2/rootfs.cpio.gz")

    # Updated dtb based on initrd
    UpdateDtbFromInitrd(
        "${CAMKES_VM_IMAGES_DIR}/tx2/linux-dtb"
        ${rootfs_file}
        "0xf7000000"
        target_gen_dtb # helper CMake target to be created, not used
        dtb_file
    )

elseif("${KernelARMPlatform}" STREQUAL "qemu-arm-virt")

    # QEMU supports ARMv7/aarch32 and ARMv8/aarch32, but currently there are
    # Linux images for aarch64 only. However, it is unclear if that is the only
    # blocker to get this example run in QEMU on aarch32.
    if(NOT KernelSel4ArchAarch64)
        message(FATAL_ERROR "Only AARCH64 is supported")
    endif()

    find_package(camkes-vm-linux REQUIRED)
    include(${CAMKES_VM_LINUX_HELPERS_PATH})
    set(cpp_flags "-DKERNELARMPLATFORM_QEMU-ARM-VIRT")

    set(linux_image "${CAMKES_VM_IMAGES_DIR}/qemu-arm-virt/linux")
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/qemu-arm-virt/rootfs.cpio.gz")

    # Updated dtb based on initrd
    UpdateDtbFromInitrd(
        "${CAMKES_VM_IMAGES_DIR}/qemu-arm-virt/linux-dtb"
        ${rootfs_file}
        "0x4d700000"
        target_gen_dtb # helper CMake target to be created, not used
        dtb_file
    )

    include(simulation)
    set(SIMULATION ON CACHE BOOL "Generate simulation script to run qemu with the proper arguments")
    if(SIMULATION)
        GenerateSimulateScript()
    endif()

elseif("${KernelARMPlatform}" STREQUAL "odroidc2")
    find_package(camkes-vm-linux REQUIRED)
    include(${CAMKES_VM_LINUX_HELPERS_PATH})
    set(cpp_flags "-DKERNELARMPLATFORM_ODROIDC2")

    set(
        CAMKES_ROOT_DTB_FILE_PATH "${CAMKES_VM_IMAGES_DIR}/odroidc2/camkes-linux-dtb"
        CACHE STRING ""
    )
    set(linux_image "${CAMKES_VM_IMAGES_DIR}/odroidc2/linux")
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/odroidc2/rootfs.cpio.gz")

    # Updated dtb based on initrd
    UpdateDtbFromInitrd(
        "${CAMKES_VM_IMAGES_DIR}/odroidc2/linux-dtb"
        ${rootfs_file}
        "0x2d700000"
        target_gen_dtb # helper CMake target to be created, not used
        dtb_file
    )

elseif("${KernelARMPlatform}" STREQUAL "zcu102")

    if("${VmZynqmpPetalinuxVersion}" STREQUAL "")
        message(
            FATAL_ERROR
                "VmZynqmpPetalinuxVersion is not set. Please set to include proper VM artifacts"
        )
    endif()

    set(linux_image "${CAMKES_VM_IMAGES_DIR}/zynqmp/${VmZynqmpPetalinuxVersion}/linux")
    set(
        rootfs_file
        "${CAMKES_VM_IMAGES_DIR}/zynqmp/${VmZynqmpPetalinuxVersion}/rootfs-minimal.cpio.gz"
    )

    set(dts_file "${CAMKES_VM_IMAGES_DIR}/zynqmp/${VmZynqmpPetalinuxVersion}/linux.dts")
    set(dtb_file "${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb")
    add_custom_command(
        OUTPUT ${dtb_file}
        COMMAND
            dtc -q -I dts -O dtb -o ${dtb_file} ${dts_file}
        VERBATIM
        DEPENDS ${dts_file}
    )
    # There is no need to create an explicit target there, any step that uses
    # 'dtb_file' and lists it properly as dependency will make CMake run the
    # command above to create (or update) it.

    list(APPEND cpp_includes "${CAMKES_VM_DIR}/components/VM_Arm/plat_include/zynqmp")

    CAmkESAddImportPath(${KernelARMPlatform}/${VmZynqmpPetalinuxVersion})
endif()

AddCamkesCPPFlag(cpp_flags CONFIG_VARS VmEmmc2NoDMA VmVUSB Tk1DeviceFwd Tk1Insecure)

# Create file server with VM images. There is an implicit dependency on the
# input files, so the rules above will be invoked to generate/update the files.
# There is no need to specify an explicit dependency on the helper target
# 'target_gen_dtb'
set(cpio_files "linux:${linux_image}" "linux-dtb:${dtb_file}")
if(rootfs_file)
    list(APPEND cpio_files "linux-initrd:${rootfs_file}")
endif()

DefineCAmkESVMFileServer(FILES ${cpio_files})

CAmkESAddImportPath(${KernelARMPlatform})

# Declare root server
DeclareCAmkESRootserver(vm_minimal.camkes CPP_FLAGS ${cpp_flags} CPP_INCLUDES ${cpp_includes})
