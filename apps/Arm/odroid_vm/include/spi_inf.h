/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/*
 * spi external interfaces
 */

#ifndef SPI_INF_H
#define SPI_INF_H

#include <stdbool.h>
#include <stdint.h>

#define SPI_TRANS_MAX_SIZE    255

typedef struct spi_buf_t{
    uint8_t txbuf[SPI_TRANS_MAX_SIZE];
    uint8_t rxbuf[SPI_TRANS_MAX_SIZE];
    volatile bool lock;                    //shared buffer lock
}spi_dev_port, *spi_dev_port_p;

#endif
