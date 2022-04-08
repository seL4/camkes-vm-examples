#
# Copyright 2022, UNSW (ABN 57 195 873 179)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

# Define kernel config options
set(KernelSel4Arch "x86_64" CACHE STRING "" FORCE)
set(KernelMaxNumNodes 2 CACHE STRING "" FORCE)
