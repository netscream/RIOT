/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 */

#include <stdio.h>
#include <inttypes.h>

#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"

/* 8051 implementation */
void gnrc_netif_hdr_print(gnrc_netif_hdr_t *hdr)
{
    //char addr_str[GNRC_NETIF_HDR_L2ADDR_PRINT_LEN];
    char addr_str[24];

    printf("if_pid: %u ", hdr->if_pid);
    printf("if_pid: %u ", hdr->rssi);
    printf("lqi: %u \n", hdr->lqi);
    printf("flags: ");

    if (hdr->flags) {
        //if (hdr->flags & GNRC_NETIF_HDR_FLAGS_BROADCAST) {
	if (hdr->flags & 0x80) {
            printf("BROADCAST ");
        }

        //if (hdr->flags & GNRC_NETIF_HDR_FLAGS_MULTICAST) {
	if (hdr->flags & 0x40) {
            printf("MULTICAST ");
        }
        puts("");
    }
    else {
        puts("0x0");
    }


    if (hdr->src_l2addr_len > 0) {
        printf("src_l2addr: %s\n",
               gnrc_netif_addr_to_str(addr_str, sizeof(addr_str),
                                      gnrc_netif_hdr_get_src_addr(hdr),
                                      (uint32_t)hdr->src_l2addr_len));
    }
    else {
        puts("src_l2addr: (nil)");
    }

    if (hdr->dst_l2addr_len > 0) {
        printf("dst_l2addr: %s\n",
               gnrc_netif_addr_to_str(addr_str, sizeof(addr_str),
                                      gnrc_netif_hdr_get_dst_addr(hdr),
                                      (uint32_t)hdr->dst_l2addr_len));
    }
    else {
        puts("dst_l2addr: (nil)");
    }
}

/** @} */
