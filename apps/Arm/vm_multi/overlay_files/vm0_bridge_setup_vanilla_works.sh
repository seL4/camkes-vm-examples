#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

# ip link add name br0 type bridge
# ip link set up dev br0
ip link set up dev eth0
ip link set up dev eth1
# ip link set master br0 dev eth0
# ip link set master br0 dev eth1
# udhcpc -i br0


#!/bin/bash Create VLAN interfaces
ip link add link eth0 name eth0.101 type vlan id 101
ip link add link eth0 name eth0.102 type vlan id 102
ip link add link eth0 name eth0.103 type vlan id 103

# Assign IP addresses (optional)
ip addr add 192.168.101.9/24 dev eth0.101
ip addr add 192.168.102.9/24 dev eth0.102
ip addr add 192.168.103.9/24 dev eth0.103

# Bring up the interfaces
ip link set eth0.101 up
ip link set eth0.102 up
ip link set eth0.103 up

# Enable IP forwarding
sysctl -w net.ipv4.ip_forward=1

# Set up routing rules (if needed)
ip route add 192.168.101.0/24 dev eth0.101
ip route add 192.168.102.0/24 dev eth0.102
ip route add 192.168.103.0/24 dev eth0.103

# Configure firewall rules (optional)
iptables -A FORWARD -i eth0.101 -o eth0.102 -j ACCEPT
iptables -A FORWARD -i eth0.102 -o eth0.101 -j ACCEPT

iptables -A FORWARD -i eth0.103 -o eth0.102 -j ACCEPT
iptables -A FORWARD -i eth0.102 -o eth0.103 -j ACCEPT

iptables -A FORWARD -i eth0.101 -o eth0.103 -j ACCEPT
iptables -A FORWARD -i eth0.103 -o eth0.101 -j ACCEPT

