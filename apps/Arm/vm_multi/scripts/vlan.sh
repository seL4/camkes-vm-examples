# # VLAN 100 interface
sudo ip link add link eth0 name eth0.100 type vlan id 100
sudo ip addr add 192.168.100.1/24 dev eth0.100
sudo ip link set dev eth0.100 up

sudo ovs-vsctl add-br br0
sudo ovs-vsctl add-port br0 eth1

# Create and configure trunk ports
sudo ovs-vsctl add-port br0 trunk0 -- set interface trunk0 type=internal
sudo ovs-vsctl add-port br0 trunk1 -- set interface trunk1 type=internal

# Set the allowed VLANs on the trunk ports
sudo ovs-vsctl set port trunk0 trunks=100,101
sudo ovs-vsctl set port trunk1 trunks=102,103

