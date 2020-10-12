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
#include <camkes/io.h>

#include <camkes.h>
pwm_t pwm;
pwm_t *timer_drv = NULL;
static ps_io_ops_t io_ops;

void timer_handle_event(void *token, ltimer_event_t event)
{

    /* Signal other components. */
    timer_update_emit();

}

void pre_init()
{
    int error = camkes_io_ops(&io_ops);
    ZF_LOGF_IF(error, "Failed to get camkes_io_ops");

    error = pwm_init(&pwm, io_ops, PWM_TIMER_PATH, timer_handle_event, NULL);
    if (error) {
        printf("PWM timer does not exist.\n");
    }
    timer_drv = &pwm;

    /* Run in periodic mode and start the timer. */
    pwm_set_timeout(timer_drv, NS_IN_S, true);
}

