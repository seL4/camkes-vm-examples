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

if(NOT CAMKES_VM_APP)
    set(CAMKES_VM_APP "optiplex9020" CACHE INTERNAL "")
endif()

add_subdirectory("${CAMKES_VM_APP}")
