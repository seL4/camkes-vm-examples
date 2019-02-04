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

project(camkes-arm-tk1-vm C)

include(../../arm_vm_helpers.cmake)

# Create our CPP Flags based on ARM VM config variables
if ("${KernelARMPlatform}" STREQUAL "tk1")
    set(cpp_flags "-DKERNELARMPLATFORM_TK1")

    # Different device tree if compiling with Tk1Insecure
    if(Tk1Insecure)
        set(device_tree_src "linux-tk1-nonsecured.dts")
    else()
        set(device_tree_src "linux-tk1-secure.dts")
    endif()

    if(VmTk1InitrdRootfs)
        set(linux_binary_name "linux-tk1-initrd")
    else()
        set(linux_binary_name "linux-tk1-debian")
    endif()
    add_custom_command(OUTPUT linux/linux-dtb
        COMMAND bash -c "which dtc && dtc -I dts -O dtb ${CMAKE_CURRENT_LIST_DIR}/../../linux/${device_tree_src} > linux/linux-dtb"
        VERBATIM)
    # Create custom target for setting the dtb
    add_custom_target(set_dtb DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb")

    # Add Linux VM images into file server
    AddToFileServer("linux" "${CMAKE_CURRENT_LIST_DIR}/../../linux/${linux_binary_name}")
    AddToFileServer("linux-dtb" "${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb" DEPENDS set_dtb)

elseif ("${KernelARMPlatform}" STREQUAL "tx1")
    set(cpp_flags "-DKERNELARMPLATFORM_TX1")

    AddToFileServer("linux" "${CMAKE_SOURCE_DIR}/projects/camkes-vm-images/tx1/linux")
    AddToFileServer("linux-dtb" "${CMAKE_SOURCE_DIR}/projects/camkes-vm-images/tx1/linux-dtb")

elseif("${KernelARMPlatform}" STREQUAL "exynos5422")
    set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")

    AddToFileServer("linux" "${CMAKE_SOURCE_DIR}/projects/camkes-vm-images/exynos5422/linux")
    AddToFileServer("linux-dtb" "${CMAKE_SOURCE_DIR}/projects/camkes-vm-images/exynos5422/linux-dtb")

endif()

AddCamkesCPPFlag(cpp_flags
    CONFIG_VARS VmEmmc2NoDMA VmVUSB VmVchan Tk1DeviceFwd Tk1Insecure)

DefineCAmkESVMFileServer()

CAmkESAddImportPath(${KernelARMPlatform})

# Declare root server
DeclareCAmkESRootserver(vm_minimal.camkes CPP_FLAGS ${cpp_flags})
