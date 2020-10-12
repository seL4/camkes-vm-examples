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
