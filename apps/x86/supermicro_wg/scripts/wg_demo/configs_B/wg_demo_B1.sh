#!/bin/sh
set -e
# Print an error message to capture error condition
trap "if [ \$? -ne 0 ]; then echo 'Exited with error'; fi" 0;

sysctl -w net.ipv4.ip_forward=1
ip route add default via 192.168.2.3 dev eth0
ip route add 10.0.2.0/24 via 192.168.2.1 dev eth0
wg-quick up wg0_B