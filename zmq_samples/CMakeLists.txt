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

project(zmq_samples)

# Include CAmkES VM helper functions
include("../../vm/camkes_vm_helpers.cmake")
include("../../vm-linux/vm-linux-helpers.cmake")

# Define kernel config options
set(KernelX86Sel4Arch x86_64 CACHE STRING "" FORCE)
set(KernelMaxNumNodes 1 CACHE STRING "" FORCE)

# Number of vms.
set(NumberVMs 3)
# CMake foreach RANGE loops are 0..num inclusive....
MATH(EXPR NumberVMsLess1 "${NumberVMs}-1")

# Declare VM component: Init
foreach(i RANGE ${NumberVMsLess1})
    DeclareCAmkESVM("Init${i}")
endforeach()

DeclareCamkesComponent(virtqueueinit)

# Get Default Linux VM files
GetDefaultLinuxKernelFile(kernel_file)
GetDefaultLinuxRootfsFile(rootfs_file)

foreach(i RANGE ${NumberVMsLess1})
    AddFileToOverlayDir("interfaces" ${CMAKE_CURRENT_SOURCE_DIR}/scripts/network_interface_${i} "etc/network" overlay_vm${i})
endforeach()

set(SYSROOT "${CMAKE_CURRENT_BINARY_DIR}/sysroot")
FindCustomPollyToolchain(LINUX_32BIT_TOOLCHAIN "linux-gcc-7-32bit-pic")

ExternalProject_Add(zmq_external
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/tools/libzmq
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/zmq_external
    BUILD_ALWAYS ON
    EXCLUDE_FROM_ALL
    CMAKE_ARGS
      -DCMAKE_TOOLCHAIN_FILE=${LINUX_32BIT_TOOLCHAIN}
      -DCMAKE_INSTALL_PREFIX=${SYSROOT}
      -DPOLLER=epoll
      -DWITH_PERF_TOOL=ON
      -DBUILD_TESTS=OFF
)

ExternalProject_Add(demoapps_external
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/user
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/user
    DEPENDS zmq_external
    BUILD_ALWAYS ON
    EXCLUDE_FROM_ALL
    CMAKE_ARGS
      -DCMAKE_TOOLCHAIN_FILE=${LINUX_32BIT_TOOLCHAIN}
      -DCMAKE_INSTALL_PREFIX=${SYSROOT}
)


DeclareExternalProjObjectFiles(demoapps_external ${SYSROOT}/bin FILES client server sink source worker pub sub)
macro(AddZMQBinary binary_name)
    foreach(i RANGE ${NumberVMsLess1})
        AddFileToOverlayDir("${binary_name}" ${CMAKE_CURRENT_BINARY_DIR}/sysroot/bin/${binary_name} "bin/" overlay_vm${i})
    endforeach()
endmacro()

AddZMQBinary(client)
AddZMQBinary(server)
AddZMQBinary(sink)
AddZMQBinary(source)
AddZMQBinary(worker)
AddZMQBinary(pub)
AddZMQBinary(sub)


foreach(i RANGE ${NumberVMsLess1})
    AddFileToOverlayDir("S91zmq_bench" ${CMAKE_CURRENT_SOURCE_DIR}/src/user/zmq_bench.sh "etc/init.d" overlay_vm${i})
    # Add the buildroot overlay to our bare rootfs image
    AddOverlayDirToRootfs(overlay_vm${i} ${rootfs_file} "buildroot" "rootfs_install"
        output_rootfs_location_vm${i} rootfs_target_vm${i})
    AddToFileServer("vm${i}_rootfs.cpio" "${output_rootfs_location_vm${i}}" DEPENDS rootfs_target_vm${i})
endforeach()


# Decompress Linux Kernel image and add to file server
DecompressLinuxKernel(extract_linux_kernel decompressed_kernel ${kernel_file})
AddToFileServer("bzimage" ${decompressed_kernel} DEPENDS extract_linux_kernel)

# Initialise CAmkES Root Server with addition CPP includes
DeclareCAmkESVMRootServer(zmq_samples.camkes)
