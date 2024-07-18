#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

# create vlan sub-interfaces
ip link add link eth0 name eth0.100 type vlan id 100
ip link add link eth0 name eth0.101 type vlan id 101
ip link add link eth0 name eth0.102 type vlan id 102

/usr/share/openvswitch/scripts/ovs-ctl start

ovs-vsctl add-br br0
ovs-vsctl add-port br0 eth0.100
ovs-vsctl add-port br0 eth0.101
ovs-vsctl add-port br0 eth0.102
ovs-vsctl add-port br0 eth1

ip link set up dev br0
ip link set up dev eth0
ip link set up dev eth0.100
ip link set up dev eth0.101
ip link set up dev eth0.102
ip link set up dev eth1

udhcpc -i br0
