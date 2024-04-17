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

sysctl -w net.ipv4.ip_forward=1 > /dev/null 2>&1

iptables --flush
iptables -t nat -F
iptables -X
iptables -Z
#iptables -P PREROUTING ACCEPT
#iptables -P POSTROUTING ACCEPT
iptables -P OUTPUT ACCEPT
iptables -P INPUT ACCEPT
iptables -P FORWARD ACCEPT
iptables -A INPUT -i $TAP -p tcp -m tcp --dport 67 -j ACCEPT
iptables -A INPUT -i $TAP -p udp -m udp --dport 67 -j ACCEPT
iptables -A INPUT -i $TAP -p tcp -m tcp --dport 53 -j ACCEPT
iptables -A INPUT -i $TAP -p udp -m udp --dport 53 -j ACCEPT
iptables -A FORWARD -i $TAP -o $TAP -j ACCEPT
iptables -A FORWARD -s $NETWORK/$NETMASK -i $TAP -j ACCEPT
iptables -A FORWARD -d $NETWORK/$NETMASK -o $TAP -m state --state RELATED,ESTABLISHED -j ACCEPT
# make a distinction between the tap packets and routed packets, don't want the
# bridged frames/packets to be masqueraded.
iptables -t nat -A POSTROUTING -s $NETWORK/$NETMASK -d $NETWORK/$NETMASK -j ACCEPT
iptables -t nat -A POSTROUTING -s $NETWORK/$NETMASK -j MASQUERADE

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

# set the traffic get through the egress
iptables -A FORWARD -i $TAP -o $EGRESS -j ACCEPT
iptables -t nat -A POSTROUTING -o $EGRESS -j MASQUERADE
# let the known traffic get back at tap
iptables -A FORWARD -i $EGRESS -o $TAP -m state --state RELATED,ESTABLISHED -j ACCEPT
