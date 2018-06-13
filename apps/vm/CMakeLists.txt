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

project(camkes-arm-vm C)

# Function appends a given list of CMake config variables as CAmkES CPP flags
# 'configure_string': The variable to append the CPP flags onto
# 'CONFIG_VARS': followed by the CMake variables to turn into CPP flags.
# Converts the CMake variables to upper case i.e. VmVchan -> VMVCHAN.
# If the CMake variable is a boolean it passes the flag with a 0 or 1 value
# e.g VMVCHAN=0. Intended to be used with CMake variables that contain boolean
# values.
function(AddCamkesCPPFlag configure_string)
    cmake_parse_arguments(PARSE_ARGV 1 ADD_CPP
         ""
         ""
         "CONFIG_VARS"
    )
    foreach(configure_var IN LISTS ADD_CPP_CONFIG_VARS)
        # Convert the configuration variable name into uppercase
        string(TOUPPER  ${configure_var} config_var_name)
        if(${${configure_var}})
            # If ON value, set the flag to "=1"
            list(APPEND ${configure_string} "-D${config_var_name}=1")
        else()
            # If OFF value, set the flag to "=0"
            list(APPEND ${configure_string} "-D${config_var_name}=0")
        endif()
    endforeach()
    # Update the configure_string value
    set(${configure_string} "${${configure_string}}" PARENT_SCOPE)
endfunction(AddCamkesCPPFlag)

# Common build definitions
CAmkESAddImportPath(components)
CAmkESAddImportPath(interfaces)

# Create our CPP Flags based on ARM VM config variables
set(cpp_flags "")
AddCamkesCPPFlag(cpp_flags
    CONFIG_VARS VmEmmc2NoDMA VmVUSB VmVchan Tk1DeviceFwd Tk1Insecure)

if("${KernelARMPlatform}" STREQUAL "exynos5410")
    # Create interface include library
    add_library(exynos_include INTERFACE)
    target_include_directories(exynos_include INTERFACE include)
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
elseif("${KernelARMPlatform}" STREQUAL "tk1")
    # Declare tk1 root server
    DeclareCAmkESRootserver(vm_tk1.camkes CPP_FLAGS ${cpp_flags})
else()
    message(FATAL_ERROR "Invalid ARM platform")
endif()
