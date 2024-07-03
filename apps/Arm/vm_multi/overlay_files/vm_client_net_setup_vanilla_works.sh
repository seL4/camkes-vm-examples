#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

# set -e
 
# ip link set up dev eth0
# udhcpc -i eth0

ip link set up dev eth0

node_str=$(cat /proc/cmdline | awk '{print $1}')
node_num="${node_str: -1}"

ip link add link eth0 name "eth0.10$node_num" type vlan id "10$node_num"

ip link set up dev "eth0.10$node_num"

ip addr add "192.168.10$node_num.1/24" dev "eth0.10$node_num"

# Replace 'default_gateway' with your actual gateway IP
ip route add default via 192.168.10"$node_num".9 dev "eth0.10$node_num"

# Ping default gateway for mac learning
ping -w 192.168.10$(($node_num + 1)).9 &

# Add vm0 into /etc/hosts
echo "192.168.10$node_num.9   vm0" >> /etc/hosts


