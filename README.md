<!--
  Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)

  SPDX-License-Identifier: CC-BY-SA-4.0
-->

# camkes-vm-apps

This project is for running virtualised Linux guests on seL4 for ARM and x86 platforms. The `camkes-vm` implements
a virtual machine monitor (VMM) server, faciliating the intialisation, booting and run-time management of a guest OS. 
You can view the code for the VMMs in the `camkes-vm` repository under `VM_Arm` and `VM`. 

Currently the supported platforms include:
* Exynos5 (exynos5410, exynos5422)
* TK1
* TX1
* TX2
* QEMU ARM virt machine
* x86
* x86_64 (coming)

## Getting and Building
The following example builds the camkes arm vmm for the TK1.
```bash
repo init -u https://github.com/SEL4PROJ/camkes-vm-manifest.git
repo sync
mkdir build
cd build
../init-build.sh -DCAMKES_VM_APP=vm_minimal -DPLATFORM=tk1
ninja
```
*Note: To buid for another platform you can substitute the value of the `-DPLATFORM` variable e.g. (exynos5422, tx1, tx2, qemu-arm-virt)*
*Note: If building for x86 you don't need to specify the `-DPLATFORM` variable*

### For Arm
An EFI application file will be left in `images/capdl-loader-image-arm-tk1` We normally boot using TFTP, by first copying `capdl-loader-image-arm-tk1` to a tftpserver then on the U-Boot serial console doing:
```bash
dhcp tftpboot $loadaddr
/capdl-loader-image-arm-tk1
bootefi ${loadaddr}
```

### For x86
Boot images/kernel-x86_64-pc99 and images/capdl-loader-experimental-image-x86_64-pc99 (or \*.ia32-pc99 if built for 32-bit) with the multiboot boot loader of your choice.

## CAmkES ARM VMM Applications
This project contains various reference CAmkES applications that incorporate the VMM component. These applications include:
* `vm_minimal`: A simple VMM application, launching a single guest Linux with a buildroot RAM-based file system
* `vm_cross_connector`: Application demonstrating communication between a Linux guest user-level process and a native seL4 component. This leveraging
cross vm communication channels established between the VMM and the seL4 component.
* `vm_multi`: Application demonstrating running multiple guest VM's. This is achieved through having multiple VMM components, each managing a single
Linux guest (1-1 model).
* `vm_serial_server`: Application demonstrating the use of Virtio Console, where a VM's serial I/O is forwarded to/from a serial server.
* `vm_virtio_net`: Application demonstrating the use of Virtio Net, where a virtual network interface is presented to a VM and subsequent network
traffic on the virtual network interface is sent to/from other native seL4 components.

See the `apps/Arm/` subdirectory for all the supported virtual machine manager apps, their implementations and the platforms they target.

## CAmkES x86 VMM Applications

* The `minimal` application is simple CAmkES VM application. The application is configured with:

