<!--
  Copyright 2016, Data61
  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
  ABN 41 687 119 230.
  This software may be distributed and modified according to the terms of
  the BSD 2-Clause license. Note that NO WARRANTY is provided.
  See "LICENSE_BSD2.txt" for details.
  @TAG(DATA61_BSD)

-->

camkes-arm-vm
=============

This repo is for running virtualised Linux on seL4 for arm platforms.

Currently the exynos5, tk1 and tx2 machines are supported.

## tk1 configuration

We currently provide two linux binaries and two device tree configurations.
* `linux-tk1-debian` will try and load a debian userspace off of an emmc partition
* `linux-tk1-initrd` will load an included buildroot ramdisk
* `linux-tk1-secure.dts` is a device tree configuration with the devices that aren't provided to the linux are disabled.
* `linux-tk1-nonsecured.dts` is a device tree configuration with all devices enabled.

In the tk1 app configuration there is a `boot mode selection` option that chooses between the two linux binaries, and
an `Insecure` option which selects whether to provide all hardware devices to the linux vm or not. If `Insecure` is not set
then the VM will only be provided a minimal amount of hardware frames and the `linux-tk1-secure.dts` DTS will be used.  If `Insecure`
is set then the VM will be provided all device frames apart from the Memory controller and the `linux-tk1-nonsecured.dts` DTS will be used.

U-Boot is required to initialise the USB devices if `linux-tk1-secure.dts` is used.  This is done by: `usb start 0`.

The tk1 currently uses linux binaries constructed using [Buildroot 2016.08.1](https://buildroot.org/downloads/) with the following configs:
* `buildroot_tk1_initrd_defconfig` builds linux with an included ramdisk that will be loaded at boot.
* `buildroot_tk1_emmc_defconfig` builds linux without a ramdisk and it will mount and run /sbin/init /dev/mmcblk0p2

To copy the files back into this project, change into the Linux directory and run make for each file as shown below. **Note that you can only
update one of linux-tk1-debian or linux-tk1-initrd at a time as they are built from different Buildroot configs**:
```
cd projects/vm/linux
LINUX_FILE=linux-tk1-initrd BUILDROOT_DIR=~/workspaces/buildroot-2016.08.1/ make
LINUX_FILE=linux-tk1-dtb-secure BUILDROOT_DIR=~/workspaces/buildroot-2016.08.1/ make
```


Both of these configs use the tegra124-jetson-tk1-sel4vm-secure device tree file.  
To change to the tegra124-jetson-tk1-sel4vm.dts this will need to be changed in the buildroot make menuconfig.
To change the mounted emmc partition the chosen dts file's bootargs entry will need to be updated.

The linux version used can be found in the seL4 branch of our [linux-tegra repo](https://github.com/SEL4PROJ/linux-tegra/tree/sel4).


When buildroot starts:
```
buildroot login: root
#
```
When debian starts:
```
Debian GNU/Linux 8 tk1 ttyS0
tk1 login: root
Password: root
root@tk1:~#
```



### wireless configuration
To configure the Ralink 2780 USB wifi dongle:

```
ifconfig wlan0 up
iw wlan0 scan
wpa_passphrase SSID_NAME >> /etc/wpa_supplicant.conf
	ENTER_SSID_PASSPHRASE
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
iw wlan0 link
dhclient wlan0
```
