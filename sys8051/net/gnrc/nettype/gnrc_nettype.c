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

#include "net/gnrc/nettype.h"

gnrc_nettype_t gnrc_nettype_from_ethertype(uint16_t type)
{
    switch (type) {
#ifdef MODULE_GNRC_IPV6
        case ETHERTYPE_IPV6:
            return GNRC_NETTYPE_IPV6;
#endif
#ifdef MODULE_CCN_LITE
        case ETHERTYPE_NDN:
            return GNRC_NETTYPE_CCN;
#endif
        default:
            return GNRC_NETTYPE_UNDEF;
    }
}

uint16_t gnrc_nettype_to_ethertype(gnrc_nettype_t type)
{
    switch (type) {
#ifdef MODULE_GNRC_IPV6
        case GNRC_NETTYPE_IPV6:
            return ETHERTYPE_IPV6;
#endif
#ifdef MODULE_CCN_LITE
        case GNRC_NETTYPE_CCN:
            return ETHERTYPE_NDN;
#endif
        default:
            return ETHERTYPE_UNKNOWN;
    }
}

gnrc_nettype_t gnrc_nettype_from_protnum(uint8_t num)
{
    switch (num) {
#ifdef MODULE_GNRC_ICMPV6
        case PROTNUM_ICMPV6:
            return GNRC_NETTYPE_ICMPV6;
#endif
#ifdef MODULE_GNRC_IPV6
        case PROTNUM_IPV6:
            return GNRC_NETTYPE_IPV6;
#endif
#ifdef MODULE_GNRC_TCP
        case PROTNUM_TCP:
            return GNRC_NETTYPE_TCP;
#endif
#ifdef MODULE_GNRC_UDP
        case PROTNUM_UDP:
            return GNRC_NETTYPE_UDP;
#endif
#ifdef MODULE_GNRC_IPV6_EXT
        case PROTNUM_IPV6_EXT_HOPOPT:
        case PROTNUM_IPV6_EXT_DST:
        case PROTNUM_IPV6_EXT_RH:
        case PROTNUM_IPV6_EXT_FRAG:
        case PROTNUM_IPV6_EXT_AH:
        case PROTNUM_IPV6_EXT_ESP:
        case PROTNUM_IPV6_EXT_MOB:
            return GNRC_NETTYPE_IPV6_EXT;
#endif
        default:
            return GNRC_NETTYPE_UNDEF;
    }
}

uint8_t gnrc_nettype_to_protnum(gnrc_nettype_t type)
{
    switch (type) {
#ifdef MODULE_GNRC_ICMPV6
        case GNRC_NETTYPE_ICMPV6:
            return PROTNUM_ICMPV6;
#endif
#ifdef MODULE_GNRC_IPV6
        case GNRC_NETTYPE_IPV6:
            return PROTNUM_IPV6;
#endif
#ifdef MODULE_GNRC_TCP
        case GNRC_NETTYPE_TCP:
            return PROTNUM_TCP;
#endif
#ifdef MODULE_GNRC_UDP
        case GNRC_NETTYPE_UDP:
            return PROTNUM_UDP;
#endif
        default:
            return PROTNUM_RESERVED;
    }
}

/** @} */

