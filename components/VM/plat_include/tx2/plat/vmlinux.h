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
#ifndef VMLINUX_TX2_H
#define VMLINUX_TX2_H

#include <assert.h>
#include <sel4arm-vmm/vm.h>

/* This is the starting input pin on the GIC where the LIC IRQs are connected.
 * The LIC IRQs are connected to the next 288 starting from this pin.
 */
#define GIC_LIC_INTID_BASE      (32)
#define TX2_IRQ_PPI_VTIMER      (27)

enum IRQConstants {
    TX2_TOP_TKE_SHARED0 = GIC_LIC_INTID_BASE,
    TX2_TOP_TKE_SHARED1,
    TX2_TOP_TKE_SHARED2,
    TX2_TOP_TKE_SHARED3,
    TX2_TOP_TKE_SHARED4,
    TX2_TOP_TKE_SHARED5,
    TX2_TOP_TKE_SHARED6,
    TX2_TOP_TKE_SHARED7,
    TX2_TOP_TKE_SHARED8,
    TX2_TOP_TKE_SHARED9,
    TX2_RTC,
    TX2_LIC_GTE_0,
    TX2_LIC_GTE_1,
    TX2_AON_GTE,
    TX2_BPMP_WDT_REMOTE, /* 14 */
    TX2_SPE_WDT_REMOTE,
    TX2_SCE_WDT_REMOTE,
    TX2_TOP_WDT_REMOTE,
    TX2_AOWDT_REMOTE,
    TX2_RESERVED_19,
    TX2_DSIA,
    TX2_DSIB,
    TX2_DSIC,
    TX2_DSID,
    TX2_RESERVED_24,
    TX2_I2C,
    TX2_I2C2,
    TX2_I2C3,
    TX2_I2C4,
    TX2_I2C5,
    TX2_I2C6,
    TX2_I2C7,
    TX2_I2C8,
    TX2_I2C9,
    TX2_I2C10,
    TX2_QSPI,
    TX2_SPI1,
    TX2_SPI2,
    TX2_SPI3,
    TX2_SPI4,
    TX2_CAN1_0,
    TX2_CAN1_1,
    TX2_CAN2_0,
    TX2_CAN2_1,
    TX2_UFSHC,
    TX2_GPIO0_0, /* 45 */
    TX2_GPIO0_1,
    TX2_GPIO0_2,
    TX2_GPIO1_0,
    TX2_GPIO1_1,
    TX2_GPIO1_2,
    TX2_GPIO2_0,
    TX2_GPIO2_1,
    TX2_GPIO2_2,
    TX2_GPIO3_0,
    TX2_GPIO3_1,
    TX2_GPIO3_2,
    TX2_GPIO4_0,
    TX2_GPIO4_1,
    TX2_GPIO4_2,
    TX2_AON_GPIO_0,
    TX2_AON_GPIO_1,
    TX2_SDMMC1,
    TX2_SDMMC2,
    TX2_SDMMC3,
    TX2_SDMMC4,
    TX2_SDMMC1_SYS,
    TX2_SDMMC2_SYS,
    TX2_SDMMC3_SYS,
    TX2_SDMMC4_SYS,
    TX2_GPU_STALL,
    TX2_GPU_NONSTALL,
    TX2_PCIE_INT,
    TX2_PCIE_MSI,
    TX2_PCIE_WAKE,
    TX2_CENTRAL_DMA_CH0, /* 75 */
    TX2_CENTRAL_DMA_CH1,
    TX2_CENTRAL_DMA_CH2,
    TX2_CENTRAL_DMA_CH3,
    TX2_CENTRAL_DMA_CH4,
    TX2_CENTRAL_DMA_CH5,
    TX2_CENTRAL_DMA_CH6,
    TX2_CENTRAL_DMA_CH7,
    TX2_CENTRAL_DMA_CH8,
    TX2_CENTRAL_DMA_CH9,
    TX2_CENTRAL_DMA_CH10,
    TX2_CENTRAL_DMA_CH11,
    TX2_CENTRAL_DMA_CH12,
    TX2_CENTRAL_DMA_CH13,
    TX2_CENTRAL_DMA_CH14,
    TX2_CENTRAL_DMA_CH15,
    TX2_CENTRAL_DMA_CH16,
    TX2_CENTRAL_DMA_CH17,
    TX2_CENTRAL_DMA_CH18,
    TX2_CENTRAL_DMA_CH19,
    TX2_CENTRAL_DMA_CH20,
    TX2_CENTRAL_DMA_CH21,
    TX2_CENTRAL_DMA_CH22,
    TX2_CENTRAL_DMA_CH23,
    TX2_CENTRAL_DMA_CH24,
    TX2_CENTRAL_DMA_CH25,
    TX2_CENTRAL_DMA_CH26,
    TX2_CENTRAL_DMA_CH27,
    TX2_CENTRAL_DMA_CH28,
    TX2_CENTRAL_DMA_CH29,
    TX2_CENTRAL_DMA_CH30,
    TX2_CENTRAL_DMA_CH31,
    TX2_CENTRAL_DMA_COMMON,
    TX2_SIMON0,
    TX2_SIMON1,
    TX2_SIMON2,
    TX2_SIMON3,
    TX2_UARTA, /* 112 */
    TX2_UARTB,
    TX2_UARTC,
    TX2_UARTD,
    TX2_UARTE,
    TX2_UARTF,
    TX2_UARTG,
    TX2_NVCSI,
    maxIRQ                          = GIC_LIC_INTID_BASE + 287
} platform_interrupt_t;

