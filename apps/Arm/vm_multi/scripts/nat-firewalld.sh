#!/bin/bash

TAP=tap0
EGRESS=eth0
NETWORK=10.10.10.0
NETMASK=255.255.255.0
GATEWAY=10.10.10.1
DHCPRANGE=10.10.10.100,10.10.10.254

ip link show dev $TAP &> /dev/null
if [ $? -ne 0 ]; then
    ip tuntap add dev $TAP mode tap
    ip link set up dev $TAP
    ip addr add dev $TAP $GATEWAY/$NETMASK
fi

# Enable routing
sysctl -w net.ipv4.ip_forward=1 > /dev/null 2>&1

# Enable masquerade and dhcp/dns on the TAP device
zone=$(firewall-cmd --get-default-zone)
firewall-cmd --zone=$zone --add-forward --permanent
firewall-cmd --zone=$zone --add-interface=$TAP --permanent
firewall-cmd --zone=$zone --add-service=dhcp --add-service=dns --permanent
firewall-cmd --reload

# Kill old dnsmasq instance
kill -9 $(cat /var/run/qemu-dnsmasq-$TAP.pid)

dns_cmd=(
    dnsmasq
    --strict-order
    --except-interface=lo
    --interface=$TAP
    --listen-address=$GATEWAY
    --bind-interfaces
    --dhcp-authoritative 
    --dhcp-range=$DHCPRANGE
    --conf-file=""
    --pid-file=/var/run/qemu-dnsmasq-$TAP.pid
    --dhcp-leasefile=/var/run/qemu-dnsmasq-$TAP.leases
    --dhcp-no-override
)

echo ${dns_cmd[@]} | bash
