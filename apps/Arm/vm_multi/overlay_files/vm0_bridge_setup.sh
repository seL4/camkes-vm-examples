#!/bin/sh
#
# Copyright 2020, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(DATA61_BSD)
#

set -e

ifconfig eth0 up
ifconfig eth1 up
brctl addbr br0
brctl addif br0 eth0
brctl addif br0 eth1
udhcpc -i br0
