/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */

#pragma once

#include <sel4arm-vmm/vm.h>
#include <sel4arm-vmm/devices.h>

#include <sel4pci/virtio_emul.h>

#define VIRTIO_NET_IOPORT_SIZE      0x400
#define VIRTIO_NET_IOPORT_START     0x6200
#define VIRTIO_NET_INTERRUPT_PIN    1
