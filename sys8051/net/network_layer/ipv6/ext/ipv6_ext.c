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

#include "net/ipv6/ext.h"

ipv6_ext_t *ipv6_ext_get_next(ipv6_ext_t* XDATA ext)
{
    return (ipv6_ext_t *)((uint8_t *)(ext) + (ext->len * IPV6_EXT_LEN_UNIT) +
                          IPV6_EXT_LEN_UNIT);
}

/** @} */

