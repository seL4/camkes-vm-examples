#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(gpio C)

file(GLOB sources src/*.c)

DeclareCAmkESComponent(gpio
    SOURCES ${sources}
    INCLUDES include 
    LIBS app_include)
