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

#include <utils/util.h>

#include <camkes.h>
#include <camkes/dataport.h>

#include <sel4vmmcore/drivers/virtio_console/virtio_con.h>
#include <sel4arm-vmm/devices.h>
#include <sel4arm-vmm/devices/vpci.h>

#include <platsupport/serial.h>
#include <virtio/virtio_console.h>

static virtio_con_t *virtio_con = NULL;
extern void *serial_getchar_buf;

//this matches the size of the buffer in serial server
#define BUFSIZE 4088

char txbuf[BUFSIZE];
void handle_serial_console()
{
    struct {
        uint32_t head;
        uint32_t tail;
        char buf[BUFSIZE];
    } volatile *buffer = serial_getchar_buf;

    int count = 0;
    while (buffer->head != buffer->tail) {
        txbuf[count % BUFSIZE] = buffer->buf[buffer->head];
        buffer->head = (buffer->head + 1) % BUFSIZE;
        count += 1 % BUFSIZE;
    }
    virtio_console_putchar(virtio_con->emul, txbuf, count);
}

static void emulate_console_putchar(char c)
{
    putchar_putchar(c);
}

void make_virtio_con(vm_t *vm, void *cookie)
{
    virtio_con = virtio_console_init(vm, emulate_console_putchar);
    if (!virtio_con) {
        ZF_LOGF("Failed to initialise virtio console");
    }
}

DEFINE_MODULE(virtio_con, NULL, make_virtio_con)
