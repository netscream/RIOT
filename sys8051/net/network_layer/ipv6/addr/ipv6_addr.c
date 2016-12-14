/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

/**
 * @{
 *
 * @file
 *
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "net/ipv6/addr.h"

#ifdef MODULE_FMT
#include "fmt.h"
#else
#include <stdio.h>
#endif

/*const ipv6_addr_t ipv6_addr_unspecified = IPV6_ADDR_UNSPECIFIED;
const ipv6_addr_t ipv6_addr_loopback = IPV6_ADDR_LOOPBACK;
const ipv6_addr_t ipv6_addr_link_local_prefix = IPV6_ADDR_LINK_LOCAL_PREFIX;
const ipv6_addr_t ipv6_addr_solicited_node_prefix = IPV6_ADDR_SOLICITED_NODE_PREFIX;
const ipv6_addr_t ipv6_addr_all_nodes_if_local = IPV6_ADDR_ALL_NODES_IF_LOCAL;
const ipv6_addr_t ipv6_addr_all_nodes_link_local = IPV6_ADDR_ALL_NODES_LINK_LOCAL;
const ipv6_addr_t ipv6_addr_all_routers_if_local = IPV6_ADDR_ALL_ROUTERS_IF_LOCAL;
const ipv6_addr_t ipv6_addr_all_routers_link_local = IPV6_ADDR_ALL_ROUTERS_LINK_LOCAL;
const ipv6_addr_t ipv6_addr_all_routers_site_local = IPV6_ADDR_ALL_ROUTERS_SITE_LOCAL;
*/
const ipv6_addr_t ipv6_addr_unspecified = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
const ipv6_addr_t ipv6_addr_loopback = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }};
const ipv6_addr_t ipv6_addr_link_local_prefix = {{ 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
const ipv6_addr_t ipv6_addr_solicited_node_prefix = {{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00 }};
const ipv6_addr_t ipv6_addr_all_nodes_if_local = {{ 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }};
const ipv6_addr_t ipv6_addr_all_nodes_link_local = {{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }};
const ipv6_addr_t ipv6_addr_all_routers_if_local = {{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }};
const ipv6_addr_t ipv6_addr_all_routers_link_local = {{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }};
const ipv6_addr_t ipv6_addr_all_routers_site_local = {{ 0xff, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }};

bool ipv6_addr_equal(const ipv6_addr_t *a, const ipv6_addr_t *b)
{
    return ((a->u32[0].u32 == b->u32[0].u32) && 
           (a->u32[1].u32 == b->u32[1].u32) &&
           (a->u32[2].u32 == b->u32[2].u32) &&
           (a->u32[3].u32 == b->u32[3].u32));
    /*return (a->u64[0].u64 == b->u64[0].u64) &&
           (a->u64[1].u64 == b->u64[1].u64);*/
}

uint8_t ipv6_addr_match_prefix(const ipv6_addr_t *a, const ipv6_addr_t *b)
{
    uint8_t prefix_len = 0;
    int i = 0;
    if ((a == NULL) || (b == NULL)) {
        return 0;
    }

    if (a == b) {
        return 128;
    }

    for (i = 0; i < 16; i++) {
        /* if bytes are equal add 8 */
        if (a->u8[i] == b->u8[i]) {
            prefix_len += 8;
        }
        else {
            uint8_t xor = (a->u8[i] ^ b->u8[i]);
            int j = 0;
            /* while bits from byte equal add 1 */
            for (j = 0; j < 8; j++) {
                if ((xor & 0x80) == 0) {
                    prefix_len++;
                    xor = xor << 1;
                }
                else {
                    break;
                }
            }

            break;
        }
    }

    return prefix_len;
}

void ipv6_addr_init_prefix(ipv6_addr_t *out, const ipv6_addr_t *prefix,
                           uint8_t bits)
{
    uint8_t bytes;

    if (bits > 128) {
        bits = 128;
    }

    bytes = bits / 8;

    memcpy(out, prefix, bytes);

    if (bits % 8) {
        uint8_t mask = 0xff << (8 - (bits - (bytes * 8)));

        out->u8[bytes] &= ~mask;
        out->u8[bytes] |= (prefix->u8[bytes] & mask);
    }
}

void ipv6_addr_init_iid(ipv6_addr_t *out, const uint8_t *iid, uint8_t bits)
{
    uint8_t unaligned_bits, bytes, pos;

    if (bits > 128) {
        bits = 128;
    }

    unaligned_bits = bits % 8;
    bytes = bits / 8;
    //pos = (IPV6_ADDR_BIT_LEN / 8) - bytes;
    pos = 128/8 - bytes;
    if (unaligned_bits) {
        uint8_t mask = 0xff << unaligned_bits;
        out->u8[pos - 1] &= mask;
        out->u8[pos - 1] |= (*iid & ~mask);
        iid++;
    }

    memcpy(&(out->u8[pos]), iid, bytes);
}

int ipv6_addr_split(char *addr_str, char seperator, int _default)
{
    char *sep = addr_str;
    while(*++sep) {
        if (*sep == seperator) {
            *sep++ = '\0';
            if (*sep) {
                _default = atoi(sep);
            }
            break;
        }
    }

    return _default;
}
/* 8051 implementation */
void ipv6_addr_print(const ipv6_addr_t *addr)
{
    //char addr_str[IPV6_ADDR_MAX_STR_LEN];
    char addr_str[40];
    assert(addr);
    ipv6_addr_to_str(addr_str, addr, sizeof(addr_str));
#ifdef MODULE_FMT
    print_str(addr_str);
#else
    printf("%s", addr_str);
#endif
}

uint16_t ipv6_addr_is_unspecified(const ipv6_addr_t *addr)
{
    return (memcmp(addr, &ipv6_addr_unspecified, sizeof(ipv6_addr_t)) == 0);
}

bool ipv6_addr_is_loopback(const ipv6_addr_t *addr)
{
   return (memcmp(addr, &ipv6_addr_loopback, sizeof(ipv6_addr_t)) == 0);
}

bool ipv6_addr_is_ipv4(const ipv6_addr_t *addr)
{
    return (memcmp(addr, &ipv6_addr_unspecified,
                   sizeof(ipv6_addr_t) - sizeof(ipv4_addr_t)) == 0);
}

bool ipv6_addr_is_ipv4_mapped(const ipv6_addr_t *addr)
{
    return ((memcmp(addr, &ipv6_addr_unspecified,
                    sizeof(ipv6_addr_t) - sizeof(ipv4_addr_t) - 2) == 0) &&
            (addr->u16[5].u16 == 0xffff));
}

bool ipv6_addr_is_multicast(const ipv6_addr_t *addr)
{
    return (addr->u8[0] == 0xff);
}

bool ipv6_addr_is_link_local(const ipv6_addr_t *addr)
{
    return (memcmp(addr, &ipv6_addr_link_local_prefix, sizeof(addr->u32)) == 0) ||
           (ipv6_addr_is_multicast(addr) &&
            //(addr->u8[1] & 0x0f) == IPV6_ADDR_MCAST_SCP_LINK_LOCAL);
              (addr->u8[1] & 0x0f) == 0x2);
}

bool ipv6_addr_is_site_local(const ipv6_addr_t *addr)
{
    network_uint16_t tmp = { addr->u16[0].u16 };
    //return ((byteorder_ntohs(&tmp) & 0xffc0) == (uint16_t)IPV6_ADDR_SITE_LOCAL_PREFIX);
    return (((byteorder_ntohs(&tmp) & 0xffc0) ==
             //IPV6_ADDR_SITE_LOCAL_PREFIX) ||
            (ipv6_addr_is_multicast(addr) && (addr->u8[1] & 0x0f) == 0x2)));
             //(addr->u8[1] & 0x0f) == IPV6_ADDR_MCAST_SCP_SITE_LOCAL));
}

bool ipv6_addr_is_unique_local_unicast(const ipv6_addr_t *addr)
{
    return ((addr->u8[0] == 0xfc) || (addr->u8[0] == 0xfd));
}

/**
 * @}
 */
