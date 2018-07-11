<!--
  Copyright 2018, Data61
  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
  ABN 41 687 119 230.

  This software may be distributed and modified according to the terms of
  the GNU General Public License version 2. Note that NO WARRANTY is provided.
  See "LICENSE_GPLv2.txt" for details.

  @TAG(DATA61_GPL)
-->

UbuntuOS Guest
============

Installation
------------

This example has been tested by booting Ubuntu 18.04 LTS from a USB flash drive connected to the
cma34cr. Ubuntu can also be booted from the flash drive of the cma34cr by using the entire drive,
i.e. no dual booting etc. See the centos example for the appropriate hardware passthrough configuration.

Kernel Image
------------

The roofs.cpio in this directory is built from a Ubuntu 18.04 installation. This was generated as per
the instructions found [here](https://docs.sel4.systems/CAmkESVM.html#booting-from-hard-drive)

Kernel Command Line
-------------------

Assuming a default installation of Ubuntu the command line passed to the Linux kernel.
In the case where you provided a custom name for your root partitions, or if
something changes in later Ubuntu versions, then this needs to be updated from the
command line in the grub configuration of your actual installation

Booting the VMM
--------------

After building you will have two files in the images/ directory, a kernel- and a capdl-.
These need to be booted with a multiboot compatible loader, with the capdl- image passed
as a boot module. This can be done with with a PXE based network loader, or by adding to
the grub menu of the installed Ubuntu distro.

Hardware Configuration
----------------------

The hardware configuration for this example is minimal. This includes the passing of
cma34cr USB hardware. For details on passing through ethernet and SATA hardware see the
centos example.
