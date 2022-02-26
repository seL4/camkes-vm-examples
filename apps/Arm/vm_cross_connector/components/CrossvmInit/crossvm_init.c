/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>
#include <camkes.h>

void vm0_event_handler(void *arg)
{
    printf("Got an event from vm0\n");
	ready_reg_callback(vm0_event_handler, NULL);
}

void vm1_event_handler(void *arg)
{
    printf("Got an event from vm1\n");
	ready1_reg_callback(vm1_event_handler, NULL);
}

int run(void)
{
    memset(dest, '\0', 4096);
    strcpy(dest, "This is a crossvm dataport test string to VM0\n");

    memset(dest1, '\0', 4096);
    strcpy(dest1, "This is a crossvm dataport test string to VM1\n");

	ready_reg_callback(vm1_event_handler, NULL);
	ready1_reg_callback(vm0_event_handler, NULL);

    return 0;
}
