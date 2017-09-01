/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */
#include <stdio.h>

#include <utils/time.h>
#include <platsupport/mach/pwm.h>

#include <camkes.h>

pwm_t pwm;
pwm_t *timer_drv = NULL;

void irq_handle(void)
{
    /* Hardware routine. */
    pwm_handle_irq(timer_drv, PWM_T4_INTERRUPT);

    /* Signal other components. */
    timer_update_emit();

    irq_acknowledge();
}

void pre_init()
{
    pwm_config_t config;

    /*
     * Provide hardware info to platsupport.
     */
    config.vaddr = (void*)timerbase;

    int error = pwm_init(&pwm, config);
    if (error) {
        printf("PWM timer does not exist.\n");
    }
    timer_drv = &pwm;

    /* Run in periodic mode and start the timer. */
    pwm_start(timer_drv);
    pwm_set_timeout(timer_drv, NS_IN_S, true);
}

