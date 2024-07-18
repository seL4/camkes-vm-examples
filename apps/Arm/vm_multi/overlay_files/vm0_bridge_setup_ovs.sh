#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

echo 1 > /proc/sys/net/ipv4/ip_forward

# ip link add name br0 type bridge
# ip link set up dev br0
ip link set up dev eth0
ip link set up dev eth1
# ip link set master br0 dev eth0
# ip link set master br0 dev eth1
# udhcpc -i br0

/usr/share/openvswitch/scripts/ovs-ctl start

ovs-vsctl add-br br0
ovs-vsctl add-port br0 eth0
# ovs-vsctl add-port br0 eth1

ovs-vsctl add-port br0 vlan101 tag=101 -- set interface vlan101 type=internal
ovs-vsctl add-port br0 vlan102 tag=102 -- set interface vlan102 type=internal
ovs-vsctl add-port br0 vlan103 tag=103 -- set interface vlan103 type=internal

ip addr add 192.168.101.9/24 dev vlan101
ip addr add 192.168.102.9/24 dev vlan102
ip addr add 192.168.103.9/24 dev vlan103

ip link set up dev br0
ip link set up dev vlan101
ip link set up dev vlan102
ip link set up dev vlan103

iptables -A FORWARD -i vlan101 -o vlan102 -j ACCEPT
iptables -A FORWARD -i vlan102 -o vlan101 -j ACCEPT

iptables -A FORWARD -i vlan101 -o vlan103 -j ACCEPT
iptables -A FORWARD -i vlan103 -o vlan101 -j ACCEPT

iptables -A FORWARD -i vlan102 -o vlan103 -j ACCEPT
iptables -A FORWARD -i vlan103 -o vlan102 -j ACCEPT



# dnsmasq \
# --interface=vlan100 \
# --dhcp-range=vlan100,192.168.100.10,192.168.100.100,255.255.255.0,12h \
# --dhcp-option=vlan100,3,192.168.100.1 \
# --dhcp-option=vlan100,6,8.8.8.8,8.8.4.4
# 
# # Function to repeatedly request DHCP in the background
# request_dhcp() {
#   echo "Starting DHCP client..."
#   while ! udhcpc -i vlan100; do
#     echo "Retrying DHCP request on vlan100..."
#     sleep 5  # Wait for 5 seconds before retrying
#   done
#   echo "DHCP request successful."
# }
# 
# # Run the request_dhcp function in the background
# request_dhcp &

