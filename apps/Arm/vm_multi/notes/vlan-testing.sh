
# On local machine
dev=tap0
vid=3005
ip=10.35.10.1/24
sudo ip link add link $dev name $dev.$vid type vlan id $vid
sudo ip link set dev $dev.$vid up
sudo ip addr add $ip dev $dev.$vid


# On VM0
dev=eth1
vid=3005
ip link add link $dev name $dev.$vid type vlan id $vid
ip link set dev $dev.$vid up
ovs-vsctl add-port br0 $dev.$vid # tag=$vid

# On VM0
dev=eth0
vid=80
ip link add link $dev name $dev.$vid type vlan id $vid
ip link set dev $dev.$vid up
ovs-vsctl add-port br0 $dev.$vid # tag=$vid


ovs-ofctl add-flow br0 "arp,dl_vlan=3005,action=mod_vlan_vid=80,NORMAL"
ovs-ofctl add-flow br0 "arp,dl_vlan=3009,action=mod_vlan_vid=90,NORMAL"
ovs-ofctl add-flow br0 in_port=eth1,dl_vlan=3005,actions=mod_vlan_vid=80,output=eth0
ovs-ofctl add-flow br0 in_port=eth0,dl_vlan=80,actions=mod_vlan_vid=3005,output=eth1
ovs-ofctl add-flow br0 in_port=eth0,dl_vlan=90,actions=mod_vlan_vid=3009,output=eth1
ovs-ofctl add-flow br0 in_port=eth1,dl_vlan=3009,actions=mod_vlan_vid=90,output=eth0


# On VM0
ovs-vsctl add-port br0 eth1.3005 ovs-vsctl add-port br0 eth0.80
ovs-ofctl add-flow br0 "arp, actions=NORMAL"
ovs-ofctl add-flow br0 in_port=eth1.3005,actions=strip_vlan,output=eth0.80
ovs-ofctl add-flow br0 in_port=eth0.80,actions=strip_vlan,mod_vlan_vid:3005,output=eth1.3005

ovs-ofctl add-flow br0 "arp, actions=NORMAL"
ovs-ofctl add-flow br0 in_port=eth1,actions=strip_vlan,mod_vlan_vid:80,output=eth0
ovs-ofctl add-flow br0 in_port=eth0,actions=strip_vlan,mod_vlan_vid:3005,output=eth1


# On VM1
dev=eth0
vid=80
ip=10.35.10.2/24
ip link add link $dev name $dev.$vid type vlan id $vid
ip link set dev $dev.$vid up
ip addr add $ip dev $dev.$vid


# dev=eth0
# vid=3005
# ip=10.35.10.2/24
# ip link add link $dev name $dev.$vid type vlan id $vid
# ip link set dev $dev.$vid up
# ip addr add $ip dev $dev.$vid


# Run on local machine
# Add default route via that link
ip route add 10.36.10.0/24 via 10.35.10.1


# Forwarding logic for the router itself an IP in the second network
dev=eth0
vid=90
ip=10.39.10.2/24
ip link add link $dev name $dev.$vid type vlan id $vid
ip link set dev $dev.$vid up
ip addr add $ip dev $dev.$vid
# Allow for ARP forwarding
sysctl -w net.ipv4.conf.eth0.80.proxy_arp=1


# On VM0
dev=eth1
vid=3009
ip link add link $dev name $dev.$vid type vlan id $vid
ip link set dev $dev.$vid up
ovs-vsctl add-port br0 $dev.$vid # tag=$vid
# On VM0
dev=eth0
vid=90
ip link add link $dev name $dev.$vid type vlan id $vid
ip link set dev $dev.$vid up
ovs-vsctl add-port br0 $dev.$vid # tag=$vid
# Set up the return trip out of VM0
ovs-ofctl add-flow br0 in_port=eth1.3009,actions=strip_vlan,output=eth0.90
ovs-ofctl add-flow br0 in_port=eth0.90,actions=strip_vlan,mod_vlan_vid:3009,output=eth1.3009


# On your local machine
dev=tap0
vid=3009
ip=10.39.10.5/24
sudo ip link add link $dev name $dev.$vid type vlan id $vid
sudo ip link set dev $dev.$vid up
sudo ip addr add $ip dev $dev.$vid




