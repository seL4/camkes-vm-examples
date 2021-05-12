/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <platsupport/mux.h>
#include <platsupport/plat/mux.h>
#include <platsupport/gpio.h>
#include <platsupport/plat/gpio.h>
#include <platsupport/irq_combiner.h>

#include "utils.h"

#include <camkes.h>

#define UART0_CTSN  GPIOID(GPA0, 2)
#define PWM_EN      XEINT17
#define CAN_CSn     XEINT16
#define CAN_INTn    XEINT15
#define CAN_RESETn  XEINT25
#define MPU_CS      XEINT14
#define MPU_INT     XEINT8
#define ACC_MAG_CS  XEINT21
#define ACC_INT     XEINT18
#define MAG_INT     XEINT23
#define GYRO_CS     XEINT11
#define GYRO_INT    XEINT20
#define BARO_CS     XEINT10
#define SPI_EXT_CS  XEINT13
#define SPI_EXT_INT XEINT19
#define PPM_GPIO    XEINT5
#define LIDAR_INT   UART0_CTSN

#define CAN_EINT_CIRQ      XEINT15_CIRQ
#define MPU_EINT_CIRQ      XEINT8_CIRQ

#define PPM_CIRQ           XEINT5_CIRQ

//#define LIDAR_EINT_IRQ_CIRQ

