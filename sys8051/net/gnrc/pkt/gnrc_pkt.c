/*
 * Copyright (C) 2016 Freie Universität Berlin
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

#include <assert.h>

#include "net/gnrc/pkt.h"

gnrc_pktsnip_t *gnrc_pktsnip_search_type(gnrc_pktsnip_t* XDATA ptr,
                                         gnrc_nettype_t XDATA type)
{
    while (ptr != NULL) {
        if (ptr->type == type) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

uint32_t gnrc_pkt_len(gnrc_pktsnip_t* XDATA pkt)
{
    uint32_t len = 0;

    while (pkt) {
        len += pkt->size;
        pkt = pkt->next;
    }

    return len;
}

uint32_t gnrc_pkt_len_upto(gnrc_pktsnip_t* XDATA pkt, gnrc_nettype_t XDATA type)
{
    uint32_t len = 0;

    while (pkt) {
        len += pkt->size;

        if (pkt->type == type) {
            break;
        }

        pkt = pkt->next;
    }

    return len;
}

uint32_t gnrc_pkt_count(const gnrc_pktsnip_t* XDATA pkt)
{
    uint32_t count = 0;

    while (pkt) {
        ++count;
        pkt = pkt->next;
    }

    return count;
}

/** @} */
