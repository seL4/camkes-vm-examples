/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <spi_inf.h>
#include <platsupport/spi.h>
#include <platsupport/gpio.h>
#include <utils.h>
#include <string.h>
#include <camkes.h>
#include <errno.h>

#include <common.h>
#include <utils/util.h>

#define SPI_PORT          SPI1

#define SPI_CS_RELEASE 1
#define SPI_CS_ASSERT  0

struct spi_slave {
    int id;
    spi_dev_port_p* port;
    void(*cs)(int state);
    spi_slave_config_t cfg;
};

/**
 * Slave parameters. An ID in the table should match the id of an incoming request 
 */
#define SLAVE_PARAMS(i, p, s, d, f, g)  \
        {                               \
            .id = i,                    \
            .port = (spi_dev_port_p*)p, \
	    .cs = g,                    \
            .cfg = {                    \
                .speed_hz = s,          \
                .nss_udelay = d,        \
	        .fb_delay = f           \
             }                          \
        }

const static struct spi_slave slave_params[] = {
    SLAVE_PARAMS(CAN_APP_ID, &spi1_can, 10000000,  10, 1, &gpio_spi_can_nss),
    SLAVE_PARAMS(-1        , NULL     ,        0,   0, 0, &gpio_spi_mpu_nss),
    SLAVE_PARAMS(-1        , NULL     ,        0,   0, 0, &gpio_spi_acc_mag_nss),
    SLAVE_PARAMS(-1        , NULL     ,        0,   0, 0, &gpio_spi_gyro_nss),
    SLAVE_PARAMS(-1        , NULL     ,        0,   0, 0, &gpio_spi_baro_nss),
    SLAVE_PARAMS(-1        , NULL     ,        0,   0, 0, &gpio_spi_ext_nss)
};

/// A handle to the SPI bus that this component drives
static spi_bus_t* spi_bus;
clock_sys_t clock_sys;
int cur_slave_id = -1;

/**
 * Pulls the slave configuration from the database
 */
static const struct spi_slave*
get_slave(int id)
{
    int i;
    for(i = 0; i < ARRAY_SIZE(slave_params); i++){
        if(slave_params[i].id == id){
            return &slave_params[i];
        }
    }
    return NULL;
}

static inline void
chip_select(const struct spi_slave* slave, int state)
{
    slave->cs(state);
    udelay(slave->cfg.nss_udelay);
}

/**
 * SPI driver calls this when the transfer is complete.
 * All we need to do is store the status and post on the semaphore
 * that the main thread is waiting on.
 */
static void
spi_complete_callback(spi_bus_t* bus, int status, void* token)
{
    *(int*)token = status;
    bus_sem_post();
}

/**
 * Called on every SPI IRQ. Redirect control to the driver
 */
void
spi1_int_handle(void)
{
    spi_handle_irq(spi_bus);
    spi1_int_acknowledge();
}

static freq_t set_spi_freq(clk_t *clk, freq_t hz)
{
	return (unsigned int)clktree_set_spi1_freq((uint32_t)hz);
}

/* Camkes entry point */
void
spi__init(void)
{
    int err;
    clk_t *clk;

    /* Initialise the SPI bus */
    clock_sys_init_default(&clock_sys);
    err = exynos_spi_init(SPI_PORT, spi1_reg, NULL, &clock_sys, &spi_bus);
    assert(!err);
    if(err){
        LOG_ERROR("Failed to initialise SPI port\n");
        return;
    }

    clk = clk_get_clock(&clock_sys, CLK_SPI1);
    clk->set_freq = set_spi_freq;

    /* Prime the semaphore such that the first call to 'wait' will block */
    bus_sem_wait();
}

/**
 * Performs an SPI transfer
 */
static int
do_spi_transfer(const struct spi_slave* slave, void* txbuf, unsigned int wcount,
                void* rxbuf, unsigned int rcount)
{
    int ret;
    int status;

    if (cur_slave_id != slave->id) {
        spi_prepare_transfer(spi_bus, &slave->cfg);
	cur_slave_id = slave->id;
    }

    chip_select(slave, SPI_CS_ASSERT);

    /* Begin the transfer */
    ret = spi_xfer(spi_bus, txbuf, wcount, rxbuf, rcount, spi_complete_callback, &status);
    if(ret >= 0){
        bus_sem_wait();
        ret = status;
    }

    chip_select(slave, SPI_CS_RELEASE);
    return ret;
}

/**
 * Initiate the transfer of shared data
 */
int
spi_transfer(int id, unsigned int wcount, unsigned int rcount)
{
    const struct spi_slave* slave;
    /* Find the slave configuration */
    slave = get_slave(id);
    assert(slave);
    if(slave == NULL){
        return -1;
    }
    /* Transfer the data from the shared data port */
    return do_spi_transfer(slave, (*slave->port)->txbuf, wcount, (*slave->port)->rxbuf, rcount);
}

/**
 * Initiate the transfer of a single byte, passed as an argument. Return the byte received.
 */
int
spi_transfer_byte(int id, char byte)
{
    const struct spi_slave* slave;
    char data[2];
    int ret;
    /* Find the slave configuration */
    slave = get_slave(id);
    assert(slave);
    if(slave == NULL){
        return -1;
    }
    /* Transfer the data from the provided arguments */
    ret = do_spi_transfer(slave, &byte, 1, data, 1);
    if(ret < 0){
        return ret;
    }else{
        return data[1];
    }
}

