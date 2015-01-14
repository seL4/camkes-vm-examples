/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#include <platsupport/mux.h>
#include <platsupport/gpio.h>

#include "gpio.h"

#define UART0_CTSN  GPIOID(GPA0, 2)
#define UART0_RTSN  GPIOID(GPA0, 3)
#define PWM_EN      XEINT17

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
gpio_t o_pwm_en;

#define CLK_SRC_PERIC0  0x250
#define CLK_DIV_PERIC0  0x558
void setup_clocks(void)
{
	uint32_t val;

	/* Select XXTI as the clock source for UART 1 and 3 */
	volatile uint32_t *clk_addr;
        clk_addr = (char*)clk_tree + CLK_SRC_PERIC0;
	val = *clk_addr;
	val &= ~0xF0F0;
	*clk_addr = val;

	/* Select clock divider to 0 for UART 1 and 3 */
	clk_addr = (char*)clk_tree + CLK_DIV_PERIC0;
	val = *clk_addr;
	val &= ~0xF0F0;
	*clk_addr = val;
}

void pre_init(void) {
    exynos_mux_init(gpio1base, gpio2base, NULL, NULL, &exynos_mux);
    exynos_gpio_sys_init(&exynos_mux, &gpio_sys);

    /* Hack to change UART clock source. */
    setup_clocks();

    /* Enable UART0, UART3*/
    printf("GPIO: enable UART 0, 1 and 3\n");
    mux_feature_enable(&exynos_mux, MUX_UART0);
    mux_feature_enable(&exynos_mux, MUX_UART1);
    mux_feature_enable(&exynos_mux, MUX_UART3);

    /* PWM */
    printf("GPIO: Enable PWM output\n");
    mux_feature_enable(&exynos_mux, MUX_I2C0);
    gpio_new(&gpio_sys, PWM_EN, GPIO_DIR_OUT, &o_pwm_en);
    gpio_clr(&o_pwm_en);
    printf("GPIO: Initialisation complete\n");
}

