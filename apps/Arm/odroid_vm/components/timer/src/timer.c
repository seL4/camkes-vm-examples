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
