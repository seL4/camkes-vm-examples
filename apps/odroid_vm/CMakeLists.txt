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

project(camkes-arm-odroid-vm C)

include(../../arm_vm_helpers.cmake)

# Common build definitions
CAmkESAddImportPath(components)
CAmkESAddImportPath(interfaces)

# Create interface include library
add_library(app_include INTERFACE)
target_include_directories(app_include INTERFACE include)

# Create our CPP Flags based on ARM VM config variables
set(cpp_flags "-DKERNELARMPLATFORM_EXYNOS5410")
AddCamkesCPPFlag(cpp_flags
    CONFIG_VARS VmEmmc2NoDMA VmVUSB VmVchan Tk1DeviceFwd Tk1Insecure)

# Add VM components
add_subdirectory(components/pilot)
add_subdirectory(components/uart)
add_subdirectory(components/gpio)
add_subdirectory(components/pwm)
add_subdirectory(components/can)
add_subdirectory(components/spi)
add_subdirectory(components/clk)
add_subdirectory(components/timer)
# Add Vchan helloworld component if VmVchan is enabled
if(VmVchan)
    add_subdirectory(components/helloworld)
endif()
# Declare odroid root server
DeclareCAmkESRootserver(vm_odroid.camkes CPP_FLAGS ${cpp_flags})
