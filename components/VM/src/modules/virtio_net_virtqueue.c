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
#include <arm_vm/gen_config.h>
#include <vmlinux.h>

#include <camkes.h>

#include <virtqueue.h>
#include <camkes/virtqueue.h>
#include <virtio/virtio_net.h>

#include "virtio_net_virtqueue.h"

static virtio_net_t *virtio_net = NULL;

virtqueue_device_t recv_virtqueue;
virtqueue_driver_t send_virtqueue;

static int tx_virtqueue_forward(char *eth_buffer, size_t length, virtio_net_t *virtio_net)
{
    volatile void *buf = NULL;
    int err = camkes_virtqueue_buffer_alloc(&send_virtqueue, &buf, length);
    if (err) {
        return -1;
    }

    memcpy(buf, (void *)eth_buffer, length);

    if (camkes_virtqueue_driver_send_buffer(&send_virtqueue, buf, length) != 0) {
        camkes_virtqueue_buffer_free(&send_virtqueue, buf);
        return -1;
    }
    send_virtqueue.notify();
    return 0;
}

static void virtio_net_notify_free_send(void)
{
    void *buf = NULL;
    size_t buf_size = 0, wr_len = 0;
    vq_flags_t flag;
    virtqueue_ring_object_t handle;
    if (!virtqueue_get_used_buf(&send_virtqueue, &handle, &wr_len)) {
        ZF_LOGE("Client virtqueue dequeue failed");
        return;
    }
    while (camkes_virtqueue_driver_gather_buffer(&send_virtqueue, &handle, &buf, &buf_size, &flag)) {
        /* Clean up and free the buffer we allocated */
        camkes_virtqueue_buffer_free(&send_virtqueue, buf);
    }
}

static int virtio_net_notify_handle_recv(void)
{

    volatile void *buf = NULL;
    size_t buf_size = 0;
    vq_flags_t flag;
    virtqueue_ring_object_t handle;
    if (!virtqueue_get_available_buf(&recv_virtqueue, &handle)) {
        ZF_LOGE("Client virtqueue dequeue failed");
        return -1;
    }

    while (camkes_virtqueue_device_gather_buffer(&recv_virtqueue, &handle, &buf, &buf_size, &flag)) {
        int err = virtio_net_rx((char *) buf, buf_size, virtio_net);
        if (err) {
            ZF_LOGE("Unable to forward recieved buffer to the guest");
        }
    }

    if (!virtqueue_add_used_buf(&recv_virtqueue, &handle, 0)) {
        ZF_LOGE("Unable to enqueue used recv buffer");
        return -1;
    }

    recv_virtqueue.notify();
}

void virtio_net_notify(vm_t *vm)
{
    if (VQ_DEV_POLL(&recv_virtqueue)) {
        int err = virtio_net_notify_handle_recv();
        if (err) {
            ZF_LOGE("Failed to handle virtio net recv");
        }
    }

    if (VQ_DRV_POLL(&send_virtqueue)) {
        virtio_net_notify_free_send();
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
