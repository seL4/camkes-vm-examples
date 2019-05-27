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
#include <vmlinux.h>

#include <camkes.h>

#include <virtqueue.h>
#include <camkes/virtqueue.h>
#include <virtio/virtio_net.h>

#include "virtio_net_virtqueue.h"

static virtio_net_t *virtio_net = NULL;

virtqueue_device_t *recv_virtqueue;
virtqueue_driver_t *send_virtqueue;

static int tx_virtqueue_forward(char *eth_buffer, size_t length, virtio_net_t *virtio_net)
{
    volatile void *alloc_buffer = NULL;
    int err = camkes_virtqueue_buffer_alloc(send_virtqueue, &alloc_buffer, length);
    if (err) {
        ZF_LOGE("Unable to allocate virtqueue buffer");
        return -1;
    }
    memcpy((void *)alloc_buffer, (void *)eth_buffer, length);

    err = virtqueue_driver_enqueue(send_virtqueue, alloc_buffer, length);
    if (err) {
        ZF_LOGE("Unknown error while enqueuing available buffer");
        camkes_virtqueue_buffer_free(send_virtqueue, alloc_buffer);
        return -1;
    }

    err = virtqueue_driver_signal(send_virtqueue);
    if (err != 0) {
        ZF_LOGE("Unknown error while signaling sending virtqueue");
    }

    return err;
}

static void virtio_net_notify_free_send(void)
{
    volatile void *used_buf = NULL;
    size_t used_buf_sz = 0;
    int err = virtqueue_driver_dequeue(send_virtqueue,
                                       &used_buf,
                                       &used_buf_sz);
    if (err) {
        ZF_LOGE("Unable to dequeue used buff");
        return;
    }
    camkes_virtqueue_buffer_free(send_virtqueue, used_buf);
}

static int virtio_net_notify_handle_recv(void)
{
    volatile void *available_buff = NULL;
    size_t available_buff_sz = 0;
    int err = virtqueue_device_dequeue(recv_virtqueue, &available_buff, &available_buff_sz);
    if (err) {
        ZF_LOGE("Unable to dequeue recv virtqueue");
        return -1;
    }
    err = virtio_net_rx((char *)available_buff, available_buff_sz, virtio_net);
    if (err) {
        ZF_LOGE("Unable to forward recieved buffer to the guest");
    }

    virtqueue_device_enqueue(recv_virtqueue, available_buff, available_buff_sz);
}

void virtio_net_notify(vm_t *vm)
{
    int err;
    if (virtqueue_driver_poll(send_virtqueue) == 1) {
        virtio_net_notify_free_send();
    }
    if (virtqueue_device_poll(recv_virtqueue) == 1) {
        err = virtio_net_notify_handle_recv();
        if ("Failed to handle virtio net recv");
    }
}

void make_virtqueue_virtio_net(vm_t *vm, void *cookie)
{
    virtio_net_callbacks_t callbacks;
    callbacks.tx_callback = tx_virtqueue_forward;
    callbacks.irq_callback = NULL;
    virtio_net = virtio_net_init(vm, &callbacks);

    /* Initialise recv virtqueue */
    int err = camkes_virtqueue_device_init(&recv_virtqueue, 0);
    if (err) {
        ZF_LOGF("Unable to initialise recv virtqueue");
    }

    /* Initialise send virtqueue */
    err = camkes_virtqueue_driver_init(&send_virtqueue, 1);
    if (err) {
        ZF_LOGF("Unable to initialise send virtqueue");
    }

}

DEFINE_MODULE(virtio_net, NULL, make_virtqueue_virtio_net)
