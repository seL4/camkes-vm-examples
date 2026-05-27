#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(camkes-vm-cross-connector C)

include(${CAMKES_ARM_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})
include(${CAMKES_VM_LINUX_MODULE_HELPERS_PATH})
include(${CAMKES_VM_LINUX_SOURCE_HELPERS_PATH})
include(${CAMKES_VM_LINUX_MODULE_HELPERS_PATH})
include(ExternalProject)
include(external-project-helpers)

# Create our CPP Flags based on ARM VM config variables
if("${KernelARMPlatform}" STREQUAL "exynos5422")
    set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")
    set(linux_repo "https://github.com/hardkernel/linux.git")
    set(linux_tag "4.14.87-153")
    set(linux_arch "arm")
    set(linux_cross_compile "arm-linux-gnueabi-")
elseif("${KernelARMPlatform}" STREQUAL "stm32mp257f-ev1")
    set(cpp_flags "-DKERNELARMPLATFORM_STM32MP25F-EV1")
    set(linux_arch "arm64")
    list(APPEND cpp_includes "${CAMKES_VM_DIR}/components/VM_Arm/plat_include/stm32mp2")
elseif("${KernelARMPlatform}" STREQUAL "qemu-arm-virt")
    set(cpp_flags "-DKERNELARMPLATFORM_QEMU-ARM-VIRT")
    set(linux_repo "https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git")
    set(linux_tag "v4.9.189")
    set(linux_arch "arm64")
    set(linux_cross_compile "aarch64-linux-gnu-")
    include(simulation)
    set(SIMULATION ON CACHE BOOL "Generate simulation script to run qemu with the proper arguments")
    if(SIMULATION)
        GenerateSimulateScript()
    endif()
endif()

if("${PLATFORM}" STREQUAL "stm32mp2")
    AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/${PLATFORM}/linux")
else()
    AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux")
endif()

if(BUILD_CROSSVM)
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs.cpio.gz")
    # Checkout and configure linux to build crossvm module
    ExternalProject_Add(
        checkout_linux
        GIT_REPOSITORY
        ${linux_repo}
        GIT_TAG
        ${linux_tag}
        GIT_SHALLOW
        1
        GIT_PROGRESS
        1
        BUILD_COMMAND
        ""
        INSTALL_COMMAND
        ""
        CONFIGURE_COMMAND
        ""
        SOURCE_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/linux_out
    )
    # Linux config and symvers are to be copied to unpacked archive
    set(linux_config "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux_configs/config")
    set(linux_symvers "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux_configs/Module.symvers")
    # Configure unpacked archive with config and symvers
    ConfigureLinux(
        ${CMAKE_CURRENT_BINARY_DIR}/linux_out
        ${linux_config}
        ${linux_symvers}
        configure_vm_linux
        ARCH
        ${linux_arch}
        CROSS_COMPILE
        ${linux_cross_compile}
        DEPENDS
        checkout_linux
    )

    # Compile CrossVM Dataport Module
    DefineLinuxModule(
        ${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-modules/camkes-connector-modules/connection
        output_module
        output_module_target
        KERNEL_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/linux_out
        ARCH
        ${linux_arch}
        CROSS_COMPILE
        ${linux_cross_compile}
        DEPENDS
        checkout_linux
        configure_vm_linux
    )
    AddFileToOverlayDir(
        "connection.ko"
        ${output_module}
        "lib/modules/4.14.87/kernel/drivers/vmm"
        overlay
        DEPENDS
        output_module_target
    )

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
    )
    AddExternalProjFilesToOverlay(
        dataport-apps
        ${CMAKE_CURRENT_BINARY_DIR}/dataport_apps
        overlay
        "usr/bin"
        FILES
        dataport_read
        dataport_write
    )

    # Complile CrossVM Event Apps
    ExternalProject_Add(
        event-apps
        URL
        file:///${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-apps/camkes-connector-apps/pkgs/emits_event
        BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/emits_event_apps
        INSTALL_COMMAND
        ""
        BUILD_ALWAYS
        ON
        EXCLUDE_FROM_ALL
        CMAKE_ARGS
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    )
    AddExternalProjFilesToOverlay(
        event-apps
        ${CMAKE_CURRENT_BINARY_DIR}/emits_event_apps
        overlay
        "usr/bin"
        FILES
        emits_event_emit
    )

    # Complile CrossVM Consume Event Apps
    ExternalProject_Add(
        consume-event-apps
        URL
        file:///${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-apps/camkes-connector-apps/pkgs/consumes_event
        BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/consume_event_apps
        INSTALL_COMMAND
        ""
        BUILD_ALWAYS
        ON
        EXCLUDE_FROM_ALL
        CMAKE_ARGS
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    )
    AddExternalProjFilesToOverlay(
        consume-event-apps
        ${CMAKE_CURRENT_BINARY_DIR}/consume_event_apps
        overlay
        "usr/bin"
        FILES
        consumes_event_wait
    )

    # Add script to initialise dataport module
    AddFileToOverlayDir(
        "S90crossvm_module_init"
        ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/cross_vm_module_init
        "etc/init.d"
        overlay
    )
else()
    # User pre-configured rootfs file with crossvm modules and apps installed
    if("${PLATFORM}" STREQUAL "stm32mp2")
        set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${PLATFORM}/rootfs_crossvm.cpio.gz")
    else()
        set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs_crossvm.cpio.gz")
    endif()
endif()

if("${KernelARMPlatform}" STREQUAL "stm32mp257f-ev1")
    set(dts_file "${CAMKES_VM_IMAGES_DIR}/${PLATFORM}/stm32mp257f-ev1-vm.dts")
    add_custom_command(
        OUTPUT linux/linux-dtb
        COMMAND
            dtc -q -I dts -O dtb -o linux/linux-dtb ${dts_file}
        VERBATIM
        DEPENDS ${dts_file}
	)
    # Create custom target for setting the dtb
    add_custom_target(set_dtb DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb")

    # Updated dtb based on initrd
    UpdateDtbFromInitrd(
      "${CMAKE_CURRENT_BINARY_DIR}/linux/linux-dtb"
        ${rootfs_file}
	"0xb4400000"
        dtb_gen_target # dependence rule only
        output_dtb_location
    )

    # will add "${output_dtb_location}/linux-dtb" in file server "fserv"
    AddToFileServer("linux-dtb" "${output_dtb_location}" DEPENDS dtb_gen_target)
    AddToFileServer("linux-initrd" ${rootfs_file})
else()
# Add script to test dataport module
AddFileToOverlayDir(
    "S91crossvm_test"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/cross_vm_test
    "etc/init.d"
    overlay
)

# Construct new rootfs
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
endif()

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

if("${PLATFORM}" STREQUAL "stm32mp2")
    CAmkESAddImportPath(${PLATFORM})
else()
    CAmkESAddImportPath(${KernelARMPlatform})
endif()

# Define our VM Component with out cross vm dataports glue code
DeclareCAmkESComponent(VM SOURCES src/cross_vm_connections.c)

# Define our demo component that shares a dataport with the VM
DeclareCAmkESComponent(CrossvmInit SOURCES components/CrossvmInit/crossvm_init.c)

# Declare root server
DeclareCAmkESRootserver(
    vm_cross_connector.camkes
    CPP_FLAGS
    ${cpp_flags}
    CPP_INCLUDES
    ${CAMKES_VM_DIR}/components/VM_Arm
    ${cpp_includes}
)
