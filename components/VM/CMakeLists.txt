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

file(GLOB vm_src src/*.c)

list(APPEND vm_src src/modules/map_frame_hack.c)

if(Tk1DeviceFwd)
    list(APPEND vm_src src/modules/plat/tk1/device_fwd.c)
endif()

if(VmVchan)
    list(APPEND vm_src src/modules/vchan.c)
endif()

# A module that is expected to exist for each platform but not required.
# It should provide basic device intialisation required for every vm configuratoin
set(platform_module ${CMAKE_CURRENT_LIST_DIR}/src/modules/plat/${KernelPlatform}/init.c)
if(EXISTS ${platform_module})
    list(APPEND vm_src ${platform_module})
endif()

if(KernelPlatformExynos5410)
    list(APPEND vm_src ${CMAKE_CURRENT_LIST_DIR}/src/modules/plat/exynos5410/init.c)
endif()
# Declare the CAmkES VM component
DeclareCAmkESComponent(VM
    SOURCES ${vm_src}
    INCLUDES include plat_include/${KernelPlatform}
    LIBS sel4allocman elf sel4simple sel4simple-default cpio sel4arm-vmm sel4dma FileServer-client Configuration)

# Append the USB driver library if building for exynos
if("${KernelARMPlatform}" STREQUAL "exynos5410")
    DeclareCAmkESComponent(VM LIBS usbdrivers)
endif()

# Append the VM VChan library is building with VmVchan
if(VmVchan)
    DeclareCAmkESComponent(VM LIBS sel4vchan)
endif()
