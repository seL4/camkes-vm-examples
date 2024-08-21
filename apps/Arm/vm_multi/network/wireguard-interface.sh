#!/bin/sh

# set -e

InterfaceName="wg0"
PrivateKey="/root/wg0.conf"
Address="10.0.0.1/24"
ListenPort="5810"


# Help menu
show_help() {
    echo "Set up basic of a wireguard server. You will still need to allocate allowable peers."
    echo
    echo "Usage: $0 [options] InterfaceName PrivateKey Address"
    echo
    echo "Options:"
    echo "  --interface,   Name of the wireguard interface you'd like to create"
    echo "  --pk,          Private Key of the new wireguard interface you're creating"
    echo "  --addr,        Address that this node can be addressed with inside wireguard"
    echo "  --port,        Set the port the wireguard server should be listening on"
    echo "  --help,        Print this help menu"
    echo
}


# Parse CLI flag arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --interface)
        shift
        InterfaceName="$1"
        shift
        ;;
    --pk)
        shift
        PrivateKey="$1"
        shift
        ;;
    --addr)
        shift
        Address="$1"
        shift
        ;;
    --port)
        shift
        ListenPort="$1"
        shift
        ;;
    --help)
        show_help
        exit 0
        ;;
  esac
done


sub_net="$(echo "10.198.10.1" | sed 's/^[0-9]*\.[0-9]*\.[0-9]*\.//').0/24"



# Set pubkey in wireguard
echo wg pubkey < $PrivateKey
wg pubkey < $PrivateKey


# Add a wireguard interface
echo ip link add dev $InterfaceName type wireguard
ip link add dev $InterfaceName type wireguard

# Add address for wireguard (used internally in wireguard comms)
echo ip address add dev $InterfaceName $Address
ip address add dev $InterfaceName $Address

# Set interface UP
echo ip link set $InterfaceName up
ip link set $InterfaceName up

# Set up route for easy resolution
echo ip route add $sub_net dev wg0 proto kernel scope link src $Address
ip route add $sub_net dev wg0 proto kernel scope link src $Address

# Set the priv key from wireguard instance
echo wg set $InterfaceName private-key $PrivateKey
wg set $InterfaceName private-key $PrivateKey

# Specify port inbound communication needs to come from
echo wg set $InterfaceName listen-port $ListenPort
wg set $InterfaceName listen-port $ListenPort


echo "Wireface setup complete"


# Allow a remote peer
# wg set wg0 peer $peer_pubkey allowed-ips $peer_wg_ip endpoint $peer_real_ip:$peer_real_port

# Contact remote host. Everything is set up now
# ping 10.0.0.2



# sudo wg set wg0 peer RSg/jyvPHmGtmt+8cHT/XwQfJbfYQydRCNJb/x+KjRY= allowed-ips 0.0.0.0/0,::/0 endpoint 10.10.10.113:5891 persistent-keepalive 25
# sudo ip route add 10.192.10.0/32 dev wg0 via 10.192.10.1



