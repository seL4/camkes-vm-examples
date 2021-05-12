/*
 * Copyright (c) 2010-2013 iMatix Corporation and Contributors
 * SPDX-License-Identifier: MIT
 */
// Originally sourced from https://github.com/booksbyus/zguide

//  Task worker
//  Connects PULL socket to tcp://localhost:5557
//  Collects workloads from ventilator via that socket
//  Connects PUSH socket to tcp://localhost:5558
//  Sends results to sink via that socket

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])
{
    const char *bind_to1;
    const char *bind_to2;

    int roundtrip_count;
    if (argc != 3) {
        printf ("usage: %s <bind-to> <bind-to>\n", argv[0]);
        return -1;
    }
    bind_to1 = argv[1];
    bind_to2 = argv[2];

    //  Socket to receive messages on
    void *context = zmq_ctx_new ();
    void *receiver = zmq_socket (context, ZMQ_PULL);
    zmq_connect (receiver, bind_to1);

    //  Socket to send messages to
    void *sender = zmq_socket (context, ZMQ_PUSH);
    zmq_connect (sender, bind_to2);

    //  Process tasks forever
    while (1) {
        char buffer [256];
        int size = zmq_recv (receiver, buffer, 255, 0);
        if (size == -1) {
            printf("Got eof");
            return -1;
        }
        buffer[size] = '\0';
        char *string = buffer;
        printf ("%s.", string);     //  Show progress
        fflush (stdout);
        struct timespec t;
        t.tv_sec  =  atoi (string) / 1000;
        t.tv_nsec = (atoi (string) % 1000) * 1000000;
        nanosleep (&t, NULL);

        zmq_send (sender, "", 1, 0);        //  Send results to sink
    }
    zmq_close (receiver);
    zmq_close (sender);
    zmq_ctx_destroy (context);
    return 0;
}
