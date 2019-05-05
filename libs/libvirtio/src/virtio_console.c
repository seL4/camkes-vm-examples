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

#include <sel4vmmcore/drivers/virtio_console/virtio_con.h>
#include <sel4vm/devices.h>
#include <sel4vm/devices/vpci.h>

#include <virtio/virtio.h>
#include <virtio/virtio_plat.h>
#include <virtio/virtio_console.h>

#include "virtio_vpci.h"

typedef struct virtio_con_cookie {
    virtio_con_t *virtio_con;
    virq_handle_t virtio_con_irq_handle;
} virtio_con_cookie_t;

static void virtio_console_ack(void *token) {}

static void console_handle_irq(void *cookie)
{
    virtio_con_cookie_t *virtio_cookie = (virtio_con_cookie_t *)cookie;
    if (!virtio_cookie) {
        ZF_LOGE("NULL virtio cookie given to raw irq handler");
        return;
    }
    vm_inject_IRQ(virtio_cookie->virtio_con_irq_handle);
}

virtio_con_t *virtio_console_init(vm_t *vm, console_putchar_fn_t putchar)
{

    int err;
    struct console_passthrough backend;
    virtio_con_cookie_t *console_cookie;
    virtio_con_t *virtio_con;

    backend.handleIRQ = console_handle_irq;
    backend.putchar = putchar;

    console_cookie = (virtio_con_cookie_t *)calloc(1, sizeof(struct virtio_con_cookie));
    if (console_cookie == NULL) {
        ZF_LOGE("Failed to allocated virtio console cookie");
        return NULL;
    }

    virtio_emul_vm_t *virtio_emul_vm;
    virtio_emul_vm = (virtio_emul_vm_t *)calloc(1, sizeof(virtio_emul_vm_t));
    if (virtio_emul_vm == NULL) {
        ZF_LOGE("Failed to allocate virtio_emul_vm object");
        free(console_cookie);
        return NULL;
    }

    err = install_virtio_vpci_device(vm);
    if (err) {
        ZF_LOGE("Failed to install virtio vpci device");
        free(console_cookie);
        free(virtio_emul_vm);
        return NULL;
    }

    virtio_emul_vm->vm = vm;

    backend.console_data = (void *)console_cookie;
    virtio_con = common_make_virtio_con(virtio_emul_vm, vm->arch.pci, vm->arch.io_port, VIRTIO_IOPORT_START, VIRTIO_IOPORT_SIZE,
                                        VIRTIO_INTERRUPT_PIN, VIRTIO_CON_PLAT_INTERRUPT_LINE, backend);
    console_cookie->virtio_con = virtio_con;
    console_cookie->virtio_con_irq_handle = vm_virq_new(vm, VIRTIO_CON_PLAT_INTERRUPT_LINE, &virtio_console_ack, NULL);
    return virtio_con;
}
