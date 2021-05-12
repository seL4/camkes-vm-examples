/*
 * Copyright (c) 2010-2013 iMatix Corporation and Contributors
 * SPDX-License-Identifier: MIT
 */
// Originally sourced from https://github.com/booksbyus/zguide

//  Task sink
//  Binds PULL socket to tcp://localhost:5558
//  Collects results from workers via that socket

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])
{

    const char *bind_to;
    int roundtrip_count;
    if (argc != 3) {
        printf ("usage: %s <bind-to> "
                "<roundtrip-count>\n", argv[0]);
        return -1;
    }
    bind_to = argv[1];
    roundtrip_count = atoi (argv[2]);

    //  Prepare our context and socket
    void *context = zmq_ctx_new ();
    void *receiver = zmq_socket (context, ZMQ_PULL);
    zmq_bind (receiver, bind_to);

    //  Wait for start of batch
    char buffer [256];
    int size = zmq_recv (receiver, buffer, 255, 0);
    if (size == -1) {
        printf("Got eof");
        return -1;
    }
    //  Start our clock now
    struct timeval tv;
    gettimeofday (&tv, NULL);
    int64_t start_time =  (int64_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000);

    //  Process 100 confirmations
    int task_nbr;
    for (task_nbr = 0; task_nbr < roundtrip_count; task_nbr++) {
        char buffer [256];
        int size = zmq_recv (receiver, buffer, 255, 0);
        if (size == -1) {
            printf("Got eof");
            return -1;
        }
        buffer[size] = '\0';
        char *string = buffer;
        if ((task_nbr / 10) * 10 == task_nbr)
            printf (":");
        else
            printf (".");
        fflush (stdout);
    }
    //  Calculate and report duration of batch
    gettimeofday (&tv, NULL);
    int64_t end_time =  (int64_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000);

    printf ("Total elapsed time: %d msec\n",
        (int) (end_time - start_time));

    zmq_close (receiver);
    zmq_ctx_destroy (context);
    return 0;
}
