#
# Copyright 2022, UNSW (ABN 57 195 873 179)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

# Define kernel config options
set(KernelSel4Arch "x86_64" CACHE STRING "" FORCE)
set(KernelHugePage OFF CACHE BOOL "" FORCE)
set(KernelMaxNumIOAPIC  5 CACHE STRING "" FORCE)
# this is for the Serial-Over-Lan console via IPMI
set(LibPlatSupportX86ConsoleDevice "com2" CACHE STRING "" FORCE)

ApplyCommonSimulationSettings(${KernelArch})
set(KernelIOMMU ON CACHE BOOL "" FORCE)

# Use AHCI by default
set(SataserverUseAHCI ON CACHE STRING "")
set(CAmkESVMDestHardware "supermicro" CACHE STRING "" FORCE)