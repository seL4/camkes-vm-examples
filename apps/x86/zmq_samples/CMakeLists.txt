#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(zmq_samples)

include(ExternalProject)
# Include CAmkES VM helper functions
include(${CAMKES_VM_HELPERS_PATH})
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})

if(CAKEML_FILTER)
    set(zmq_camkes "zmq_cakeml.camkes")
    include("libs/cakeml_helpers.cmake")

    # Number of vms.
    set(NumberVMs 2)

    CakeMLPP(components/cakeml-filter libs/virtqueueScript.sml)

    DeclareCAmkESComponent(
        CakeMLFilter
        CAKEML_SOURCES
        components/cakeml-filter/componentScript.sml
        ${CMAKE_CURRENT_BINARY_DIR}/components/cakeml-filter/virtqueueScript.sml
        CAKEML_INCLUDES
        ${CAKEMLDIR}/semantics/alt_semantics
        SOURCES
        components/cakeml-filter/global_endpoint.c
        LIBS
        vswitch
        CAKEML_HEAP_SIZE
        50
    )
else()
    set(zmq_camkes "zmq_samples.camkes")
    # Number of vms.
    set(NumberVMs 3)
endif()

# CMake foreach RANGE loops are 0..num inclusive....
MATH(EXPR NumberVMsLess1 "${NumberVMs}-1")

# Declare VM component: Init
foreach(i RANGE ${NumberVMsLess1})
    DeclareCAmkESVM("Init${i}")
endforeach()

DeclareCamkesComponent(virtqueueinit)

# Get 32-bit Linux VM files
GetArchDefaultLinuxKernelFile("32" kernel_file)
GetArchDefaultLinuxRootfsFile("32" rootfs_file)

foreach(i RANGE ${NumberVMsLess1})
    AddFileToOverlayDir(
        "interfaces"
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/network_interface_${i}
        "etc/network"
        overlay_vm${i}
    )
endforeach()

set(SYSROOT "${CMAKE_CURRENT_BINARY_DIR}/sysroot")
include(cross_compiling)
FindCustomPollyToolchain(LINUX_32BIT_TOOLCHAIN "linux-gcc-32bit-pic")

ExternalProject_Add(
    zmq_external
    SOURCE_DIR
    ${LIBZMQ_PATH}
    BINARY_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/zmq_external
    BUILD_ALWAYS
    ON
    EXCLUDE_FROM_ALL
    CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${LINUX_32BIT_TOOLCHAIN}
    -DCMAKE_INSTALL_PREFIX=${SYSROOT}
    -DPOLLER=epoll
    -DWITH_PERF_TOOL=ON
    -DBUILD_TESTS=OFF
)

ExternalProject_Add(
    demoapps_external
    SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/src/user
    BINARY_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/src/user
    DEPENDS
    zmq_external
    BUILD_ALWAYS
    ON
    EXCLUDE_FROM_ALL
    CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${LINUX_32BIT_TOOLCHAIN}
    -DCMAKE_INSTALL_PREFIX=${SYSROOT}
)

include(external-project-helpers)
DeclareExternalProjObjectFiles(
    demoapps_external
    ${SYSROOT}/bin
    FILES
    client
    server
    sink
    source
    worker
    pub
    sub
)
macro(AddZMQBinary binary_name)
    foreach(i RANGE ${NumberVMsLess1})
        AddFileToOverlayDir(
            "${binary_name}"
            ${CMAKE_CURRENT_BINARY_DIR}/sysroot/bin/${binary_name}
            "bin/"
            overlay_vm${i}
        )
    endforeach()
endmacro()

AddZMQBinary(client)
AddZMQBinary(server)
AddZMQBinary(sink)
AddZMQBinary(source)
AddZMQBinary(worker)
AddZMQBinary(pub)
AddZMQBinary(sub)

set(rootfs_images "")

foreach(i RANGE ${NumberVMsLess1})
    set(overlay_target "overlay_vm${i}")
    set(rootfs_target "rootfs_target_vm${i}")
    if(NOT CAKEML_FILTER)
        AddFileToOverlayDir(
            "S91zmq_bench"
            ${CMAKE_CURRENT_SOURCE_DIR}/src/user/zmq_bench_${i}.sh
            "etc/init.d"
            "${overlay_target}"
        )
    endif()
    # Add the buildroot overlay to our bare rootfs image
    AddOverlayDirToRootfs(
        "${overlay_target}"
        "${rootfs_file}"
        "buildroot"
        "rootfs_install"
        rootfs_img
        "${rootfs_target}" # helper CMake target to be created, not used
    )
    list(APPEND rootfs_images "vm${i}_rootfs.cpio:${rootfs_img}")
endforeach()

# Decompress Linux Kernel image and add to file server
DecompressLinuxKernel(
    extract_linux_kernel # helper CMake target to be created, not used
    decompressed_kernel
    ${kernel_file}
)

# Create file server with VM images. There is an implicit dependency on the
# input files, so the rules above will be invoked to generate/update the files.
# There is no need to specify an explicit dependency on the helper targets
# 'extract_linux_kernel' and each '${rootfs_target}'
DefineCAmkESVMFileServer(FILES "bzimage:${decompressed_kernel}" "${rootfs_images}")

# Initialise CAmkES Root Server with addition CPP includes
DeclareCAmkESRootserver("${zmq_camkes}" CPP_INCLUDES "${VM_PROJECT_DIR}/components/VM")
