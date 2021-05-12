#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(demoapps)

# This finds and imports the ZMQ library that is built by another project
find_package(ZeroMQ "4.2.5" EXACT)

macro(create_bin name file)
	add_executable(${name} ${file})
	target_link_libraries(${name} -static libzmq-static)
	install(TARGETS ${name} DESTINATION bin/)
endmacro()


create_bin(client clientserver/client.c)
create_bin(server clientserver/server.c)

create_bin(pub pubsub/pub.c)
create_bin(sub pubsub/sub.c)

create_bin(sink pipeline/sink.c)
create_bin(source pipeline/source.c)
create_bin(worker pipeline/worker.c)
