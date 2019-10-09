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

#include <sel4vmmplatsupport/drivers/virtio.h>
#include <sel4vmmplatsupport/drivers/virtio_net.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/vpci.h>

#include <virtio/virtio.h>

#include "virtio_vpci.h"

typedef struct private_data {
    vmm_pci_space_t *pci;
    vmm_io_port_list_t *io_ports;
} private_data_t;

static memory_fault_result_t
pci_virtio_io_fault_handler(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length, void *cookie)
{
    uint16_t virtio_port = VIRTIO_IOPORT_START + (fault_addr & (VIRTIO_IOPORT_SIZE - 1));
    unsigned int value = 0;
    seL4_Word fault_data = 0;

    bool is_in = false;
    if (is_vcpu_read_fault(vcpu)) {
        is_in = true;
    } else {
        value = get_vcpu_fault_data(vcpu);
    }
    struct device *d = cookie;
    private_data_t *data = d->priv;
    emulate_io_handler(data->io_ports, virtio_port, is_in, fault_length, (void *)&value);

    if (is_in) {
        memcpy(&fault_data, (void *)&value, fault_length);
        /* Shift data to compensate for alignment.
         * We align the data at the correct offset in the word */
        seL4_Word shift = (fault_addr & 0x3) * 8;
        set_vcpu_fault_data(vcpu, fault_data << shift);
    }

    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}

const struct device dev_vpci_virtio_io = {
    .name = "vpci.virtio_io",
    .pstart = PCI_IO_REGION_ADDR + PCI_IO_REGION_SIZE,
    .size = VIRTIO_IOPORT_SIZE,
    .priv = NULL,
};

int install_virtio_vpci_device(vm_t *vm, vmm_pci_space_t *pci, vmm_io_port_list_t *io_ports)
{
    private_data_t *data = calloc(1, sizeof(*data));
    if (data == NULL) {
        ZF_LOGE("Failed to allocate private_data_t object");
        return -1;
    }

    data->pci = pci;
    data->io_ports = io_ports;
    struct device *vpci_dev = calloc(1, sizeof(*vpci_dev));
    if (vpci_dev == NULL) {
        ZF_LOGE("Failed to allocate vpci_dev object");
        return -1;
    }
    *vpci_dev = dev_vpci_virtio_io;
    vpci_dev->priv = data;

    vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, vpci_dev->pstart, vpci_dev->size,
            pci_virtio_io_fault_handler, (void *)vpci_dev);
    if (!reservation) {
        return -1;
    }
    return 0;
}
