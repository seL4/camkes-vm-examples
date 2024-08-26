#!/bin/bash

TAP=tap0
TAP_VID=2
EGRESS=eth0
NETWORK=10.10.10.0
NETMASK=255.255.255.0
GATEWAY=10.10.10.1
DHCPRANGE=10.10.10.100,10.10.10.254

ip link show dev $TAP &> /dev/null
if [ $? -ne 0 ]; then
    ip tuntap add dev $TAP mode tap
    ip link set up dev $TAP
fi

ip link show dev $TAP.$TAP_VID &> /dev/null
if [ $? -ne 0 ]; then
    # Add device
    ip link add link $TAP name $TAP.$TAP_VID type vlan id $TAP_VID
    ip addr add dev $TAP.$TAP_VID $GATEWAY/$NETMASK
    ip link set dev $TAP.$TAP_VID up
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
    --interface=$TAP.$TAP_VID
    --listen-address=$GATEWAY
    --bind-interfaces
    --dhcp-authoritative 
    --dhcp-range=$DHCPRANGE
    --conf-file=""
    --pid-file="/var/run/qemu-dnsmasq-$TAP-$TAP_VID.pid"
    --dhcp-leasefile="/var/run/qemu-dnsmasq-$TAP-$TAP_VID.leases"
    --dhcp-no-override
)

echo ${dns_cmd[@]} | bash
