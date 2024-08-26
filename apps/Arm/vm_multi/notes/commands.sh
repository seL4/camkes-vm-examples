To get ARPs on openvswitch to forward:
    ovs-ofctl add-flow br0 "arp, actions=NORMAL"

qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off \
    -nographic -cpu cortex-a53 -smp 4 \
    -m size=4096 \
    -netdev tap,id=mynet0,ifname=a32dacd4.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=12:90:45:c6:0e:c7 \
    -kernel /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/linux \
    -initrd /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/rootfs.cpio.gz \
    -append "node=gpu console=ttyAMA0"




qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -chardev pty,path=/dev/ttyAMA0,id=hostport \
    -device pci-serial,chardev=hostport \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off \
    -nographic -cpu cortex-a53 -smp 4 \
    -m size=4096 \
    -netdev tap,id=mynet0,ifname=a32dacd4.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=12:90:45:c6:0e:c7 \
    -kernel /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/linux \
    -initrd /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/rootfs.cpio.gz \
    -append "node=gpu console=ttyAMA0"


# Works but system doesn't believe its root
qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off \
    -nographic -cpu cortex-a53 -smp 4 \
    -m size=4096 \
    -netdev tap,id=mynet0,ifname=a32dacd4.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=12:90:45:c6:0e:c7 \
    -kernel /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/linux \
    -initrd /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/rootfs.cpio.gz \
    -append "node=gpu console=ttyAMA0 rootwait rw"

qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off \
    -nographic -cpu cortex-a53 -smp 4 \
    -m size=4096 \
    -netdev tap,id=mynet0,ifname=a32dacd4.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=12:90:45:c6:0e:c7 \
    -kernel /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/linux \
    -initrd /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/rootfs.cpio.gz \
    -append "node=gpu console=ttyAMA0 rw"



qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/r1/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/r1/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off \
    -nographic -cpu cortex-a53 -smp 4 \
    -m size=4096 \
    -netdev tap,id=mynet0,ifname=83767e06.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=8e:59:98:d3:5f:88 \
    -kernel /home/test/capsule-dev/camkes-vm-examples-manifest/build/vm_multi/images/capdl-loader-image-arm-qemu-arm-virt \
    -append "node=r1"


qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=fce7af1b.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=92:5e:77:41:33:b3 \
    -kernel /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/linux \
    -initrd /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/rootfs.cpio.gz \
    -append "node=gpu root=/dev/sda1"


heredoc << EOF
[    1.200213] Run /init as init process
mount: you must be root
mount: you must be root
mount: you must be root
mount: you must be root
can't open /dev/null: Permission denied
can't open /dev/null: Permission denied
can't open /dev/null: Permission denied
can't open /dev/null: Permission denied
hostname: sethostname: Operation not permitted
seedrng: can't determine pool size, assuming 256 bits: No such file or directory
Saving 256 bits of creditable seed for next boot
Starting syslogd: OK
Starting klogd: OK
Running sysctl: OK
Starting iptables: FAIL
Starting network: RTNETLINK answers: Operation not permitted
RTNETLINK answers: Operation not permitted
FAIL
Starting dropbear sshd: FAIL
EOF

# Didn't fix the issue
qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=fce7af1b.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=92:5e:77:41:33:b3 \
    -kernel /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/linux \
    -initrd /home/test/capsule-dev/camkes-vm-examples-manifest/projects/camkes-vm-images/qemu-arm-virt/rootfs.cpio.gz \
    -append "node=gpu root=/dev/sda1"


qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -device virtio-blk-device,drive=hd0 \
    -append "rootwait root=/dev/vda console=ttyAMA0" \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -smp 1 \
    -kernel /home/test/Files/buildroot-2024.05/output/images/Image \
    -append "rootwait console=ttyAMA0" \
    -initrd /home/test/Files/buildroot-2024.05/output/images/rootfs.cpio.gz \

    -M virt -cpu cortex-a53 \



sudo qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/test/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/test/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -nographic \
    -smp 4 \
    -kernel /home/test/Files/buildroot-2024.05/output/images/Image \
    -append "rootwait root=/dev/vda console=ttyAMA0" \
    -netdev user,id=eth0 \
    -device virtio-net-device,netdev=eth0 \
    -drive file=/home/test/Files/buildroot-2024.05/output/images/rootfs.qcow2,if=none,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    -netdev tap,id=mynet0,ifname=a32dacd4.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=12:90:45:c6:0e:c7 


