#
# Copyright 2019, DornerWorks
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.16.0)

# Define kernel config options
set(KernelSel4Arch
    ia32
    CACHE STRING ""
)

set(KernelMaxNumNodes
    1
    CACHE STRING "" FORCE
)
set(KernelHugePage
    OFF
    CACHE BOOL "" FORCE
)

ApplyCommonSimulationSettings(${KernelArch})
set(KernelIOMMU
    ON
    CACHE BOOL "" FORCE
)

# Use AHCI by default
set(SataserverUseAHCI
    ON
    CACHE STRING ""
)

set(CAmkESVMDestHardware
    "optiplex"
    CACHE STRING "" FORCE
)
