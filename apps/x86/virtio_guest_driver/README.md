<!--
  Copyright 2022, UNSW (ABN 57 195 873 179)

  SPDX-License-Identifier: BSD-2-Clause
-->

# Guest Block Driver Application

Please refer also to
https://docs.sel4.systems/Tutorials/camkes-vm-linux.html
for a general set-up guide.

## Intro

The Guest Block Driver Application demonstrates the communication between 2 VMs (as clients) and
one VM (as the driver) using CAmkES Dataports. The driver VM has a passthrough SATA device;
the other VMs have no passthrough devices.

The root file system is a simple buildroot; login/password: root/root.

TODO: detailed introduction

## Config

This is a sample configuration for a Supermicro SYS-5019D-FN8TP,
It should work on other x86 machines, with some changes to the configuration options.

The passthrough device configuration options in `virtio_guest_driver.camkes` may need to be changed. These include:
  - Physical address of the base of the memory-mapped device and I/O ports
  - Size of the memory region
  - IRQ-related information (PCI, MSI, etc)
  - PCI bus/dev/function information of the device


The configuration options `LibPlatSupportX86ConsoleDevice` and `KernelMaxNumIOAPIC` in `app_settings.cmake` also need to match
the machine that the sample runs on.

## Build and run the application

The following instructions build the application for x86_64:
```
mkdir build
cd build
../init-build.sh -DCAMKES_VM_APP=virtio_guest_driver
ninja
```

## Usages

launch the guest driver on the driver VM:
```
mount /dev/sda2 /mnt
dataport_sata_backend 2 &
```
the source code of `dataport_sata_backend` can be found here:

projects/vm-linux/camkes-linux-artifacts/camkes-linux-apps/camkes-connector-apps/pkgs/dataport/sata_backend.c

this application assumes that:
  - `/dev/uio1` and `/dev/uio2` exist (they should, if the dataports are set up correctly in `src/cross_vm_connections.c`)
  - `/mnt/data/file-image-1` and `/mnt/data/file-image-2` exist

A suitable file can be created thus:
```
cd /mnt/data/
dd if=/dev/zero of=file-image-1 bs=1M count=100
fdisk file-image-1 # do partition
losetup -o 1048576 -f file-image-1
mkfs -t ext2 /dev/loop0
losetup -D
```

mount the filesystem on client VMs:
```
mount /dev/vda1 /mnt
```

## Issues
TODO