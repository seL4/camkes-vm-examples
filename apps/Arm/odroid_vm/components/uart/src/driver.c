/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdio.h>

#include <platsupport/chardev.h>
#include <platsupport/serial.h>
#include <utils/util.h>
#include <camkes.h>

//#define BAUD_RATE 115200
#define BAUD_RATE 57600

#ifdef CONFIG_PLAT_EXYNOS5410
#define DEV_ID  PS_SERIAL1
#elif CONFIG_PLAT_IMX31
#define DEV_ID  IMX31_UART1
#else
#error
#endif

struct uart_token {
    size_t cur_bytes;
    size_t req_bytes;
    char* buf;
};

static ps_chardevice_t serial_device;

void
interrupt_handle(void)
{
    ps_chardevice_t* device;
    ps_cdev_handle_irq(&serial_device, 0);
    interrupt_acknowledge();
}

/* UART 1 dummy input clock */
static freq_t
dummy_get_freq(clk_t* clk) {
    return 24000000;
}

static freq_t
dummy_set_freq(clk_t* clk, UNUSED freq_t hz){
    return dummy_get_freq(clk);
}

#define DUMMY_CLK(cust_id)              \
    {                                   \
        .id = cust_id,                  \
        .name = "DUMMY " #cust_id,      \
        .priv = NULL,                   \
        .req_freq = 0,                  \
        .parent = NULL,                 \
        .sibling = NULL,                \
        .child = NULL,                  \
        .clk_sys = NULL,                \
        .init = NULL,                   \
        .get_freq = &dummy_get_freq,    \
        .set_freq = &dummy_set_freq,    \
        .recal = NULL                   \
    }

#if DEV_ID == 1
clk_t uart_dummy_clk = DUMMY_CLK(CLK_UART1);
#else
clk_t uart_dummy_clk = DUMMY_CLK(CLK_UART3);
#endif

void uart__init(void)
{
    /* Iniitialise the UART */
    printf("Initialising UART driver\n");
    if(exynos_serial_init(DEV_ID, uart0base, NULL, &uart_dummy_clk,
            &serial_device)){
        printf("Failed to initialise UART\n");
        while(1);
    }
    ps_cdev_set_flags(&serial_device, serial_device.flags & ~SERIAL_AUTO_CR);
    serial_configure(&serial_device, BAUD_RATE, 8, PARITY_NONE, 1);
    /* Prime semaphores */
    read_sem_wait();
    write_sem_wait();
}


static void
read_callback(ps_chardevice_t* device, enum chardev_status stat,
              size_t bytes_transfered, void* token){
    struct uart_token* t;
    t = (struct uart_token*)token;
    /* We might get a short read due to a timeout. */
    t->cur_bytes += bytes_transfered;
    t->buf += bytes_transfered;
    if(t->cur_bytes < t->req_bytes){
        int ret;
        ret = ps_cdev_read(device, t->buf, t->req_bytes - t->cur_bytes,
                           &read_callback, token);
        if(ret < 0){
            printf("Error reading from UART\n");
            read_sem_post();
        }
    }else{
        read_sem_post();
    }
}

static void
write_callback(ps_chardevice_t* device, enum chardev_status stat,
              size_t bytes_transfered, void* token){
    struct uart_token* t = token;
    t->cur_bytes += bytes_transfered;
    if(t->cur_bytes == t->req_bytes){
        write_sem_post();
    }
}


int uart_read(int uart_num, char *c, int rsize)
{
    struct uart_token token;
    if (uart_num != 0) {
        printf("Only support UART0!\n");
        return -1;
    }

    token.cur_bytes = 0;
    token.req_bytes = rsize;
    token.buf = c;
    if(ps_cdev_read(&serial_device, token.buf, token.req_bytes, &read_callback, &token) < 0){
        printf("Error reading from UART\n");
        return -1;
    }
    read_sem_wait();

    return token.cur_bytes;
}

int uart_write(int uart_num, int wsize)
{
    struct uart_token token;
    if (uart_num != 0) {
        printf("Only support UART0!\n");
        return -1;
    }

    token.cur_bytes = 0;
    token.req_bytes = wsize;
    token.buf = (char*)client_buf;

    if(ps_cdev_write(&serial_device, token.buf, token.req_bytes, &write_callback, &token) < 0){
        printf("Error writing to UART\n");
        return -1;
    }
    write_sem_wait();

    return wsize;
}

int run(void)
{
	char c;

	while (1) {
		uart_read(0, &c, 1);
		pilot_recv(ID, (int)c);
	}

	return 0;
}
