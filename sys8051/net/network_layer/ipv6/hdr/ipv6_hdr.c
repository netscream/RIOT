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

void ipv6_hdr_set_version(ipv6_hdr_t *hdr)
{
    hdr->v_tc_fl.u8[0] &= 0x0f;
    hdr->v_tc_fl.u8[0] |= 0x60;
}

uint8_t ipv6_hdr_get_version(const ipv6_hdr_t *hdr)
{
    return ((hdr->v_tc_fl.u8[0]) >> 4);
}

bool ipv6_hdr_is(const ipv6_hdr_t *hdr)
{
    return (((hdr->v_tc_fl.u8[0]) & 0xf0) == 0x60);
}

void ipv6_hdr_set_tc(ipv6_hdr_t *hdr, uint8_t tc)
{
    hdr->v_tc_fl.u8[0] &= 0xf0;
    hdr->v_tc_fl.u8[0] |= (0x0f & (tc >> 4));
    hdr->v_tc_fl.u8[1] &= 0x0f;
    hdr->v_tc_fl.u8[1] |= (0xf0 & (tc << 4));
}

void ipv6_hdr_set_tc_ecn(ipv6_hdr_t *hdr, uint8_t ecn)
{
    hdr->v_tc_fl.u8[0] &= 0xf3;
    hdr->v_tc_fl.u8[0] |= (0x0c & (ecn << 2));
}

void ipv6_hdr_set_tc_dscp(ipv6_hdr_t *hdr, uint8_t dscp)
{
    hdr->v_tc_fl.u8[0] &= 0xfc;
    hdr->v_tc_fl.u8[0] |= (0x03 & (dscp >> 4));
    hdr->v_tc_fl.u8[1] &= 0x0f;
    hdr->v_tc_fl.u8[1] |= (0xf0 & (dscp << 4));
}

uint8_t ipv6_hdr_get_tc(const ipv6_hdr_t *hdr)
{
    return ((((hdr->v_tc_fl.u8[0]) & 0x0f) << 4) |
            ((hdr->v_tc_fl.u8[1] & 0xf0) >> 4));
}

uint8_t ipv6_hdr_get_tc_ecn(const ipv6_hdr_t *hdr)
{
    return (((hdr->v_tc_fl.u8[0]) & 0x0c) >> 2);
}

uint8_t ipv6_hdr_get_tc_dscp(const ipv6_hdr_t *hdr)
{
    return ((((hdr->v_tc_fl.u8[0]) & 0x03) << 4) |
            ((hdr->v_tc_fl.u8[1] & 0xf0) >> 4));
}

void ipv6_hdr_set_fl(ipv6_hdr_t *hdr, uint32_t fl)
{
    hdr->v_tc_fl.u8[1] &= 0xf0;
    hdr->v_tc_fl.u8[1] |= (0x0f & (byteorder_htonl(fl)->u8[1]));
    hdr->v_tc_fl.u16[1] = byteorder_htonl(fl)->u16[1];
}

uint32_t ipv6_hdr_get_fl(const ipv6_hdr_t *hdr)
{
    network_uint32_t tmp = { hdr->v_tc_fl.u32 }; 
    return byteorder_ntohl(&tmp) & 0xfffff;
}

uint16_t ipv6_hdr_inet_csum(uint16_t sum, ipv6_hdr_t *hdr,
                                          uint8_t prot_num, uint16_t len)
{
    uint32_t tmp = sum+len+prot_num;
    if (tmp > 0xffff) {
        /* increment by one for overflow to keep it as 1's complement sum */
        sum++;
    }

    return inet_csum(sum + len + prot_num, hdr->src.u8,
                     (2 * sizeof(ipv6_addr_t)));
}

/** @} */

