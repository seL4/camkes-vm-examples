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

#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define HW_TYPE 1

void arping_reply(char *eth_buffer, virtio_net_t *virtio_net) {

    if(eth_buffer == NULL) {
        return;
    }
    struct ethhdr *rcv_req = (struct ethhdr *) eth_buffer;
    struct ether_arp *arp_req = (struct ether_arp *) (eth_buffer + sizeof(struct ethhdr));
    if (ntohs(rcv_req->h_proto) == ETH_P_ARP) {
        unsigned char reply_buffer[1500];
        struct ethhdr *send_reply = (struct ethhdr *) reply_buffer;
        struct ether_arp *arp_reply = (struct ether_arp *) (reply_buffer + sizeof(struct ethhdr));

        memcpy(send_reply->h_dest, arp_req->arp_sha, ETH_ALEN);
        send_reply->h_proto = htons(ETH_P_ARP);

        // MAC Address
        memcpy(arp_reply->arp_tha, arp_req->arp_sha, ETH_ALEN);
        memcpy(arp_reply->arp_sha, arp_req->arp_sha, ETH_ALEN);
        arp_reply->arp_sha[5] = arp_reply->arp_sha[5] + 2;

        memcpy(send_reply->h_source, arp_reply->arp_sha, ETH_ALEN);
        // IP Addresss
        memcpy(arp_reply->arp_spa, (void *)arp_req->arp_tpa, IPV4_LENGTH);
        memcpy(arp_reply->arp_tpa, (void *)arp_req->arp_spa, IPV4_LENGTH);
        // Misc arp fields
        arp_reply->ea_hdr.ar_hrd = htons(HW_TYPE);
        arp_reply->ea_hdr.ar_pro = htons(ETH_P_IP);
        arp_reply->ea_hdr.ar_op = htons(ARP_REPLY);
        arp_reply->ea_hdr.ar_hln = ETH_ALEN;
        arp_reply->ea_hdr.ar_pln = IPV4_LENGTH;

        unsigned int len[1];
        len[0] = sizeof(struct ethhdr) + sizeof(struct ether_arp);
        void *cookie;
        void *emul_buf = (void*)virtio_net->emul_driver->i_cb.allocate_rx_buf(virtio_net->emul_driver->cb_cookie, len[0], &cookie);
        if (emul_buf) {
            memcpy(emul_buf, (void*)reply_buffer, len[0]);
            virtio_net->emul_driver->i_cb.rx_complete(virtio_net->emul_driver->cb_cookie, 1, &cookie, len);
        }
    }
}
