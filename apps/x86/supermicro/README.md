<!--
  Copyright 2022, UNSW (ABN 57 195 873 179)

  SPDX-License-Identifier: BSD-2-Clause
-->

# Multi VM Demo Application

Please refer also to
https://docs.sel4.systems/Tutorials/camkes-vm-linux.html
for a general set up guide.

## Intro
This is a sample configuration for a Supermicro SYS-5019D-FN8TP
It sets up 4 32-bit Linux virtual machines (VMs).  VM0 has passthrough ethernet for LAN0
(PCI device 66:0:0); the other VMs have no passthrough devices.

Serial console on port COM2 (the one redirected to IPMI
Serial-over-LAN by the machine's firmware) provides debug output, and
redirection to the VM consoles on their port /dev/ttyS0 at 115200
baud.

Each VM has a virtio-net NIC that are connected together via a virtual
switch.  These come up unconfigured in the example.

VMs 0, 1, and 2 have partitions 1, 2, and 3 of the sata disc on the
hardware available as /dev/vda1

The root file system is a simple buildroot; login/password root/root


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
../init-build.sh -DCAMKES_VM_APP=supermicro
ninja
```

## Testing

Some tips for testing the Virtio Net and the passthrough ethernet.

1. Switching the serial port between different VMs: press `Return`,
   then `@`,  then the index number of the VM you want to switch to.

2. Testing Virtio Net
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
ip link set up dev eth1
udhcpc -i eth1
```

4. The SATA shared disk

The default configuration assumes a disc with three partitions.  These
are available as `/dev/vda1` in each of vm0, vm1, and vm2.

The sataserver code currently allows only primary MBR partitions, so
you are restricted to 4 max per disc.

## ISSUES

The supermicro machine has multiple PCI roots.  We replaced the
recursive scan for PCI devices with a brute force 'scan everything'
approach in PR .../util_libs/pull/132 to get around this without
having to include an ACPI bytecode interpreter in the kernel.

The supermicro machine has 5 IOAPICS.  The gigabit and 10Gb NICS are
on IOAPIC 1; the code for handling other than the 0th IOAPIC seems
buggy.  Right now the VM does not get any interrupts.

This is also true of anything plugged into the only PCI slot.

The SATA server currently polls for end of operation.  On a fast SSD
this isn;t that much of an issue; but should be fixed.


These PRs are currently outstanding:
   https://github.com/seL4/seL4/pull/324 improvements to allow 64-bit
   guests
   https://github.com/seL4/seL4_projects_libs/pull/16 Likewise...
   https://github.com/seL4/camkes-vm/pull/32 add the SATA config for
   the supermicro.
   https://github.com/seL4/camkes-vm-examples/pull/27 the supermicro
   config itself (where this file is at the moment)
   https://github.com/seL4/util_libs/pull/132 use brute force scan for
   PCI devices.
   
These features do not yet have a PR up:
  -- virtIO-socket
  -- virtIO-console between VMs
