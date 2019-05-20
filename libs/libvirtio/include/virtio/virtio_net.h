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

#include <sel4vmmcore/drivers/virtio_net/virtio_net.h>

typedef struct virtio_net_callbacks {
    int (*tx_callback)(char *data, size_t length, virtio_net_t *virtio_net);
    void (*irq_callback)(int irq, virtio_net_t *virtio_net);
} virtio_net_callbacks_t;

int virtio_net_rx(char *data, size_t length, virtio_net_t *virtio_net);

virtio_net_t *virtio_net_init(vm_t *vm, virtio_net_callbacks_t *callbacks);
