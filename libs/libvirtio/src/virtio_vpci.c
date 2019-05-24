/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#include <sel4vmmcore/drivers/virtio_net/virtio.h>
#include <sel4vmmcore/drivers/virtio_net/virtio_net.h>
#include <sel4arm-vmm/devices.h>
#include <sel4arm-vmm/devices/vpci.h>

#include <virtio/virtio.h>

#include "virtio_vpci.h"

static int width_to_size(enum fault_width fw)
{

    if (fw == WIDTH_BYTE) {
        return 1;
    } else if (fw == WIDTH_HALFWORD) {
        return 2;
    } else if (fw == WIDTH_WORD) {
        return 4;
    } else if (fw == WIDTH_DOUBLEWORD) {
        return 8;
    }
    return 0;
}


static int pci_virtio_io_fault_handler(struct device *d, vm_t *vm, fault_t *fault)
{
    uint16_t virtio_port = VIRTIO_IOPORT_START + (fault_get_address(fault) & (VIRTIO_IOPORT_SIZE - 1));
    unsigned int value = 0;
    seL4_Word fault_data = 0;

    bool is_in = false;
    if (fault_is_read(fault)) {
        is_in = true;
    } else {
        value = fault_get_data(fault);
    }
    emulate_io_handler(vm->io_port, virtio_port, is_in, width_to_size(fault_get_width(fault)), (void *)&value);

    if (is_in) {
        memcpy(&fault_data, (void *)&value, width_to_size(fault_get_width(fault)));
        /* Shift data to compensate for alignment.
         * We align the data at the correct offset in the word */
        seL4_Word shift = (fault_get_address(fault) & 0x3) * 8;
        fault_set_data(fault, fault_data << shift);
    }

    return advance_fault(fault);
}

const struct device dev_vpci_virtio_io = {
    .devid = DEV_CUSTOM,
    .name = "vpci.virtio_io",
    .pstart = PCI_IO_REGION_ADDR + PCI_IO_REGION_SIZE,
    .size = VIRTIO_IOPORT_SIZE,
    .handle_page_fault = &pci_virtio_io_fault_handler,
    .priv = NULL,
};

int install_virtio_vpci_device(vm_t *vm)
{
    int err = vm_add_device(vm, &dev_vpci_virtio_io);
    if (err) {
        ZF_LOGE("Failed to install Virtio PCI IOPort device");
    }
    return err;
}
