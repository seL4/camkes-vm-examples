#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

/usr/share/openvswitch/scripts/ovs-ctl start

# create mgmt vlan sub-interfaces
ip link add link eth0 name eth0.100 type vlan id 100
ip link add link eth0 name eth0.101 type vlan id 101
ip link add link eth0 name eth0.102 type vlan id 102

# add mgmt vlan sub-interfaces to the mgmt bridge
ovs-vsctl add-br mgbr
ovs-vsctl set bridge mgbr other-config:hwaddr=02:00:00:00:AA:01
ovs-vsctl add-port mgbr eth0.100
ovs-vsctl add-port mgbr eth0.101
ovs-vsctl add-port mgbr eth0.102
ovs-vsctl add-port mgbr eth1

ip link set up dev mgbr
ip link set up dev eth0
ip link set up dev eth0.100
ip link set up dev eth0.101
ip link set up dev eth0.102
ip link set up dev eth1

udhcpc -i mgbr

# create VNF bridge with a linear topology as
# vm0 -> vm1 -> vm2 -> vm3 -> vm0
## each VNF gets 2 VLAN ids:
##  vm1: 200, 201
##  vm2: 202, 203
##  vm3: 204, 205
ovs-vsctl add-br br0 -- set Bridge br0 fail_mode=secure
ovs-vsctl add-port br0 eth0
ovs-ofctl add-flow br0 in_port=eth0,dl_vlan=201,actions=mod_vlan_vid:202,output=in_port # vm1 -> vm2
ovs-ofctl add-flow br0 in_port=eth0,dl_vlan=202,actions=mod_vlan_vid:201,output=in_port # vm1 <- vm2
ovs-ofctl add-flow br0 in_port=eth0,dl_vlan=203,actions=mod_vlan_vid:204,output=in_port # vm2 -> vm3
ovs-ofctl add-flow br0 in_port=eth0,dl_vlan=204,actions=mod_vlan_vid:203,output=in_port # vm2 -> vm3

###
# Below is simply for testing
## create VLAN subinterfaces on vm0 to source and sink traffic, to test that the VNFs are forwarding
ip link add link eth0 name eth0.200 type vlan id 200
ip link add link eth0 name eth0.205 type vlan id 205

ip link set up dev eth0.200
ip link set up dev eth0.205
ip link set up dev br0

ip addr add 192.168.0.1/24 dev eth0.200

# if we ping 192.168.0.2, we should eventually see the ARP request hit eth0.205 if the VNFs are
# forwarding
