#
# Copyright 2023, DornerWorks
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

# Define kernel config options
set(KernelSel4Arch x86_64 CACHE STRING "" FORCE)
set(KernelX86_64VTX64BitGuests ON CACHE BOOL "" FORCE)
set(KernelMaxNumNodes 1 CACHE STRING "" FORCE)
set(LibSel4VMMUseHPET ON CACHE BOOL "" FORCE)
