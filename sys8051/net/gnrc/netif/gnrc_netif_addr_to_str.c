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
#include <stdio.h> //for NULL
#include "net/gnrc/netif.h"

char _half_byte_to_char(uint8_t half_byte)
{
    return (half_byte < 10) ? ('0' + half_byte) : ('a' + (half_byte - 10));
}

char *gnrc_netif_addr_to_str(char *out, uint32_t out_len, const uint8_t *addr,
                             uint32_t addr_len)
{
    uint32_t i;

    if (out_len < (3 * addr_len)) { /* 2 for every byte, 1 for ':' or '\0' */
        return NULL;
    }

    out[0] = '\0';

    for (i = 0; i < addr_len; i++) {
        out[(3 * i)] = _half_byte_to_char(addr[i] >> 4);
        out[(3 * i) + 1] = _half_byte_to_char(addr[i] & 0xf);

        if (i != (addr_len - 1)) {
            out[(3 * i) + 2] = ':';
        }
        else {
            out[(3 * i) + 2] = '\0';
        }
    }

    return out;
}

/** @} */
