/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
/*
 * Timer interface implementation.
 */

#include <platsupport/mach/pwm.h>

#include <camkes.h>

extern pwm_t *timer_drv;

/*
 * Get current time.
 *
 * TODO: platsupport returns a 64-bit time.
 */
int tm_get_time()
{
	return pwm_get_time(timer_drv);
}
