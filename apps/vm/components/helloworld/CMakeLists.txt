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

project(helloworld C)

file(GLOB sources src/*.c)

DeclareCAmkESComponent(helloworld
    SOURCES ${sources}
    INCLUDES src
    LIBS Configuration sel4allocman sel4simple sel4simple-default sel4arm-vmm sel4dma sel4vchan)
