/*
 * Copyright (c) 2010-2013 iMatix Corporation and Contributors
 * SPDX-License-Identifier: MIT
 */
// Originally sourced from https://github.com/booksbyus/zguide
//  Weather update client
//  Connects SUB socket to tcp://localhost:5556
//  Collects weather updates and finds avg temp in zipcode

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv [])
{
    //  Socket to talk to server
    printf ("Collecting updates from weather server…\n");
    const char *bind_to;
    int roundtrip_count;

    if (argc != 3) {
        printf ("usage: %s <bind-to> "
                "<roundtrip-count>\n", argv[0]);
        return -1;
    }
    bind_to = argv[1];
    roundtrip_count = atoi (argv[2]);


    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    int rc = zmq_connect (subscriber, bind_to);
    if (rc) {
        printf ("error in zmq_connect: %s\n", zmq_strerror (errno));
        return -1;
    }

    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE,
                         NULL, 0);
    if (rc) {
        printf ("error in zmq_setsockopt: %s\n", zmq_strerror (errno));
        return -1;
    }

    //  Process 100 updates
    int update_nbr;
    long total_temp = 0;
    for (update_nbr = 0; update_nbr < roundtrip_count; update_nbr++) {
        char buffer [256];
        int size = zmq_recv (subscriber, buffer, 255, 0);
        if (size == -1) {
            printf("Got eof");
            return -1;
        }
        buffer[size] = '\0';
        char *string = buffer;

        int zipcode, temperature, relhumidity;
        sscanf (string, "%d %d %d",
            &zipcode, &temperature, &relhumidity);
        total_temp += temperature;
        printf("temp: %s\n", string);
    }
    printf ("Average temperature for zipcode was %dF\n",
        (int) (total_temp / update_nbr));

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}
