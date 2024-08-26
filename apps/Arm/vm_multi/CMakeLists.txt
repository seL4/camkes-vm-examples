#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(vm-multi C)

include(${CAMKES_ARM_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

if("${PLATFORM}" STREQUAL "exynos5422")
# Create our CPP Flags based on ARM VM config variables
    set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5422")
elseif("${PLATFORM}" STREQUAL "qemu-arm-virt")
    set(cpp_flags "-DKERNELARMPLATFORM_QEMU-ARM-VIRT")
    set(SIMULATION ON CACHE BOOL "Generate simulation script to run qemu with the proper arguments")
    include(simulation)
    if(SIMULATION)
        GenerateSimulateScript()
    endif()
endif()

# Generate our overlayed rootfs images
set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs.cpio.gz")

# Setup our passthrough vm overlay
#AddFileToOverlayDir(
#    "inittab"
#    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/inittab_hvc0
#    "etc"
#    overlay_vm0
#)
AddFileToOverlayDir(
    "S90bridge_setup"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/vm0_bridge_setup.sh
    "etc/init.d"
    overlay_vm0
)
AddOverlayDirToRootfs(
    overlay_vm0
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    vm0_output_overlayed_rootfs_location
    rootfs_target_vm0
    GZIP
)

# Setup our client vm overlays
#AddFileToOverlayDir(
#    "inittab"
#    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/inittab_hvc0
#    "etc"
#    overlay_client_vm
#)


# Add startup dropbear script to client_vm
AddFileToOverlayDir(
    "authorized_keys"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/authorized_keys
    "etc/dropbear"
    overlay_client_vm
)
AddFileToOverlayDir(
    "id_rsa.pub"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/id_rsa.pub
    "home/root/.ssh"
    overlay_client_vm
)
AddFileToOverlayDir(
    "id_rsa"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/id_rsa
    "home/root/.ssh"
    overlay_client_vm
)
AddFileToOverlayDir(
    "dropbear_rsa_host_key.pub"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/dropbear_rsa_host_key.pub
    "etc/dropbear"
    overlay_client_vm
)
AddFileToOverlayDir(
    "dropbear_rsa_host_key"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/dropbear_rsa_host_key
    "etc/dropbear"
    overlay_client_vm
)
AddFileToOverlayDir(
    "StartDropbear"
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/StartDropbear
    "etc/init.d"
    overlay_client_vm
)

# Add startup dropbear script to vm0
AddFileToOverlayDir(
    "authorized_keys"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/authorized_keys
    "etc/dropbear"
    overlay_vm0
)
AddFileToOverlayDir(
    "id_rsa.pub"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/id_rsa.pub
    "home/root/.ssh"
    overlay_vm0
)
AddFileToOverlayDir(
    "id_rsa"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/id_rsa
    "home/root/.ssh"
    overlay_vm0
)
AddFileToOverlayDir(
    "dropbear_rsa_host_key.pub"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/dropbear_rsa_host_key.pub
    "etc/dropbear"
    overlay_vm0
)
AddFileToOverlayDir(
    "dropbear_rsa_host_key"
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbear/dropbear_rsa_host_key
    "etc/dropbear"
    overlay_vm0
)
AddFileToOverlayDir(
    "StartDropbear"
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/StartDropbear
    "etc/init.d"
    overlay_vm0
)



AddFileToOverlayDir(
    ".profile"
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/.profile
    "root"
    overlay_client_vm
)

AddFileToOverlayDir(
    ".profile"
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/.profile
    "root"
    overlay_vm0
)

# AddFileToOverlayDir(
#     "hosts"
#     ${CMAKE_CURRENT_SOURCE_DIR}/scripts/hosts
#     "etc"
#     overlay_client_vm
# )
# 
# AddFileToOverlayDir(
#     "hosts"
#     ${CMAKE_CURRENT_SOURCE_DIR}/scripts/hosts
#     "etc"
#     overlay_vm0
# )



AddFileToOverlayDir(
    "wireguard-interface.sh"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/wireguard-interface.sh
    "root"
    overlay_vm0
)
AddFileToOverlayDir(
    "generate-client-vlan"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/generate-client-vlan
    "root"
    overlay_vm0
)
AddFileToOverlayDir(
    "silent"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/vnfs/silent
    "root"
    overlay_vm0
)
AddFileToOverlayDir(
    "build-router.sh"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/build-router.sh
    "root"
    overlay_vm0
)
AddFileToOverlayDir(
    "connect-vlans"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/connect-vlans
    "root"
    overlay_vm0
)
AddFileToOverlayDir(
    "remote-etc-hosts"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/remote-etc-hosts
    "root"
    overlay_vm0
)
AddFileToOverlayDir(
    "setup"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/orchestration/setup
    "root"
    overlay_vm0
)
AddFileToOverlayDir(
    "config.yml"
    ${CMAKE_CURRENT_SOURCE_DIR}/network/config.yml
    "root"
    overlay_vm0
)



AddFileToOverlayDir(
    "privkey"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/wg-privkey
    "etc/wireguard"
    overlay_vm0
)

AddFileToOverlayDir(
    "S90client_net_vm"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/vm_client_net_setup.sh
    "etc/init.d"
    overlay_client_vm
)
AddOverlayDirToRootfs(
    overlay_client_vm
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    client_output_overlayed_rootfs_location
    rootfs_target_client_vm
    GZIP
)

AddToFileServer(
    "linux-initrd-vm0"
    "${vm0_output_overlayed_rootfs_location}"
    DEPENDS
    rootfs_target_vm0
)
AddToFileServer(
    "linux-initrd-vm-client"
    "${client_output_overlayed_rootfs_location}"
    DEPENDS
    rootfs_target_client_vm
)

AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux")

AddCamkesCPPFlag(cpp_flags CONFIG_VARS VmEmmc2NoDMA VmVUSB VmVchan Tk1DeviceFwd Tk1Insecure)

DefineCAmkESVMFileServer()

CAmkESAddImportPath(${KernelARMPlatform})

# Declare root server
DeclareCAmkESRootserver(
    vm_multi.camkes
    CPP_FLAGS
    ${cpp_flags}
    CPP_INCLUDES
    ${CAMKES_VM_DIR}/components/VM_Arm
)
