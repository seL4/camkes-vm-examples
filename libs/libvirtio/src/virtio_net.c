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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <autoconf.h>

#include <sel4platsupport/arch/io.h>
#include <sel4utils/vspace.h>
#include <sel4utils/iommu_dma.h>
#include <simple/simple_helpers.h>
#include <vka/capops.h>
#include <utils/util.h>

#include <sel4vmmcore/drivers/virtio_net/virtio_net.h>
#include <sel4arm-vmm/devices.h>
#include <sel4arm-vmm/devices/vpci.h>

#include <virtio/virtio.h>
#include <virtio/virtio_net.h>
#include <virtio/virtio_net_plat.h>

#include "virtio_vpci.h"

typedef struct virtio_net_cookie {
    virtio_net_t *virtio_net;
    virtio_net_callbacks_t callbacks;
    virq_handle_t virtio_net_irq_handle;
} virtio_net_cookie_t;

/* Maximum transmission unit for Ethernet interface */
#define MTU 1500

#define MAC_ADDR_B1 6
#define MAC_ADDR_B2 0
#define MAC_ADDR_B3 0
#define MAC_ADDR_B4 11
#define MAC_ADDR_B5 12
#define MAC_ADDR_B6 13

void ethdriver_mac(uint8_t *b1, uint8_t *b2, uint8_t *b3, uint8_t *b4, uint8_t *b5, uint8_t *b6)
{
    *b1 = MAC_ADDR_B1;
    *b2 = MAC_ADDR_B2;
    *b3 = MAC_ADDR_B3;
    *b4 = MAC_ADDR_B4;
    *b5 = MAC_ADDR_B5;
    *b6 = MAC_ADDR_B6;
}

static void emul_raw_handle_irq(struct eth_driver *driver, int irq)
{
    virtio_net_cookie_t *virtio_cookie = (virtio_net_cookie_t *)driver->eth_data;
    if (!virtio_cookie) {
        ZF_LOGE("NULL virtio cookie given to raw irq handler");
        return;
    }
    vm_inject_IRQ(virtio_cookie->virtio_net_irq_handle);
    if (virtio_cookie->callbacks.irq_callback) {
        virtio_cookie->callbacks.irq_callback(irq, virtio_cookie->virtio_net);
    }
}

static int emul_raw_tx(struct eth_driver *driver, unsigned int num, uintptr_t *phys, unsigned int *len, void *cookie)
{
    bool complete = true;
    int err;
    virtio_net_cookie_t *virtio_cookie = (virtio_net_cookie_t *)driver->eth_data;
    if (!virtio_cookie) {
        ZF_LOGE("NULL virtio cookie given to raw tx");
        return -1;
    }

    for (int i = 0; i < num; i++) {
        char ethbuffer[MTU];
        if (len[i] > MTU) {
            ZF_LOGE("TX data exceeds MTU (%d) - truncating remaining data", MTU);
            complete = false;
            break;
        }
        memcpy((void *)ethbuffer, (void *)phys[i], len[i]);
        if (virtio_cookie->callbacks.tx_callback) {
            err = virtio_cookie->callbacks.tx_callback(ethbuffer, len[i], virtio_cookie->virtio_net);
            if (err) {
                ZF_LOGE("TX callback failed");
                complete = false;
                break;
            }
        }
    }

    return complete ? ETHIF_TX_COMPLETE : ETHIF_TX_FAILED;
}

int virtio_net_rx(char *data, size_t length, virtio_net_t *virtio_net)
{
    unsigned int len[1];
    len[0] = length;
    void *cookie;
    void *emul_buf = (void *)virtio_net->emul_driver->i_cb.allocate_rx_buf(virtio_net->emul_driver->cb_cookie, len[0],
                                                                           &cookie);
    if (emul_buf) {
        memcpy(emul_buf, (void *)data, len[0]);
        virtio_net->emul_driver->i_cb.rx_complete(virtio_net->emul_driver->cb_cookie, 1, &cookie, len);
    }
    return 0;
}

static void emul_low_level_init(struct eth_driver *driver, uint8_t *mac, int *mtu)
{
    ethdriver_mac(&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    *mtu = MTU;
}

static void virtio_net_ack(void *token) {}

virtio_net_t *virtio_net_init(vm_t *vm, virtio_net_callbacks_t *callbacks)
{
    virtio_net_cookie_t *driver_cookie;
    virtio_net_t *virtio_net;

    struct raw_iface_funcs backend = virtio_net_default_backend();
    backend.raw_tx = emul_raw_tx;
    backend.low_level_init = emul_low_level_init;
    backend.raw_handleIRQ = emul_raw_handle_irq;

    driver_cookie = (virtio_net_cookie_t *)calloc(1, sizeof(struct virtio_net_cookie));
    if (driver_cookie == NULL) {
        ZF_LOGE("Failed to allocated virtio iface cookie");
        return NULL;
    }

    int err = install_virtio_vpci_device(vm);
    if (err) {
        ZF_LOGE("Failed to install virtio vpci device");
        return NULL;
    }

    virtio_emul_vm_t *virtio_emul_vm;
    virtio_emul_vm = (virtio_emul_vm_t *)calloc(1, sizeof(virtio_emul_vm_t));
    if (virtio_emul_vm == NULL) {
        ZF_LOGE("Failed to allocate virtio_emul_vm object");
        free(driver_cookie);
        return NULL;
    }

    virtio_emul_vm->vm = vm;
    virtio_net = common_make_virtio_net(virtio_emul_vm, vm->pci, vm->io_port, VIRTIO_IOPORT_START, VIRTIO_IOPORT_SIZE,
                                        VIRTIO_INTERRUPT_PIN, VIRTIO_NET_PLAT_INTERRUPT_LINE, backend);
    if (virtio_net == NULL) {
        ZF_LOGE("Failed to initialise virtio net driver");
        return NULL;
    }
    driver_cookie->virtio_net = virtio_net;
    driver_cookie->virtio_net_irq_handle = vm_virq_new(vm, VIRTIO_NET_PLAT_INTERRUPT_LINE, &virtio_net_ack, NULL);
    if (callbacks) {
        driver_cookie->callbacks.tx_callback = callbacks->tx_callback;
        driver_cookie->callbacks.irq_callback = callbacks->irq_callback;
    }
    virtio_net->emul_driver->eth_data = (void *)driver_cookie;
    return virtio_net;
}
