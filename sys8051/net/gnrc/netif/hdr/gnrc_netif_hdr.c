/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include "net/gnrc/netif/hdr.h"

gnrc_pktsnip_t *gnrc_netif_hdr_build(uint8_t *src, uint8_t src_len, uint8_t *dst, uint8_t dst_len)
{
    gnrc_pktsnip_t *pkt = gnrc_pktbuf_add(NULL, NULL,
                                          sizeof(gnrc_netif_hdr_t) + src_len + dst_len,
                                          GNRC_NETTYPE_NETIF);

    if (pkt == NULL) {
        return NULL;
    }

    gnrc_netif_hdr_init(pkt->data, src_len, dst_len);

    if (src != NULL && src_len > 0) {
        gnrc_netif_hdr_set_src_addr(pkt->data, src, src_len);
    }

    if (dst != NULL && dst_len > 0) {
        gnrc_netif_hdr_set_dst_addr(pkt->data, dst, dst_len);
    }

    return pkt;
}

void gnrc_netif_hdr_init(gnrc_netif_hdr_t *hdr, uint8_t src_l2addr_len,
                                       uint8_t dst_l2addr_len)
{
    hdr->src_l2addr_len = src_l2addr_len;
    hdr->dst_l2addr_len = dst_l2addr_len;
    //hdr->if_pid = KERNEL_PID_UNDEF;
    hdr->if_pid = 0;
    hdr->rssi = 0;
    hdr->lqi = 0;
    hdr->flags = 0;
}

uint32_t gnrc_netif_hdr_sizeof(gnrc_netif_hdr_t *hdr)
{
    return sizeof(gnrc_netif_hdr_t) + hdr->src_l2addr_len + hdr->dst_l2addr_len;
}

uint8_t *gnrc_netif_hdr_get_src_addr(gnrc_netif_hdr_t *hdr)
{
    return ((uint8_t *)(hdr + 1));
}

void gnrc_netif_hdr_set_src_addr(gnrc_netif_hdr_t *hdr, uint8_t *addr,
        uint8_t addr_len)
{
    if (addr_len != hdr->src_l2addr_len) {
        return;
    }

    memcpy(((uint8_t *)(hdr + 1)), addr, addr_len);
}

uint8_t *gnrc_netif_hdr_get_dst_addr(gnrc_netif_hdr_t *hdr)
{
    return (((uint8_t *)(hdr + 1)) + hdr->src_l2addr_len);
}

void gnrc_netif_hdr_set_dst_addr(gnrc_netif_hdr_t *hdr, uint8_t *addr,
        uint8_t addr_len)
{
    if (addr_len != hdr->dst_l2addr_len) {
        return;
    }

    memcpy(((uint8_t *)(hdr + 1)) + hdr->src_l2addr_len, addr, addr_len);
}


/** @} */
