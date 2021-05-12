/*
 * Copyright (c) 2010-2013 iMatix Corporation and Contributors
 * SPDX-License-Identifier: MIT
 */
// Originally sourced from https://github.com/booksbyus/zguide
//  Task ventilator
//  Binds PUSH socket to tcp://localhost:5557
//  Sends batch of tasks to workers via that socket

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))

int main (int argc, char *argv[])
{
    const char *bind_to1;
    const char *bind_to2;

    int roundtrip_count;
    if (argc != 4) {
        printf ("usage: %s <bind-to> <bind-to>"
                "<roundtrip-count>\n", argv[0]);
        return -1;
    }
    bind_to1 = argv[1];
    bind_to2 = argv[2];
    roundtrip_count = atoi (argv[3]);

    void *context = zmq_ctx_new ();

    //  Socket to send messages on
    void *sender = zmq_socket (context, ZMQ_PUSH);
    zmq_bind (sender, bind_to1);

    //  Socket to send start of batch message on
    void *sink = zmq_socket (context, ZMQ_PUSH);
    zmq_connect (sink, bind_to2);

     // printf ("Press Enter when the workers are ready: ");
     // getchar ();
    printf ("Sending tasks to workers…\n");

    //  The first message is "0" and signals start of batch
    zmq_send (sink, "0", 2, 0);

    //  Initialize random number generator
    srandom ((unsigned) time (NULL));

    //  Send 100 tasks
    int task_nbr;
    int total_msec = 0;     //  Total expected cost in msecs
    for (task_nbr = 0; task_nbr < roundtrip_count; task_nbr++) {
        int workload;
        //  Random workload from 1 to 100msecs
        workload = randof (100) + 1;
        total_msec += workload;
        char string [10];
        sprintf (string, "%d", workload);
        zmq_send (sender, string, strlen(string), 0);
    }
    printf ("Total expected cost: %d msec\n", total_msec);

    zmq_close (sink);
    zmq_close (sender);
    zmq_ctx_destroy (context);
    return 0;
}
