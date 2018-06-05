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

project(VM C)

set(configure_string "")

# ARM VM Configuration Options
config_option(VmEmmc2NoDMA VM_EMMC2_NODMA
    "SD access control to prevent DMA \
    Installs SD card into the VM with read only access. Write access is \
    trapped and emulated to ensure that DMA is not used."
    DEFAULT OFF)

config_option(VmVUSB VM_VUSB
    "Para-virtual USB driver \
    Selects the device tree which enables the para virtual USB driver and \
    installs the device into the VM."
    DEPENDS "KernelPlatformExynos5410"
    DEFAULT OFF)

config_option(VmVchan VM_VCHAN
    "Enable vchan connection"
    DEPENDS "KernelPlatformExynos5410"
    DEFAULT OFF)

config_choice(VmRootfs VM_ROOTFS
    "Root file system selection \
    Selects the root device and partition which should be used for the root filesystem. \
    mmcblk0p2 -> Selects partition 2 of the on board eMMC as the root filesystem. \
    mmcblk1p2 -> Selects partition 2 of the on removable SD card as the root filesystem."
    "mmcblk0p2;VmRootfsMmcblk0p2;VM_ROOTFS_MMCBLK0P2"
    "mmcblk1p2;VmRootfsMmcblk1p2;VM_ROOTFS_MMCBLK1P2")

config_choice(VmTk1Rootfs VM_TK1_ROOTFS
    "TK1 boot mode selection. Selects whether to load root file system off emmc or ramdisk \
    initrd -> Selects initrd as the root filesystem. \
    mmcblk0p2 -> Selects partition 2 of the emmc card as the root filesystem."
    "initrd;VmTk1InitrdRootfs;VM_TK1_INITRD_ROOTFS"
    "mmcblk0p2;VmTk1EmmcRootfs;VM_TK1_EMMC_ROOTFS")

config_option(Tk1DeviceFwd TK1_DEVICE_FWD
    "Forward VM access to UART and Clock and reset controller. \
    When using the VM with other components that need access to the UART and clock and reset controller \
    devices, this option will cause the vm to forward the faults over a RPC connection"
    DEPENDS "KernelPlatformTK1"
    DEFAULT OFF)

config_option(Tk1Insecure TK1_INSECURE
    "Insecure: Give all devices to VM component \
    The option when disabled only passes through UARTD, USB, EMMC, VGIC and a Clock and reset controller"
    DEPENDS "KernelPlatformTK1"
    DEFAULT OFF)

add_config_library(arm_vm "${configure_string}")

set(linux_binary_name "linux")
set(device_tree_src "")

if("${KernelARMPlatform}" STREQUAL "exynos5410")
    # Building exynos5410
    # Different device tree if compiling with VmVUSB
    if(VmVUSB)
        set(device_tree_src "linux-secure-vusb-dtb")
    else()
        set(device_tree_src "linux-secure-dtb")
    endif()
    # Sed the device tree updating the rootfs name
    add_custom_command(OUTPUT linux/linux-dtb
        COMMAND bash -c "sed \"s/root=\\/dev\\/mmcblk1p2/root=\\/dev\\/${VmRootfs}/g\" ${CMAKE_CURRENT_LIST_DIR}/../../linux/${device_tree_src} > linux/linux-dtb"
        VERBATIM)
    # Create custom target for setting the dtb
    add_custom_target(set_dtb DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb")
elseif("${KernelARMPlatform}" STREQUAL "tk1")
    # Building for TK1
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
else()
    message(FATAL_ERROR "Using an unsupported platform - ${KernelARMPlatform}")
endif()

# Custom command/target to copy the appropriate linux binary to the cmake build directory
add_custom_command(OUTPUT linux/linux
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/../../linux/${linux_binary_name}" "${CMAKE_CURRENT_BINARY_DIR}/linux/linux"
    VERBATIM)
add_custom_target(copy_linux DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/linux/linux")

# Create the CPIO archive with the necessary linux binary and dtb
MakeCPIO(archive.o "${CMAKE_CURRENT_BINARY_DIR}/linux/linux;${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb" DEPENDS copy_linux set_dtb)
add_library(archive_cpio STATIC EXCLUDE_FROM_ALL archive.o)
set_property(TARGET archive_cpio PROPERTY LINKER_LANGUAGE C)

file(GLOB vm_src src/*.c)

# Declare the CAmkES VM component
DeclareCAmkESComponent(VM
    SOURCES ${vm_src}
    INCLUDES src
    LIBS sel4allocman elf sel4simple sel4simple-default cpio sel4arm-vmm sel4dma Configuration archive_cpio)

# Append the USB driver library if building for exynos
if("${KernelARMPlatform}" STREQUAL "exynos5410")
    DeclareCAmkESComponent(VM LIBS usbdrivers)
endif()

# Append the VM VChan library is building with VmVchan
if(VmVchan)
    DeclareCAmkESComponent(VM LIBS sel4vchan)
endif()
