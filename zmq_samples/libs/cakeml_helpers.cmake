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

# Build a linear chain of CakeML/HOL scripts from a list of libraries
# Each library should contain @DEPENDENCY_PATH@ wherever it refers to its parent,
# e.g. in the list of `open`ed modules, and as an argument to `translation_extends`.
# This allows us to depend on a subset of available libraries, stitched together
# in the appropriate order. Output files are placed in `dest_dir`.
#
# Usage: CakeMLPP(dest_dir source_files...)
#        CakeMLPP(some/dest_dir dir1/firstDepScript.sml dir2/secondDepScript.sml ...)
function(CakeMLPP dest_dir)
    # First library should depend on camkesStart
    set(DEPENDENCY_PATH "camkesStart")
    foreach(source_file ${ARGN})
        get_filename_component(filename ${source_file} NAME)
        set(dest_file ${dest_dir}/${filename})
        configure_file(${source_file} ${dest_file} @ONLY)
        set_property(
            DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS "${source_file}"
        )
        string(
            REGEX
            REPLACE
                "Script.sml"
                ""
                theory_name
                ${filename}
        )
        set(DEPENDENCY_PATH ${theory_name})
    endforeach()
endfunction()
