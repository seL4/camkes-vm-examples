<!--
  Copyright 2019, Data61
  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
  ABN 41 687 119 230.
  This software may be distributed and modified according to the terms of
  the BSD 2-Clause license. Note that NO WARRANTY is provided.
  See "LICENSE_BSD2.txt" for details.
  @TAG(DATA61_BSD)
-->

# Virtio Net Demo Application

The `vm_virtio_net` application demonstrates the use of the VirtNet ethernet device in a Linux guest VM
in order to communicate to other components. In particular the application establishes a single VM that will
send and recieve packets over its `eth0` ethernet device.

### Virtio Net Ping

The Virtio Net Ping application demonstrates a Linux guest that runs `ping` over its `eth0` interface. This results
in ICMP Echo request packets being sent to the VMM over the Virtio Net driver. The VMM recieves these packets and
forwards them over a virtqueue-based interface connected to a native server component, being `PingClient`. The Ping Client
component will unpack the ICMP packets, print the packet contents and respond with an ICMP Echo Reply. The response is
sent back to the VMM over the virtqueue interface and forwarded to the Linux guest. The diagram below illustrates the
composition of this system.

    +--------------------------+ +-----------------------+
    |                          | |                       |
    |                          | |                       |
    |                          | |                       |
    |                          | |         LINUX         |
    |                          | |                       |
    |       PING CLIENT        | |                      <------+
    |                          | |                       |     |
    |                          | +-----------------------+     | VIRTIO NET (via eth0)
    |                          | +-----------------------+     |
    |                          | |                     <-------+
    |                          | |          VMM          |
    |           ^              | |                     <-------+
    +-----------|--------------+ +-----------------------+     |
    +-----------|----------------------------------------+     | VIRTQUEUES
    |           |                                        |     |
    |           +---------------+------------------------------+
    |                      SEL4 - CAMKES                 |
    |                                                    |
    +----------------------------------------------------+

##### Build and run the Virtio Net Ping application

The following instructions builds the application for the Odroid XU4:
```
mkdir build
cd build
 ../init-build.sh -DCAMKES_VM_APP=vm_virtio_net -DVIRTIO_NET_PING=1 -DAARCH32=1 -DPLATFORM=exynos5422
```

When built successfully, the created image will be found at the following location: `images/capdl-loader-image-arm-exynos5`.

Running the given image should produce the following output:

```
Testing ping on virtual interface:
PING 192.168.1.2 (192.168.1.2): 56 data bytes

Packet Contents:
0:	45 0 0 54 a1 92 40 0 40 1 15 c3 c0 a8 1
15:	1 c0 a8 1 2 8 0 60 65 7d 0 0 0 cf 99
30:	4b 0 0 0 0 0 0 0 0 0 0 0 0 0 0
45:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
60:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
75:	0 0 0 0 0 0 0 0 0
IP Header - Version: IPv4 protocol: 1 | src address: 192.168.1.1 | dest address: 192.168.1.2
ICMP Header - Type: 8 | id: 125 | seq: 0

64 bytes from 192.168.1.2: seq=0 ttl=64 time=4988.738 ms

Packet Contents:
0:	45 0 0 54 a2 22 40 0 40 1 15 33 c0 a8 1
15:	1 c0 a8 1 2 8 0 ac 8f 7d 0 0 1 73 6e
30:	5b 0 0 0 0 0 0 0 0 0 0 0 0 0 0
45:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
60:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
75:	0 0 0 0 0 0 0 0 0
IP Header - Version: IPv4 protocol: 1 | src address: 192.168.1.1 | dest address: 192.168.1.2
ICMP Header - Type: 8 | id: 125 | seq: 256

64 bytes from 192.168.1.2: seq=1 ttl=64 time=6025.266 ms

Packet Contents:
0:	45 0 0 54 a2 bf 40 0 40 1 14 96 c0 a8 1
15:	1 c0 a8 1 2 8 0 c0 bf 7d 0 0 2 4f 3d
30:	6b 0 0 0 0 0 0 0 0 0 0 0 0 0 0
45:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
60:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
75:	0 0 0 0 0 0 0 0 0
IP Header - Version: IPv4 protocol: 1 | src address: 192.168.1.1 | dest address: 192.168.1.2
ICMP Header - Type: 8 | id: 125 | seq: 512

64 bytes from 192.168.1.2: seq=2 ttl=64 time=7061.254 ms

Packet Contents:
0:	45 0 0 54 a3 23 40 0 40 1 14 32 c0 a8 1
15:	1 c0 a8 1 2 8 0 bf ef 7d 0 0 3 40 c
30:	7b 0 0 0 0 0 0 0 0 0 0 0 0 0 0
45:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
60:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
75:	0 0 0 0 0 0 0 0 0
IP Header - Version: IPv4 protocol: 1 | src address: 192.168.1.1 | dest address: 192.168.1.2
ICMP Header - Type: 8 | id: 125 | seq: 768

64 bytes from 192.168.1.2: seq=3 ttl=64 time=8097.177 ms

Packet Contents:
0:	45 0 0 54 a3 6a 40 0 40 1 13 eb c0 a8 1
15:	1 c0 a8 1 2 8 0 e1 1f 7d 0 0 4 f db
30:	8a 0 0 0 0 0 0 0 0 0 0 0 0 0 0
45:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
60:	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
75:	0 0 0 0 0 0 0 0 0
IP Header - Version: IPv4 protocol: 1 | src address: 192.168.1.1 | dest address: 192.168.1.2
ICMP Header - Type: 8 | id: 125 | seq: 1024

64 bytes from 192.168.1.2: seq=4 ttl=64 time=9133.245 ms

--- 192.168.1.2 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 4988.738/7061.136/9133.245 ms
```