sudo qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=c1f620d5.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=b6:d0:f7:cc:26:c0 \
    -kernel /home/test/Files/buildroot-2024.05/output/images/Image\
    -netdev user,id=eth0 \
    -device virtio-net-device,netdev=eth0 \
    -drive file=/home/test/Files/buildroot-2024.05/output/images/rootfs.ext4,if=none,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0\
    -append "rootwait root=/dev/vda console=ttyAMA0"

sudo qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/test/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/test/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=c1f620d9.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=b6:d0:f7:cc:26:c0 \
    -kernel /home/test/Files/buildroot-2024.05/output/images/Image\
    -netdev user,id=eth0 \
    -device virtio-net-device,netdev=eth0 \
    -drive file=/home/test/Files/buildroot-2024.05/output/images/rootfs.qcow2,if=none,format=qcow2,id=hd1 \
    -device virtio-blk-device,drive=hd1\
    -append "rootwait root=/dev/vda console=ttyAMA0"


sudo qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/test/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/test/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=c1f620d9.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=b6:d0:f7:cc:26:c0 \
    -kernel /home/test/Files/buildroot-2024.05/output/images/Image\
    -initrd /home/test/Files/buildroot-2024.05/output/images/rootfs.cpio.gz\
    -append "rootwait root=/dev/vda console=ttyAMA0"


sudo qemu-system-aarch64 \
    -pidfile /var/lib/amalthea/scenario1/gpu/pidfile \
    -serial unix:/var/lib/amalthea/scenario1/gpu/serial.sock,server,nowait \
    -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 \
    -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=c1f620d5.tap,script=no,downscript=no \
    -device e1000,netdev=mynet0,mac=b6:d0:f7:cc:26:c0 \
    -kernel /home/test/Files/buildroot-2024.05/output/images/Image\
    -netdev user,id=eth0 \
    -append "rootwait root=/dev/vda console=ttyAMA0 init=/bin/sh"
    # -drive file=/home/test/Files/buildroot-2024.05/output/images/rootfs.ext4,if=none,format=raw,id=hd0 \
    # -device virtio-net-device,netdev=eth0 \



sudo socat -,echo=0,raw UNIX-CONNECT:/var/lib/amalthea/scenario1/test/serial.sock
sudo socat -,echo=0,raw UNIX-CONNECT:/var/lib/amalthea/simple/gpu/serial.sock
sudo socat -,echo=0,raw UNIX-CONNECT:/var/lib/amalthea/simple/r1/serial.sock
sudo socat -,echo=0,raw UNIX-CONNECT:/var/lib/amalthea/simple/camera/serial.sock



sudo qemu-system-aarch64 -pidfile /var/lib/amalthea/simple/gpu/pidfile -serial unix:/var/lib/amalthea/simple/gpu/serial.sock,server,nowait -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=f5c36a87.tap,script=no,downscript=no -device e1000,netdev=mynet0,mac=da:e3:f9:c9:22:77 -kernel /home/test/Files/buildroot-2024.05/output/images/Image -drive file=/home/test/Files/buildroot-2024.05/output/images/rootfs_overlay.qcow2,if=none,format=qcow2,id=hd2574 -append "rootwait root=/dev/vda console=ttyAMA0"


sudo qemu-system-aarch64 -pidfile /var/lib/amalthea/simple/r1/pidfile -serial unix:/var/lib/amalthea/simple/r1/serial.sock,server,nowait -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=1830e49a.tap,script=no,downscript=no -device e1000,netdev=mynet0,mac=12:4a:26:03:99:d7 -kernel /home/briankoco/capsule-dev/camkes-vm-examples-manifest/build/vm_multi/images/capdl-loader-image-arm-qemu-arm-virt -append node=r1

sudo qemu-system-aarch64 -pidfile /var/lib/amalthea/simple/camera/pidfile -serial unix:/var/lib/amalthea/simple/camera/serial.sock,server,nowait -machine virt,virtualization=on,highmem=on,secure=off -nographic -cpu cortex-a53 -smp 4 -m size=4096 -netdev tap,id=mynet0,ifname=3bc782d4.tap,script=no,downscript=no -device e1000,netdev=mynet0,mac=42:b6:ed:2e:86:f0 -kernel /home/test/Files/buildroot-2024.05/output/images/Image -drive file=/home/test/Files/buildroot-2024.05/output/images/rootfs_overlay2.qcow2,if=none,format=qcow2,id=hd470 -append "rootwait root=/dev/vda console=ttyAMA0"
