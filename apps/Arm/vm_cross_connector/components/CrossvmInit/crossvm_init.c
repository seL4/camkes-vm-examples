/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>
#include <camkes.h>

int run(void)
{
    memset(dest, '\0', 4096);
    strcpy(dest, "This is a crossvm dataport test string\n");

    while (1) {
        ready_wait();
        printf("Got an event\n");
        done_emit_underlying();
    }

    return 0;
}
