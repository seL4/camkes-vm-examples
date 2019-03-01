/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include "virtio_net_arping.h"

#define PROTO_ARP 0x0806
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define HW_TYPE 1

struct arp_header {
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_len;
    unsigned char protocol_len;
    unsigned short opcode;
    unsigned char sender_mac[MAC_LENGTH];
    unsigned char sender_ip[IPV4_LENGTH];
    unsigned char target_mac[MAC_LENGTH];
    unsigned char target_ip[IPV4_LENGTH];
};

void arping_reply(char *eth_buffer, virtio_net_t *virtio_net) {

    void *buffer = (void *)eth_buffer;
    if(buffer == NULL) {
        return;
    }
    struct ethhdr *rcv_req = (struct ethhdr *) buffer;
    struct arp_header *arp_req = (struct arp_header *) (buffer + sizeof(struct ethhdr));
    if (ntohs(rcv_req->h_proto) == PROTO_ARP) {
        unsigned char reply_buffer[1500];
        struct ethhdr *send_reply = (struct ethhdr *) reply_buffer;
        struct arp_header *arp_reply = (struct arp_header *) (reply_buffer + sizeof(struct ethhdr));

        memcpy(send_reply->h_dest, arp_req->sender_mac, MAC_LENGTH);
        send_reply->h_proto = htons(ETH_P_ARP);

        // MAC Address
        memcpy(arp_reply->target_mac, arp_req->sender_mac, MAC_LENGTH);
        memcpy(arp_reply->sender_mac, arp_req->sender_mac, MAC_LENGTH);
        arp_reply->sender_mac[5] = arp_reply->sender_mac[5] + 2;

        memcpy(send_reply->h_source, arp_reply->sender_mac, MAC_LENGTH);
        // IP Addresss
        memcpy(arp_reply->sender_ip, (void *)arp_req->target_ip, IPV4_LENGTH);
        memcpy(arp_reply->target_ip, (void *)arp_req->sender_ip, IPV4_LENGTH);
        // Misc arp fields
        arp_reply->hardware_type = htons(HW_TYPE);
        arp_reply->protocol_type = htons(ETH_P_IP);
        arp_reply->opcode = htons(ARP_REPLY);
        arp_reply->hardware_len = MAC_LENGTH;
        arp_reply->protocol_len = IPV4_LENGTH;

        unsigned int len[1];
        len[0] = 42;
        void *cookie;
        void *emul_buf = (void*)virtio_net->emul_driver->i_cb.allocate_rx_buf(virtio_net->emul_driver->cb_cookie, len[0], &cookie);
        if (emul_buf) {
            memcpy(emul_buf, (void*)reply_buffer, len[0]);
            virtio_net->emul_driver->i_cb.rx_complete(virtio_net->emul_driver->cb_cookie, 1, &cookie, len);
        }
    }
}
