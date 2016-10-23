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

#include "net/ipv6/hdr.h"

void ipv6_hdr_print(ipv6_hdr_t *hdr)
{
    char addr_str[IPV6_ADDR_MAX_STR_LEN];

    if (!ipv6_hdr_is(hdr)) { /* 8051 implementation */
        printf("illegal version field: %u \n", ipv6_hdr_get_version(hdr));
    }

    printf("traffic class: 0x%02u (ECN: 0x%u DSCP: 0x%02u)\n", /* 8051 implementation */
           ipv6_hdr_get_tc(hdr), ipv6_hdr_get_tc_ecn(hdr), ipv6_hdr_get_tc_dscp(hdr));
    printf("flow label: 0x%05u \n", ipv6_hdr_get_fl(hdr));
    printf("length: %u next header: %u hop limit: %u \n",
           byteorder_ntohs(&hdr->len), hdr->nh, hdr->hl);
    printf("source address: %s\n", ipv6_addr_to_str(addr_str, &hdr->src,
            sizeof(addr_str)));
    printf("destination address: %s\n", ipv6_addr_to_str(addr_str, &hdr->dst,
            sizeof(addr_str)));

}

/** @} */
