<!--
     Copyright 2018, Data61
     Commonwealth Scientific and Industrial Research Organisation (CSIRO)
     ABN 41 687 119 230.

     This software may be distributed and modified according to the terms of
     the BSD 2-Clause license. Note that NO WARRANTY is provided.
     See "LICENSE_BSD2.txt" for details.

     @TAG(DATA61_BSD)
-->
# camkes-vm-examples

This repo contains a collection of example CAmkES applications that use the CAmkES VM project. This is a short description of the example applications and how to build them.

For detailed information about the CAmkES VM project and its dependencies see the documentation in [the camkes-vm repo](https://github.com/seL4/camkes-vm).

## Building

To setup the camkes-vm-examples project we can use the repo tool to fetch the latest [camkes-vm-examples-manifest](https://github.com/seL4/camkes-vm-examples):

	# Create a directory for the project
	mkdir camkes_vm_examples
	cd camkes_vm_examples
	# Fetch the project sources
	repo init -u https://github.com/seL4/camkes-vm-examples-manifest.git
	repo sync

To build an example VM application with CMake

    # From the project root create a new folder to compile the project
    mkdir build_vm
    cd build_vm

    # Invoke CMake using the shell script wrapper `init-build.sh` located in the root directory, passing the application you wish to compile as a command-line argument
    # If no application is passed the optiplex9020 app will be built by default
    ../init-build.sh -DCAMKES_VM_APP=<APPLICATION_NAME>

    # Invoke ninja to compile the example vm application
    ninja

Then boot images/kernel-x86_64-pc99 and images/capdl-loader-experimental-image-x86_64-pc99 (or \*.ia32-pc99 if built for 32-bit) with the multiboot boot loader of your choice.

## Configuration

VM example applications are provied in this repo. The following is a short description of each vm application.

### minimal

The `minimal` application is simple CAmkES VM application. The application is configured with:

- *1 Guest Linux VM:* The Linux guest is a [buildroot](https://buildroot.org/) built image sourced from the [the camkes-vm repo](https://github.com/seL4/camkes-vm). The Linux images (rootfs and kernel) are defined in the `CMakeLists.txt` file of the `minimal` application. In the `CMakeLists.txt` file we are able to find that the rootfs and kernel images  are added to the `FileServer` under the names `"rootfs.cpio"` and  `"bzimage"` respectively.

### optiplex9020

The `optiplex9020` VM application is configured with:

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

### cma34cr\_centos

The `cma34cr_centos` application is a more complex CAmkES VM configuration demonstrating the use of passthrough hardware. The `cma34cr_centos` application is configured with:

- *1 Guest Linux VM:* The Linux guest images (`bzimage` and `roofs.cpio`) are located in the applications directory (`cma34cr_centos/centos_linux`), originally sourced from an i386 altarch CentOS-7 installation. Additionally the CentosOS installation should be on a flash drive passed-through to the `cma34cr` application. Further information regarding the Linux installation can be found in the applications [README](https://github.com/seL4/camkes-vm-examples/cma34cr_centos/README.md).
- *Cross VM Connectors:* A series of shared dataports and events are established between `vm0` and the `StringReverse` component.
- *Ethernet Driver, UDPSever, Echo, Firewall:* A passthrough ethernet configuration demo. The guest VM is configured to use the Ethernet driver component through a virtio configuration.
- *Passthrough hardware storage (SATA/USB)*: A hardware configuration to boot the CentOS installation.

### zmq_samples

The `zmq_samples` application demonstrates messaging between VMs using the ZeroMQ messaging library.
See the README.md in its folder for more information.
