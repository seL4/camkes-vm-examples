/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/* TX/RX Massage Queue Functions */
#ifndef  QUEUE_H
#define  QUEUE_H

#include <can_inf.h>

void mq_init(int size);
int tx_queue_push(struct can_frame *frame);
struct can_frame *tx_queue_pop(struct can_frame *frame);
int rx_queue_push(struct can_frame *frame);
struct can_frame *rx_queue_pop(struct can_frame *frame);

#endif  /*QUEUE_H*/
