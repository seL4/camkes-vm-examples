#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

for kv in $(cat /proc/cmdline); do
    key=$(echo $kv | cut -d '=' -f 1)
    val=$(echo $kv | cut -d '=' -f 2)
    if [ "$key" = "vlan" ]; then
        vlan="$val"
    fi
done

vdev="eth0.$vlan"

ip link set up dev eth0
ip link add link eth0 name $vdev type vlan id $vlan
ip link set up dev $vdev
udhcpc -i $vdev
