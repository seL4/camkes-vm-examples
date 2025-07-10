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

set(cpp_includes "${CAMKES_VM_DIR}/components/VM_Arm")

# Set defaults, platform may overwrite this belwo
set(dtb_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux-dtb")
set(linux_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux")
set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs.cpio.gz")

# For some platforms, the DTB must contains the rootfs location and size.
set(rootfs_address "")
set(dtb_file_patched "${dtb_file}")

if("${KernelARMPlatform}" STREQUAL "exynos5422")
    # Create our CPP Flags based on ARM VM config variables
    set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")
    set(rootfs_address "0x4d700000")

elseif("${KernelARMPlatform}" STREQUAL "tx2")
    set(cpp_flags "-DKERNELARMPLATFORM_TX2")
    set(rootfs_address "0xf7000000")
    set(dtb_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux-pci-dtb")
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs_crossvm.cpio.gz")

elseif("${KernelARMPlatform}" STREQUAL "zcu102")

    if(NOT VmZynqmpPetalinux2022_1)
        message(FATAL_ERROR "Petalinux 2022_1 is the only supported version for this application")
    endif()

    set(linux_file "${CAMKES_VM_IMAGES_DIR}/zynqmp/${VmZynqmpPetalinuxVersion}/linux")
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

# Generate overlayed rootfs
AddFileToOverlayDir(
    "interfaces"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/network_interfaces/interfaces
    "etc/network"
    overlay
)

if(VIRTIO_NET_PING)
    if("${KernelARMPlatform}" STREQUAL "zcu102")
        set(name "ping_test")
        set(file "${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/zynqmp/ping_test")
    else()
        set(file "${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/ping_test")
        set(name "S90ping_test")
    endif()

    AddFileToOverlayDir(${name} ${file} "etc/init.d" overlay)

    # The ZCU102 Filesystem expects init scripts to be setup like this, else they won't run
    if("${KernelARMPlatform}" STREQUAL "zcu102")
        AddLinkToOverlayDir("K20ping_test" "../init.d/ping_test" "etc/rc0.d/" overlay)
        AddLinkToOverlayDir("K20ping_test" "../init.d/ping_test" "etc/rc1.d/" overlay)
        AddLinkToOverlayDir("K20ping_test" "../init.d/ping_test" "etc/rc6.d/" overlay)
        AddLinkToOverlayDir("S20ping_test" "../init.d/ping_test" "etc/rc2.d/" overlay)
        AddLinkToOverlayDir("S20ping_test" "../init.d/ping_test" "etc/rc3.d/" overlay)
        AddLinkToOverlayDir("S20ping_test" "../init.d/ping_test" "etc/rc4.d/" overlay)
        AddLinkToOverlayDir("S20ping_test" "../init.d/ping_test" "etc/rc5.d/" overlay)
    endif()
else()
    if("${KernelARMPlatform}" STREQUAL "zcu102")
        set(name "arping_test")
        set(file "${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/zynqmp/arping_test")
    else()
        set(file "${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/arping_test")
        set(name "S90arping_test")
    endif()

    AddFileToOverlayDir(${name} ${file} "etc/init.d" overlay)

    # The ZCU102 Filesystem expects init scripts to be setup like this, else they won't run
    if("${KernelARMPlatform}" STREQUAL "zcu102")
        AddLinkToOverlayDir("K20arping_test" "../init.d/arping_test" "etc/rc0.d/" overlay)
        AddLinkToOverlayDir("K20arping_test" "../init.d/arping_test" "etc/rc1.d/" overlay)
        AddLinkToOverlayDir("K20arping_test" "../init.d/arping_test" "etc/rc6.d/" overlay)
        AddLinkToOverlayDir("S20arping_test" "../init.d/arping_test" "etc/rc2.d/" overlay)
        AddLinkToOverlayDir("S20arping_test" "../init.d/arping_test" "etc/rc3.d/" overlay)
        AddLinkToOverlayDir("S20arping_test" "../init.d/arping_test" "etc/rc4.d/" overlay)
        AddLinkToOverlayDir("S20arping_test" "../init.d/arping_test" "etc/rc5.d/" overlay)
    endif()
endif()

AddOverlayDirToRootfs(
    overlay
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    output_overlayed_rootfs_location
    target_gen_rootfs # helper CMake target to be created, not used
    GZIP
)

set(cpio_files "linux:${linux_file}" "linux-initrd:${output_overlayed_rootfs_location}")
if(rootfs_address)
    # Updated dtb based on generated initrd. There is no need to explicitly
    # depend on 'target_gen_rootfs' here, there is an implicit dependency on
    # the file that 'output_overlayed_rootfs_location' refers to.
    UpdateDtbFromInitrd(
        ${dtb_file}
        ${output_overlayed_rootfs_location}
        ${rootfs_address}
        target_gen_dtb # helper CMake target to be created, not used
        dtb_file_patched # output, overwrite default
    )
    list(APPEND cpio_files "linux-dtb:${dtb_file_patched}")
else()
    list(APPEND cpio_files "linux-dtb:${dtb_file}")
endif()

AddCamkesCPPFlag(
    cpp_flags
    CONFIG_VARS
    VmEmmc2NoDMA
    VmVUSB
    Tk1DeviceFwd
    Tk1Insecure
    VmVirtioNetVirtqueue
)

# Create file server with VM images. There is an implicit dependency on the
# input files, so the rules above will be invoked to generate/update the files.
# There is no need to specify an explicit dependency on the helper targets
# 'target_gen_rootfs' or 'target_gen_dtb'
DefineCAmkESVMFileServer(FILES ${cpio_files})

CAmkESAddImportPath(${KernelARMPlatform})

# Declare root server
if(VIRTIO_NET_PING)
    DeclareCAmkESComponent(
        PingClient
        SOURCES
        components/PingClient/ping_client.c
        LIBS
        virtqueue
        sel4_autoconf
    )

    DeclareCAmkESRootserver(vm_echo_ping.camkes CPP_FLAGS ${cpp_flags} CPP_INCLUDES ${cpp_includes})
else()
    DeclareCAmkESRootserver(
        vm_virtio_net.camkes
        CPP_FLAGS
        ${cpp_flags}
        CPP_INCLUDES
        ${cpp_includes}
    )
endif()
