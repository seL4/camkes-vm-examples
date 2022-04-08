<!--
  Copyright 2022, UNSW (ABN 57 195 873 179)

  SPDX-License-Identifier: BSD-2-Clause
-->

# Multi VM Demo Application

The `multi` application demonstrates the use of the VirtNet ethernet device in three Linux guest VMs in order to communicate to other VMs.

The diagram below illustrates the composition of this system.

          +------------+            +------------+            +------------+
          |            |            |            |            |            |
          |            |            |            |            |            |
          |            |            |            |            |            |
          |   LINUX    |            |   LINUX    |            |   LINUX    |
          |    vm0     |            |    vm1     |            |    vm2     |
    +------>          <------+      |           <------+      |           <------+
    |     |            |     |      |            |     |      |            |     |
    |     +------------+     | (1)  +------------+     | (1)  +------------+     | (1)
    |     +------------+     |      +------------+     |      +------------+     |
    |     |          <-------+      |          <-------+      |          <-------+
    | (3) |    VMM0    |            |    VMM1    |            |    VMM2    |
    |     |          <-------+      |          <-------+      |          <-------+
    |     +------------+     | (2)  +------------+     | (2)  +------------+     | (2)
    |     +------------------|-------------------------|-------------------+     |
    |     |                  |                         |                   |     |
    +------>                 +-------------------------+-------------------------+
          |                          SEL4 - CAMKES                         |
          |                                                                |
          +----------------------------------------------------------------+

(1) VIRTIO NET (via eth0)
(2) VIRTQUEUES
(3) PASSTHROUGH ETHERNET (via eth1)

Each Linux guest VM is able to send and receive packets to others. e.g., when `vm0` sends packets to `vm1`, the packets are first sent to `VMM0` over the Virtio Net driver. The VMM receives these packets and forwards them to `VMM1` over a virtqueue-based interface, and `VMM1` forwards them to the `vm1`.

Additionally, `vm0` has passthrough ethernet and is able to communicate with the ``outside world''

## Config

The passthrough device configuration options in `multi.camkes` may need to be changed. These include:
  - Physical address of the base of the memory-mapped device and I/O ports
  - Size of the memory region
  - IRQ-related information (PCI, MSI, etc)
  - PCI bus/dev/function information of the device

## Build and run the application

The following instructions build the application for x86_64:
```
mkdir build
cd build
../init-build.sh -DCAMKES_VM_APP=multi
ninja
```
Then it should be able to be run on a haswell-like machine.

## Testing

Some tips for testing the Virtio Net and the passthrough ethernet.

1. Switching the serial port between different VMs: press `Return`, then `@`,  then the index number of the VM we want to switch to.

2. Testing the Virtio Net
```
# on vm0
ifconfig eth0 192.168.1.1

# on vm1
ifconfig eth0 192.168.1.2
ping 192.168.1.1
```

3. Setting up the passthrough ethernet
```
# on vm0
ifconfig eth1 up
udhcpc -i eth1
```
