#!/bin/sh
set -e
# Print an error message to capture error condition
trap "if [ \$? -ne 0 ]; then echo 'Exited with error'; fi" 0;

sysctl -w net.ipv4.ip_forward=1
ip route add default via 192.168.1.100 dev eth1 # camino salida
ip route add 10.0.1.0/24 via 192.168.2.1 dev eth0
iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
iptables -t nat -A PREROUTING -i eth1 -p udp --dport 51820 -j DNAT --to-destination 192.168.2.2:51820
iptables -A FORWARD -p udp -d 192.168.2.2 --dport 51820 -j ACCEPT
