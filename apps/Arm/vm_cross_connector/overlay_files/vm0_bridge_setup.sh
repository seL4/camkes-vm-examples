#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

ifconfig eth0 up
ifconfig eth1 up
brctl addbr br0
brctl addif br0 eth0
brctl addif br0 eth1
udhcpc -i br0
