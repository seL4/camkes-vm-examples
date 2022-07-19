<!--
  Copyright 2022, UNSW (ABN 57 195 873 179)

  SPDX-License-Identifier: BSD-2-Clause
-->

# Multi VM Demo Application

## Intro(TODO)

## Config

The passthrough device configuration options in `supermicro.camkes` may need to be changed. These include:
  - Physical address of the base of the memory-mapped device and I/O ports
  - Size of the memory region
  - IRQ-related information (PCI, MSI, etc)
  - PCI bus/dev/function information of the device

## Build and run the application

The following instructions build the application for x86_64:
```
mkdir build
cd build
../init-build.sh -DCAMKES_VM_APP=supermicro -DPLATFORM=supermicro
ninja
```

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
