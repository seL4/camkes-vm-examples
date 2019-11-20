#
# Copyright 2019, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(DATA61_BSD)
#

set(supported "exynos5422;tx2")
if(NOT "${PLATFORM}" IN_LIST supported)
    message(FATAL_ERROR "PLATFORM: ${PLATFORM} not supported.
         Supported: ${supported}")
endif()
set(LibUSB OFF CACHE BOOL "" FORCE)
set(VmPCISupport ON CACHE BOOL "" FORCE)
if(VIRTIO_NET_PING)
    set(VmVirtioNetVirtqueue ON CACHE BOOL "" FORCE)
else()
    set(VmVirtioNetArping ON CACHE BOOL "" FORCE)
endif()
set(VmInitRdFile ON CACHE BOOL "" FORCE)
