#
# Copyright 2019, DornerWorks
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

# Define kernel config options
set(KernelSel4Arch "x86_64" CACHE STRING "" FORCE)

set(KernelMaxNumIOAPIC  5 CACHE STRING "" FORCE)
set(KernelHugePage OFF CACHE BOOL "" FORCE)

set(LibPlatSupportX86ConsoleDevice "com2" CACHE STRING "" FORCE)

ApplyCommonSimulationSettings(${KernelArch})
set(KernelIOMMU ON CACHE BOOL "" FORCE)