/* I went down and copied out the values page by page, so in order to ensure
 * that I didn't miss anything, I'm asserting the values at the top of each page
 * to ensure there's no human error.
 */
static_assert((TX2_TOP_TKE_SHARED0 - GIC_LIC_INTID_BASE) == 0, "tx2_irqs_page1");
static_assert((TX2_BPMP_WDT_REMOTE - GIC_LIC_INTID_BASE) == 14, "tx2_irqs_page2");
static_assert((TX2_GPIO0_0 - GIC_LIC_INTID_BASE) == 45, "tx2_irqs_page3");
static_assert((TX2_CENTRAL_DMA_CH0 - GIC_LIC_INTID_BASE) == 75, "tx2_irqs_page4");
static_assert((TX2_UARTA - GIC_LIC_INTID_BASE) == 112, "tx2_irqs_page5");

#define MAX_IRQ                       maxIRQ


static const int linux_pt_irqs[] = {
    TX2_IRQ_PPI_VTIMER,
    TX2_UARTA,
    TX2_UARTB,
    TX2_UARTC,
    TX2_UARTD,
    TX2_UARTE,
    TX2_UARTF,
    TX2_UARTG,
    TX2_GPIO0_0,
    TX2_GPIO0_1,
    TX2_GPIO0_2,
    TX2_GPIO1_0,
    TX2_GPIO1_1,
    TX2_GPIO1_2,
    TX2_GPIO2_0,
    TX2_GPIO2_1,
    TX2_GPIO2_2,
    TX2_GPIO3_0,
    TX2_GPIO3_1,
    TX2_GPIO3_2,
    TX2_GPIO4_0,
    TX2_GPIO4_1,
    TX2_GPIO4_2,
    TX2_TOP_TKE_SHARED0,
    TX2_TOP_TKE_SHARED1,
    TX2_TOP_TKE_SHARED2,
    TX2_TOP_TKE_SHARED3,
    TX2_TOP_TKE_SHARED4,
    TX2_TOP_TKE_SHARED5,
    TX2_TOP_TKE_SHARED6,
    TX2_TOP_TKE_SHARED7,
    TX2_TOP_TKE_SHARED8,
    TX2_TOP_TKE_SHARED9,
};

/* This address pertains to guest-vm@f1000000 in the overlay DTS. */
#define LINUX_RAM_BASE    0xF1000000
#define LINUX_RAM_PADDR_BASE LINUX_RAM_BASE
#define LINUX_RAM_OFFSET  (LINUX_RAM_PADDR_BASE - LINUX_RAM_BASE)
#define LINUX_RAM_SIZE    0x8000000

#define DTB_ADDR          (LINUX_RAM_BASE + 0x01000000)
#define INITRD_MAX_SIZE   0x1900000 //25 MB
#define INITRD_ADDR       (DTB_ADDR - INITRD_MAX_SIZE) //0x80700000

#endif /* VMLINUX_H */