- *1 Guest Linux VM:* The Linux guest is a [buildroot](https://buildroot.org/) built image sourced from the [the camkes-vm repo](https://github.com/seL4/camkes-vm). The Linux images (rootfs and kernel) are defined in the `CMakeLists.txt` file of the `minimal` application. In the `CMakeLists.txt` file we are able to find that the rootfs and kernel images  are added to the `FileServer` under the names `"rootfs.cpio"` and  `"bzimage"` respectively.

* The `optiplex9020` VM application is configured with:

- *2 Guest Linux VM's:* The Linux guests are [buildroot](https://buildroot.org/) built images sourced from the [the camkes-vm repo](https://github.com/seL4/camkes-vm).
-  *Cross VM Connectors:* To demo the use of cross-vm connectors a series of shared dataports and events are established between `vm0` and the `StringReverse` component. The `StringReverse` demo can be invoked from the command-line of `vm0`:
    ```
    Welcome to Buildroot
    buildroot login: root
    Password:
    \# string_reverse
    [   19.739028] dataport received mmap for minor 1
    [   19.743337] dataport received mmap for minor 2
    hello
    olleh
    ```

* The `cma34cr_centos` application is a more complex CAmkES VM configuration demonstrating the use of passthrough hardware. The `cma34cr_centos` application is configured with:

- *1 Guest Linux VM:* The Linux guest images (`bzimage` and `roofs.cpio`) are located in the applications directory (`cma34cr_centos/centos_linux`), originally sourced from an i386 altarch CentOS-7 installation. Additionally the CentosOS installation should be on a flash drive passed-through to the `cma34cr` application. Further information regarding the Linux installation can be found in the applications [README](https://github.com/SEL4PROJ/camkes-vm-apps/apps/x86/cma34cr_centos/README.md).
- *Cross VM Connectors:* A series of shared dataports and events are established between `vm0` and the `StringReverse` component.
- *Ethernet Driver, UDPSever, Echo, Firewall:* A passthrough ethernet configuration demo. The guest VM is configured to use the Ethernet driver component through a virtio configuration.
- *Passthrough hardware storage (SATA/USB)*: A hardware configuration to boot the CentOS installation.

* The `zmq_samples` application demonstrates messaging between VMs using the ZeroMQ messaging library.
See the README.md in its folder for more information.

See the `apps/x86/` subdirectory for all the supported virtual machine manager apps, their implementations and the platforms they target.

## Arm Features
See the below feature matrix for the various features the CAmkES ARM VMM implements/facilitates across the various supported platforms.

| Plaform | Guest Mode | SMP Support | Multiple VM Support | Virtio PCI | Virtio Console | Virtio Net | Cross VM Connector Support | *Notes* |
|:----------:|:----------:|:-----------:|:-------------------:|:-----------:|:--------------:|:-----------:|:--------------------------:|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|
| exynos5422 | 32-bit  | Unsupported | Supported | Supported | Supported | Supported | Supported | SMP configurations are unsupported due to: * No exynos5422 kernel SMP support * No virtual power device interface to manage VCPU's at runtime (e.g. core startup) |
| TK1 | 32-bit | Unsupported | Unsupported | Unsupported | Unsupported | Unsupported | Unsupported | SMP configurations are unsupported due to: * No TK1 kernel SMP support * No virtual power device interface to manage VCPU's at runtime (e.g. core startup) Virtio PCI, Console, Net, Cross VM connector support & Multi-VM are untested  |
| TX1 | 64-bit | Supported | Unsupported | Unsupported | Unsupported | Unsupported | Unsupported | Virtio PCI, Console, Net, Cross VM connector support & Multi-VM are untested |
| TX2 | 64-bit | Supported | Supported | Supported | Supported | Supported | Unsupported | Cross VM connector support is untested |
| QEMU Virt | 64-bit | Supported | Unsupported | Supported | Supported | Supported | Supported | Multi-VM support depends on porting the TimeServer to QEMU (See https://github.com/SEL4PROJ/global-components/tree/master/components/TimeServer) |


## Arm Platform Configuration Notes

### Exynos5422, TX1, TX2, QEMU ARM Virt configuration
We provide a pre-built Linux image and Buildroot image for our guest VM's. See the images in the `camkes-vm-images` repository @ https://github.com/SEL4PROJ/camkes-vm-images
When compiling an application for these platforms, the images are sourced from the platforms subdirectory in the `camkes-vm-images` repo.
Feel free also to compile your own Linux and Rootfs images, see the README's in each platform subdirectory (within `camkes-vm-images`) for information about our
image build configurations.

### TK1 configuration

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

#### Wireless Configuration
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

### TK1 Build Notes
 The default setup does not pass though many devices to the Linux kernel. If you `make menuconfig` you can set `insecure` mode in the `Applications` submenu; this is meant to pass through all devices, but not
everything has been tested and confirmed to work yet. In particular, the SMMU needs to have extra entries added for any DMA-capable devices such as SATA.

## Arm FAQ and Implementation Notes

#### How do I update camkes-vm-apps to support platform 'X'
Whilst large parts of the ARM VMM implementation tries to be platform agnostic, there are a few things to keep in mind and implement in order to correctly run the camkes-arm-vm on your new target platform.
See the below list as as a rough list of items to check off:
* Ensure the platform is already supported by seL4 (or you have developed support for the platform prior). See the [following](https://docs.sel4.systems/Hardware/) page for a list of supported platforms.
* The platform supports ARM's hardware virtualisation features, these being found on ARMv7 (with virtualisation extensions) and ARMv8.
    * Second to this, the seL4 port to the given platform also supports running with `KernelArmHypervisorSupport`
* Does your platform use a GICv2 or GICv3? *Note: We currently only support a virtual GICV2, with virtual GICV3 support under-development. See the listed features of libsel4vm for further information: https://github.com/SEL4PROJ/seL4_projects_libs/tree/master/libsel4vm*
* When porting a VMM application to your platform, say `vm_minimal`, ensure you provide the following:
    * A `devices.camkes` file for your platform. These containing platform specific definitions, see in particular the vm `dtb` field and the `untyped_mmios` field (for device passthrough). The values in the aforementioned fields corresponding with those found in the kernel device tree.
    * A `vmlinux.h` header for you platform. See the header file for the other supported platforms at `components/VM_Arm/plat_include` in `camkes-vm` repository.
* Provide a pre-compiled Linux kernel and initrd image for your platform. Once compiled these are usually linked in through your apps `CMakeLists.txt`. See `apps/Arm/vm_minimal/CMakeLists.txt` for examples of how other platforms link in their images.

Feel free to contact the [team](https://docs.sel4.systems/processes/#contact) for further support on porting the camkes-arm-vm to your desired platform. We are also always open to contributions for new platforms.

#### Can a single VMM component support running multiple VM's?
Currently, No. A VMM component only creates and manages a single VM instance.
To support multiple VM's, you can run multiple VMM components, each managing its own VM. See `apps/vm_multi` for an example of this configuration.

#### How do I enable SMP support for a VM instance?
To configure your VM with multiple VCPU's, you can set the VM instance attribute `num_vcpus` in your camkes configuration. See [here](https://github.com/SEL4PROJ/camkes-vm-apps/blob/master/apps/Arm/vm_minimal/tx2/devices.camkes#L40) for an example.
In addition, when initialising your build, ensure the `KernelMaxNumNodes` configuration option is set to your desired value. You can also initialise your build with variable `-DNUM_NODES` variable e.g.
```bash
../init-build.sh -DCAMKES_VM_APP=vm_minimal -DPLATFORM=tx2 -DNUM_NODES=4
```
*Note: ensure your platform supports SMP configurations through the above feature matrix*
