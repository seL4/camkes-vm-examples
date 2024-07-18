#!/bin/sh
#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

ip link set up dev eth0

ip link add link eth0 name eth0.100 type vlan id 100
node_str=$(cat /proc/cmdline | awk '{print $1}')
node_num="${node_str: -1}"

ip link set up dev eth0.100

ip addr add 192.168.100."$node_num"/24 dev eth0.100

# Replace 'default_gateway' with your actual gateway IP
ip route add default via 192.168.100.9 dev eth0.100

ping 192.168.100.$(($node_num + 1)) &



# # Function to repeatedly request DHCP in the background
# request_dhcp() {
#   while ! udhcpc -i eth0.100; do
#     echo "Retrying DHCP request..."
#     sleep 5  # Wait for 5 seconds before retrying
#   done
#   echo "DHCP request successful."
# }
# 
# # Run the request_dhcp function in the background
# request_dhcp &
# 
# # Continue with other tasks or just exit
# echo "DHCP request loop is running in the background."

