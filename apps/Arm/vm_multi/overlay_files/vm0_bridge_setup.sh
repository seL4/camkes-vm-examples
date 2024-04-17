#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

ip link add name br0 type bridge
ip link set up dev br0
ip link set up dev eth0
ip link set up dev eth1
ip link set master br0 dev eth0
ip link set master br0 dev eth1
udhcpc -i br0
