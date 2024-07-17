#!/bin/bash

if [ "$#" -eq 1 ]; then
    image=$1
else
    # use default
    #image=$HOME/GitHub/capsule-dev/camkes-vm-examples-manifest/build/vm_multi/images/capdl-loader-image-arm-qemu-arm-virt
    image=$HOME/capsule-dev/camkes-vm-examples-manifest/build/vm_multi/images/capdl-loader-image-arm-qemu-arm-virt
fi

if [ ! -f $image ]; then
    echo "$image does not exist"
    exit 1
fi

qemu-system-aarch64 \
    -machine virt,virtualization=on,highmem=on,secure=off \
    -cpu cortex-a53 \
    -nographic \
    -smp 4 \
    -m size=4096 \
    -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=52:55:00:d1:55:01 \
    -kernel $image

    #-device virtio-net,netdev=mynet0,mac=52:55:00:d1:55:01,disable-modern=on,disable-legacy=off \
    #-kernel $HOME/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/linux \
    #-initrd $HOME/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/rootfs.cpio.gz
    #-machine virt,virtualization=on,highmem=on,secure=off,dumpdtb=dump.dtb \
