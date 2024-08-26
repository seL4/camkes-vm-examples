#!/bin/sh

set -e

host="127.0.0.1"
dev_one="wg0"
dev_two="wg1"
ip1="127.0.0.1"
ip2="127.0.0.1"

# Help menu
show_help() {
    echo "Adds an entry into /etc/hosts on a remote host"
    echo
    echo "Usage: $0 [options] host dev1 ip1 dev2 ip2"
    echo
    echo "Options:"
    echo "  --host,    Specify the node the router should be built on"
    echo "  --dev1,    Specify the first device IPs should be routed from/to"
    echo "  --ip1,     Specify the ip and subnet the first device needs to forward"
    echo "  --dev2,    Specify the second device IPs should be routed from/to"
    echo "  --ip2,     Specify the ip and subnet the second device needs to forward"
    echo
}


# Parse CLI
while [[ $# -gt 0 ]]; do
  case $1 in
    --host)
        shift
        host="$1"
        shift
        ;;
    --dev1)
        shift
        dev_one="$1"
        shift
        ;;
    --dev2)
        shift
        dev_two="$1"
        shift
        ;;
     --ip1)
        shift
        ip1="$1"
        shift
        ;;
    --ip2)
        shift
        ip2="$1"
        shift
        ;;
   --help)
        show_help
        exit 0
        ;;
  esac
done

if [[ -z $host && $# -ge 1 ]]; then
    host_ip="$1"
    shift
fi
if [[ -z $dev_one && $# -ge 1 ]]; then
    dev_one="$1"
    shift
fi
if [[ -z $ip1 && $# -ge 1 ]]; then
    ip1="$1"
    shift
fi
if [[ -z $dev_two && $# -ge 1 ]]; then
    dev_two="$1"
    shift
fi
if [[ -z $ip2 && $# -ge 1 ]]; then
    ip2="$1"
    shift
fi



# Verify we have all the arguments
if [[ -z $host || -z $dev_one || -z $dev_two || -z $ip1|| -z $ip2 ]]; then
    echo "Error: host, dev1, ip1, dev2, and ip2 are required."
    show_help
    exit 1
fi

# Set up interface one on the node
echo ip addr add $ip1 dev $dev_one
sshpass -p "root" dbclient -y "$host" "ip addr add $ip1 dev $dev_one"
echo ip link set $dev_one up
sshpass -p "root" dbclient -y "$host" "ip link set $dev_one up"

# Set up interface two on the node
echo ip addr add $ip2 dev $dev_two
sshpass -p "root" dbclient -y "$host" "ip addr add $ip2 dev $dev_two"
echo ip link set $dev_two up
sshpass -p "root" dbclient -y "$host" "ip link set $dev_two up"


# Enable IP forwarding
echo sshpass -p "root" dbclient -y "$host" "echo 1 | tee /proc/sys/net/ipv4/ip_forward"
sshpass -p "root" dbclient -y "$host" "echo 1 | tee /proc/sys/net/ipv4/ip_forward"

echo sshpass -p "root" dbclient -y "$host" "echo 'net.ipv4.ip_forward = 1' >> /etc/sysctl.conf"
sshpass -p "root" dbclient -y "$host" "echo 'net.ipv4.ip_forward = 1' >> /etc/sysctl.conf"

echo sshpass -p "root" dbclient -y "$host" "sysctl -p"
sshpass -p "root" dbclient -y "$host" "sysctl -p"

# Allow forwarding from Network 1 to Network 2
echo sshpass -p "root" dbclient -y "$host" "iptables -A FORWARD -i $dev_one -o $dev_two -j ACCEPT"
sshpass -p "root" dbclient -y "$host" "iptables -A FORWARD -i $dev_one -o $dev_two -j ACCEPT"

# Allow forwarding from Network 2 to Network 1
echo sshpass -p "root" dbclient -y "$host" "iptables -A FORWARD -i $dev_two -o $dev_one -j ACCEPT"
sshpass -p "root" dbclient -y "$host" "iptables -A FORWARD -i $dev_two -o $dev_one -j ACCEPT"

# Set up masquerading on the interface
echo iptables -t nat -A POSTROUTING -o $dev_one -j MASQUERADE
sshpass -p "root" dbclient -y "$host" "iptables -t nat -A POSTROUTING -o $dev_one -j MASQUERADE"