#define CS_FUNC(gpio)             \
    void                          \
    gpio_##gpio(int state)        \
    {                             \
        if(state)                 \
            gpio_set(&o_##gpio);  \
        else                      \
            gpio_clr(&o_##gpio);  \
    }


mux_sys_t exynos_mux;
gpio_sys_t gpio_sys;
irq_combiner_t irq_combiner;
gpio_t o_pwm_en;
/* SPI chip select */
gpio_t o_spi_can_nss;
gpio_t o_spi_mpu_nss;
gpio_t o_spi_acc_mag_nss;
gpio_t o_spi_gyro_nss;
gpio_t o_spi_baro_nss;
gpio_t o_spi_ext_nss;
CS_FUNC(spi_can_nss);
CS_FUNC(spi_mpu_nss);
CS_FUNC(spi_acc_mag_nss);
CS_FUNC(spi_gyro_nss);
CS_FUNC(spi_baro_nss);
CS_FUNC(spi_ext_nss);

/* SPI slave private IRQ */
gpio_t i_spi_can_int;
gpio_t i_spi_mpu_int;
gpio_t i_spi_acc_int;
gpio_t i_spi_mag_int;
gpio_t i_spi_gyro_int;
gpio_t i_spi_ext_int;
/* Lidar interrupt */
gpio_t i_lidar_int;

/* CAN reset */
gpio_t o_can_resetn;

/* PPM Input */
gpio_t i_ppm;

void
irq_grp26_int_handle(void){
    if(gpio_is_pending(&i_ppm)){
        gpio_pending_clear(&i_ppm);
        printf("               <<PPM INT>>\n");
        /* TODO: Call handler */
    }
    irq_grp26_int_acknowledge();
}

void
irq_grp28_int_handle(void){
    if(gpio_is_pending(&i_spi_mpu_int)){
        gpio_pending_clear(&i_spi_mpu_int);
        printf("               <<MPU INT>>\n");
        /* TODO: Call handler */
    }
    irq_grp28_int_acknowledge();
}

void
irq_grp31_int_handle(void){
    if(gpio_is_pending(&i_spi_can_int)){
        gpio_pending_clear(&i_spi_can_int);
	CANInt_emit();
    }
    irq_grp31_int_acknowledge();
}

void
xint16_31_int_handle(void){
#if 0
    if(gpio_is_pending(&i_spi_acc_int)){
        gpio_pending_clear(&i_spi_acc_int);
        printf("               <<ACC INT>>\n");
        /* TODO: Call handler */
    }
    if(gpio_is_pending(&i_spi_mag_int)){
        gpio_pending_clear(&i_spi_mag_int);
        printf("               <<MAG INT>>\n");
        /* TODO: Call handler */
    }
    if(gpio_is_pending(&i_spi_gyro_int)){
        gpio_pending_clear(&i_spi_gyro_int);
        printf("               <<GYRO INT>>\n");
        /* TODO: Call handler */
    }
#endif
    if(gpio_is_pending(&i_spi_ext_int)){
        gpio_pending_clear(&i_spi_ext_int);
        printf("               <<SPI EXT INT>>\n");
        /* TODO: Call handler */
    }
    xint16_31_int_acknowledge();
}
#define CLK_SRC_PERIC0  0x250
#define CLK_DIV_PERIC0  0x558
void setup_clocks(void)
{
	uint32_t val;

	/* Select XXTI as the clock source for UART 1 and 3 */
	volatile uint32_t *clk_addr = (volatile uint32_t*)((char*)clk_tree + CLK_SRC_PERIC0);
	val = *clk_addr;
	val &= ~0xF0F0;
	*clk_addr = val;

	/* Select clock divider to 0 for UART 1 and 3 */
	clk_addr = (volatile uint32_t*)((char*)clk_tree + CLK_DIV_PERIC0);
	val = *clk_addr;
	val &= ~0xF0F0;
	*clk_addr = val;
}

void pre_init(void) {
    exynos_mux_init(gpio1base, gpio2base, NULL, NULL, &exynos_mux);
    exynos_gpio_sys_init(&exynos_mux, &gpio_sys);
    exynos_irq_combiner_init(irqcbase, &irq_combiner);

    /* Hack to change UART clock source. */
    setup_clocks();

    /* Enable UART0, UART3*/
    printf("GPIO: enable UART 0, 1 and 3\n");
    mux_feature_enable(&exynos_mux, MUX_UART0, MUX_DIR_NOT_A_GPIO);
    mux_feature_enable(&exynos_mux, MUX_UART1, MUX_DIR_NOT_A_GPIO);
    mux_feature_enable(&exynos_mux, MUX_UART3, MUX_DIR_NOT_A_GPIO);
    mux_feature_enable(&exynos_mux, MUX_SPI1, MUX_DIR_NOT_A_GPIO);

    /* SPI chip selects */
    gpio_new(&gpio_sys, CAN_CSn,    GPIO_DIR_OUT, &o_spi_can_nss);
    gpio_new(&gpio_sys, MPU_CS,     GPIO_DIR_OUT, &o_spi_mpu_nss);
    gpio_new(&gpio_sys, ACC_MAG_CS, GPIO_DIR_OUT, &o_spi_acc_mag_nss);
    gpio_new(&gpio_sys, GYRO_CS,    GPIO_DIR_OUT, &o_spi_gyro_nss);
    gpio_new(&gpio_sys, BARO_CS,    GPIO_DIR_OUT, &o_spi_baro_nss);
    gpio_new(&gpio_sys, SPI_EXT_CS, GPIO_DIR_OUT, &o_spi_ext_nss);
    gpio_set(&o_spi_can_nss);
    gpio_set(&o_spi_mpu_nss);
    gpio_set(&o_spi_acc_mag_nss);
    gpio_set(&o_spi_gyro_nss);
    gpio_set(&o_spi_baro_nss);
    gpio_set(&o_spi_ext_nss);
    /* SPI private IRQ */
    gpio_new(&gpio_sys, CAN_INTn,    GPIO_DIR_IRQ_FALL, &i_spi_can_int);
    //gpio_new(&gpio_sys, MPU_INT,     GPIO_DIR_IRQ_FALL, &i_spi_mpu_int);
    //gpio_new(&gpio_sys, ACC_INT,     GPIO_DIR_IRQ_FALL, &i_spi_acc_int);
    //gpio_new(&gpio_sys, MAG_INT,     GPIO_DIR_IRQ_FALL, &i_spi_mag_int);
    //gpio_new(&gpio_sys, GYRO_INT,    GPIO_DIR_IRQ_FALL, &i_spi_gyro_int);
    gpio_new(&gpio_sys, SPI_EXT_INT, GPIO_DIR_IRQ_FALL, &i_spi_ext_int);

    /* LIDAR sync IRQ  */
    gpio_new(&gpio_sys, LIDAR_INT,   GPIO_DIR_IRQ_FALL, &i_lidar_int);

    /* CAN reset */
    gpio_new(&gpio_sys, CAN_RESETn,  GPIO_DIR_OUT,      &o_can_resetn);
    gpio_set(&o_can_resetn);

    /* PPM */
    gpio_new(&gpio_sys, PPM_GPIO,    GPIO_DIR_IRQ_FALL, &i_ppm);

    /* Configure IRQs that appear on the combiner */
    irq_combiner_enable_irq(&irq_combiner, CAN_EINT_CIRQ);
    irq_combiner_enable_irq(&irq_combiner, MPU_EINT_CIRQ);
    irq_combiner_enable_irq(&irq_combiner, PPM_CIRQ);

    /* PWM */
    printf("GPIO: Enable PWM output\n");
    mux_feature_enable(&exynos_mux, MUX_I2C0, MUX_DIR_NOT_A_GPIO);
    gpio_new(&gpio_sys, PWM_EN, GPIO_DIR_OUT, &o_pwm_en);
    gpio_clr(&o_pwm_en);
    printf("GPIO: Initialisation complete\n");
}

