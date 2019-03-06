/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <autoconf.h>
#include <vmlinux.h>

#include <sel4platsupport/arch/io.h>
#include <sel4utils/vspace.h>
#include <sel4utils/iommu_dma.h>
#include <simple/simple_helpers.h>
#include <vka/capops.h>
#include <utils/util.h>

#include <camkes.h>
#include <camkes/dataport.h>

#include <sel4vmm-drivers/virtio/virtio_net.h>
#include <sel4arm-vmm/devices.h>
#include <sel4arm-vmm/devices/vpci.h>

#include <plat/virtio_net.h>

#include "virtio_net.h"
#include "virtio_net_vpci.h"
#include "virtio_net_arping.h"

static virtio_net_t *virtio_net = NULL;
static virtio_emul_vm_t virtio_emul_vm;
static virq_handle_t virtio_net_irq_handle;

volatile Buf*__attribute__((weak)) ethdriver_buf;

/* Maximum transmission unit for Ethernet interface */
#define MTU 1500

int __attribute__((weak)) ethdriver_tx(int len) {
    ZF_LOGF("should not be here");
    return 0;
}

int __attribute__((weak)) ethdriver_rx(int *len) {
    ZF_LOGF("should not be here");
    return 0;
}

void ethdriver_mac(uint8_t *b1, uint8_t *b2, uint8_t *b3, uint8_t *b4, uint8_t *b5, uint8_t *b6) {
    *b1 = 6; *b2 = 0; *b3 = 0; *b4 = 11; *b5 = 12; *b6 = 13;
}

int __attribute__((weak)) eth_rx_ready_reg_callback(void (*proc)(void*),void *blah) {
    ZF_LOGF("should not be here");
    return 0;
}

static void emul_raw_handle_irq(struct eth_driver *driver, int irq) {
    vm_inject_IRQ(virtio_net_irq_handle);
}

static int emul_raw_tx(struct eth_driver *driver, unsigned int num, uintptr_t *phys, unsigned int *len, void *cookie) {
    size_t tot_len = 0;
    char ethbuffer[MTU];
    for (int i = 0; i < num; i++) {
        memcpy((void *)ethbuffer + tot_len, (void *)phys[i], len[i]);
        tot_len += len[i];
        if (tot_len > MTU) {
            ZF_LOGE("TX data exceeds MTU (%d) - Any remaining data will be lost", MTU);
            break;
        }
    }

    arping_reply(ethbuffer, virtio_net);
    return ETHIF_TX_COMPLETE;
}


static void emul_low_level_init(struct eth_driver *driver, uint8_t *mac, int *mtu) {
    ethdriver_mac(&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    *mtu = MTU;
}

static void virtio_net_ack(void* token) {}

void make_virtio_net(vm_t *vm, void* cookie) {
    struct raw_iface_funcs backend = virtio_net_default_backend();
    backend.raw_tx = emul_raw_tx;
    backend.low_level_init = emul_low_level_init;
    backend.raw_handleIRQ = emul_raw_handle_irq;
    int err = install_virtio_net_device(vm);
    assert(!err);
    virtio_emul_vm.vm = vm;
    virtio_net = common_make_virtio_net(&virtio_emul_vm, &vm->pci, &vm->io_port, VIRTIO_NET_IOPORT_START, VIRTIO_NET_IOPORT_SIZE,
            VIRTIO_NET_INTERRUPT_PIN, VIRTIO_NET_PLAT_INTERRUPT_LINE ,backend);
    assert(virtio_net);
    virtio_net_irq_handle = vm_virq_new(vm, VIRTIO_NET_PLAT_INTERRUPT_LINE, &virtio_net_ack, NULL);
}

DEFINE_MODULE(virtio_net, NULL, make_virtio_net)
