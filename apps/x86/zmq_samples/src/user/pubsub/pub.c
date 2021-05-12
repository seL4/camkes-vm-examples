/*
 * Copyright (c) 2010-2013 iMatix Corporation and Contributors
 * SPDX-License-Identifier: MIT
 */
// Originally sourced from https://github.com/booksbyus/zguide
//  Weather update server
//  Binds PUB socket to tcp://*:5556
//  Publishes random weather updates

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))

int main (int argc, char *argv[])
{
    const char *bind_to;
    int roundtrip_count;
    size_t message_size;
    void *ctx;
    void *s;
    int rc;
    int i;
    zmq_msg_t msg;

    if (argc != 4) {
        printf ("usage: %s <bind-to> <message-size> "
                "<roundtrip-count>\n", argv[0]);
        return -1;
    }
    bind_to = argv[1];
    message_size = atoi (argv[2]);
    roundtrip_count = atoi (argv[3]);

    ctx = zmq_init (1);
    if (!ctx) {
        printf ("error in zmq_init: %s\n", zmq_strerror (errno));
        return -1;
    }

    s = zmq_socket (ctx, ZMQ_PUB);
    if (!s) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
        return -1;
    }
    rc = zmq_bind (s, bind_to);
    if (rc != 0) {
        printf ("error in zmq_bind: %s\n", zmq_strerror (errno));
        return -1;
    }


    srandom ((unsigned) time (NULL));
    printf("%d\n", roundtrip_count);
    for (int i=0; i < roundtrip_count; i++) {
        //  Get values that will fool the boss
        //printf("Sending\n");
        int zipcode, temperature, relhumidity;
        zipcode     = randof (100000);
        temperature = randof (215) - 80;
        relhumidity = randof (50) + 10;

        //  Send message to all subscribers
        char update [20];
        sprintf (update, "%05d %d %d %d", zipcode, temperature, relhumidity, i);
        int len = zmq_send (s, update, strnlen(update, 20), 0);
        //printf("%d\n", len);
        sleep(1);
    }


    rc = zmq_close (s);
    if (rc != 0) {
        printf ("error in zmq_close: %s\n", zmq_strerror (errno));
        return -1;
    }

    rc = zmq_ctx_term (ctx);
    if (rc != 0) {
        printf ("error in zmq_ctx_term: %s\n", zmq_strerror (errno));
        return -1;
    }

    return 0;
}
