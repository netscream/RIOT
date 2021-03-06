/*
 * Copyright (C) 2014 Martin Lenders <mlenders@inf.fu-berlin.de>
 * Copyright (C) 2015 Oliver Hahm <oliver.hahm@inria.fr>
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
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 */

#include <errno.h>
#include <string.h>

#include "kernel_types.h"
#include "mutex.h"
#include "bitfield.h"

#include "net/eui64.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/ndp.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/sixlowpan/nd.h"
#include "net/gnrc/sixlowpan/netif.h"

#include "net/gnrc/ipv6/netif.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#if ENABLE_DEBUG
/* For PRIu16 etc. */
#include <inttypes.h>
#endif

/* number of "points" assigned to an source address candidate with equal scope
 * than destination address */
//#define RULE_2A_PTS         (4)
/* number of "points" assigned to an source address candidate with smaller scope
 * than destination address */
//#define RULE_2B_PTS         (2)
/* number of "points" assigned to an source address candidate in preferred state */
//#define RULE_3_PTS          (1)

//static gnrc_ipv6_netif_t ipv6_ifs[GNRC_NETIF_NUMOF];
gnrc_ipv6_netif_t XDATA ipv6_ifs[1];

/*#if ENABLE_DEBUG
static char addr_str[IPV6_ADDR_MAX_STR_LEN];
#endif*/
/* 8051 implementation */
ipv6_addr_t *_add_addr_to_entry(gnrc_ipv6_netif_t *entry, const ipv6_addr_t *addr,
                                       uint8_t prefix_len, uint8_t flags)
{
    int i = 0;
    gnrc_ipv6_netif_addr_t *tmp_addr = NULL;

    //for (i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
    for (i = 0; i < 8; i++) { 
        if (ipv6_addr_equal(&(entry->addrs[i].addr), addr)) {
            return &(entry->addrs[i].addr);
        }

        if (ipv6_addr_is_unspecified(&(entry->addrs[i].addr)) && !tmp_addr) {
            tmp_addr = &(entry->addrs[i]);
        }
    }

    if (!tmp_addr) {
        DEBUG("ipv6 netif: couldn't add %s/%" PRIu8 " to interface %" PRIkernel_pid "\n: No space left.\n",
              ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)),
              prefix_len, entry->pid);
        return NULL;
    }

    memcpy(&(tmp_addr->addr), addr, sizeof(ipv6_addr_t));
    DEBUG("ipv6 netif: Added %s/%" PRIu8 " to interface %" PRIkernel_pid "\n",
          ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)),
          prefix_len, entry->pid);

    tmp_addr->prefix_len = prefix_len;
    tmp_addr->flags = flags;

#ifdef MODULE_GNRC_SIXLOWPAN_ND
    if (!ipv6_addr_is_multicast(&(tmp_addr->addr)) &&
        (entry->flags & GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN)) {
        ipv6_addr_t *router = gnrc_ndp_internal_default_router();
        if (router != NULL) {
            mutex_unlock(&entry->mutex);    /* function below relocks mutex */
            gnrc_ndp_internal_send_nbr_sol(entry->pid, &tmp_addr->addr, router, router);
            mutex_lock(&entry->mutex);      /* relock mutex */
        }
        /* otherwise there is no default router to register to */
    }
#endif

    if (ipv6_addr_is_multicast(addr)) {
        //tmp_addr->flags |= GNRC_IPV6_NETIF_ADDR_FLAGS_NON_UNICAST;
	tmp_addr->flags |= 0x01;
    }
    else {
        if (!ipv6_addr_is_link_local(addr)) {
#ifdef MODULE_GNRC_SIXLOWPAN_ND_BORDER_ROUTER
            tmp_addr->valid = UINT32_MAX;
            tmp_addr->preferred = UINT32_MAX;
            gnrc_sixlowpan_nd_router_abr_t *abr = gnrc_sixlowpan_nd_router_abr_get();
            mutex_unlock(&entry->mutex);
            gnrc_ipv6_netif_set_rtr_adv(entry, true);
            mutex_lock(&entry->mutex);
            if (gnrc_sixlowpan_nd_router_abr_add_prf(abr, entry, tmp_addr) < 0) {
                DEBUG("ipv6_netif: error adding prefix to 6LoWPAN-ND management\n");
            }
#endif
#if defined(MODULE_GNRC_NDP_ROUTER) || defined(MODULE_GNRC_SIXLOWPAN_ND_ROUTER)
            if ((entry->flags & GNRC_IPV6_NETIF_FLAGS_ROUTER) &&
                (entry->flags & GNRC_IPV6_NETIF_FLAGS_RTR_ADV)) {
                mutex_unlock(&entry->mutex);    /* function below relocks mutex */
#ifdef MODULE_GNRC_SIXLOWPAN_ND_ROUTER
                if (entry->flags & GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN) {
                    gnrc_ndp_internal_send_rtr_adv(entry->pid, NULL, NULL, false);
                }
#endif
#ifdef MODULE_GNRC_NDP_ROUTER
                /* New prefixes MAY allow the router to retransmit up to
                 * GNRC_NDP_MAX_INIT_RTR_ADV_NUMOF unsolicited RA
                 * (see https://tools.ietf.org/html/rfc4861#section-6.2.4) */
                if (!(entry->flags & GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN)) {
                    entry->rtr_adv_count = GNRC_NDP_MAX_INIT_RTR_ADV_NUMOF;
                    gnrc_ndp_router_retrans_rtr_adv(entry);
                }
#endif
                mutex_lock(&entry->mutex);      /* relock mutex */
            }
#endif
        }
        else {
            //tmp_addr->flags |= GNRC_IPV6_NETIF_ADDR_FLAGS_NDP_ON_LINK;
	    tmp_addr->flags |= 0x80;
        }
#if defined(MODULE_GNRC_NDP_NODE) || defined(MODULE_GNRC_SIXLOWPAN_ND_ROUTER)
        /* add solicited-nodes multicast address for new address if interface is not a
         * 6LoWPAN host interface (see: https://tools.ietf.org/html/rfc6775#section-5.2) */
        if (!(entry->flags & GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN) ||
            (entry->flags & GNRC_IPV6_NETIF_FLAGS_ROUTER)) {
            ipv6_addr_t sol_node;
            ipv6_addr_set_solicited_nodes(&sol_node, addr);
            _add_addr_to_entry(entry, &sol_node, IPV6_ADDR_BIT_LEN, 0);
        }
#endif
        /* TODO: send NS with ARO on 6LoWPAN interfaces, but not so many and only for the new
         *       source address. */
    }

    //tmp_addr->valid_timeout_msg.type = GNRC_NDP_MSG_ADDR_TIMEOUT;
    tmp_addr->valid_timeout_msg.type = 0x0211;
    tmp_addr->valid_timeout_msg.content.ptr = &tmp_addr->addr;

    return &(tmp_addr->addr);
}

void _reset_addr_from_entry(gnrc_ipv6_netif_t *entry)
{
    DEBUG("ipv6 netif: Reset IPv6 addresses on interface %" PRIkernel_pid "\n", entry->pid);
    memset(entry->addrs, 0, sizeof(entry->addrs));
}

void _ipv6_netif_remove(gnrc_ipv6_netif_t *entry)
{
    if (entry == NULL) {
        return;
    }

#ifdef MODULE_GNRC_NDP
    gnrc_ndp_netif_remove(entry);
#endif

    mutex_lock(&entry->mutex);
    xtimer_remove(&entry->rtr_sol_timer);
#ifdef MODULE_GNRC_NDP_ROUTER
    xtimer_remove(&entry->rtr_adv_timer);
#endif
    _reset_addr_from_entry(entry);
    DEBUG("ipv6 netif: Remove IPv6 interface %" PRIkernel_pid "\n", entry->pid);
    //entry->pid = KERNEL_PID_UNDEF;
    entry->pid = 0;
    entry->flags = 0;

    mutex_unlock(&entry->mutex);
}

/* 8051 implementation */
void gnrc_ipv6_netif_init(void)
{
    int i = 0;
    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        mutex_init(&(ipv6_ifs[i].mutex));
        _ipv6_netif_remove(&ipv6_ifs[i]);
    }
}

/* 8051 implementation */
void gnrc_ipv6_netif_add(kernel_pid_t pid)
{
    int i = 0;
    gnrc_ipv6_netif_t *free_entry = NULL;

    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        if (ipv6_ifs[i].pid == pid) {
            /* pid has already been added */
            return;
        }

        //else if ((ipv6_ifs[i].pid == KERNEL_PID_UNDEF) && !free_entry) {
	else if ((ipv6_ifs[i].pid == 0) && !free_entry) {
            /* found the first free entry */
            free_entry = &ipv6_ifs[i];
        }
    }

    if (!free_entry) {
        DEBUG("ipv6 netif: Could not add %" PRIkernel_pid " to IPv6: No space left.\n", pid);
        return;
    }

    /* Otherwise, fill the free entry */

    mutex_lock(&free_entry->mutex);

    DEBUG("ipv6 netif: Add IPv6 interface %" PRIkernel_pid " (i = %d)\n", pid,
          free_entry - ipv6_ifs);
    free_entry->pid = pid;
    //free_entry->mtu = GNRC_IPV6_NETIF_DEFAULT_MTU;
    //free_entry->cur_hl = GNRC_IPV6_NETIF_DEFAULT_HL;
    free_entry->mtu = 1280;
    free_entry->cur_hl = 64;
    free_entry->flags = 0;

    _add_addr_to_entry(free_entry, &ipv6_addr_all_nodes_link_local,
                       //IPV6_ADDR_BIT_LEN, 0);
		       128, 0);

    mutex_unlock(&free_entry->mutex);

#ifdef MODULE_GNRC_NDP
    gnrc_ndp_netif_add(free_entry);
#endif

    DEBUG(" * pid = %" PRIkernel_pid "  ", free_entry->pid);
    DEBUG("cur_hl = %d  ", free_entry->cur_hl);
    DEBUG("mtu = %d  ", free_entry->mtu);
    DEBUG("flags = %04" PRIx16 "\n", free_entry->flags);
}

void gnrc_ipv6_netif_remove(kernel_pid_t pid)
{
    gnrc_ipv6_netif_t *entry = gnrc_ipv6_netif_get(pid);
    _ipv6_netif_remove(entry);
}

/* 8051 implementation */
gnrc_ipv6_netif_t *gnrc_ipv6_netif_get(kernel_pid_t pid)
{
    int i = 0;
    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        if (ipv6_ifs[i].pid == pid) {
            DEBUG("ipv6 netif: Get IPv6 interface %" PRIkernel_pid " (%p, i = %d)\n", pid,
                  (void *)(&(ipv6_ifs[i])), i);
            return &(ipv6_ifs[i]);
        }
    }

    return NULL;
}

#if defined(MODULE_GNRC_NDP_ROUTER) || defined(MODULE_GNRC_SIXLOWPAN_ND_ROUTER)
void gnrc_ipv6_netif_set_router(gnrc_ipv6_netif_t *netif, bool enable)
{
#ifdef MODULE_GNRC_SIXLOWPAN_ND_ROUTER
    if (netif->flags & GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN) {
        gnrc_sixlowpan_nd_router_set_router(netif, enable);
        return;
    }
#endif
#ifdef MODULE_GNRC_NDP_ROUTER
    gnrc_ndp_router_set_router(netif, enable);
#endif
}

void gnrc_ipv6_netif_set_rtr_adv(gnrc_ipv6_netif_t *netif, bool enable)
{
#if defined(MODULE_GNRC_SIXLOWPAN_ND_ROUTER)
    if (netif->flags & GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN) {
        gnrc_sixlowpan_nd_router_set_rtr_adv(netif, enable);
        return;
    }
#endif
#ifdef MODULE_GNRC_NDP_ROUTER
    gnrc_ndp_router_set_rtr_adv(netif, enable);
#endif
}
#endif

ipv6_addr_t *gnrc_ipv6_netif_add_addr(kernel_pid_t pid, const ipv6_addr_t *addr,
                                      uint8_t prefix_len, uint8_t flags)
{
    gnrc_ipv6_netif_t *entry = gnrc_ipv6_netif_get(pid);
    ipv6_addr_t *res;

    if ((entry == NULL) || (addr == NULL) || (ipv6_addr_is_unspecified(addr)) ||
        ((prefix_len - 1) > 127)) {    /* prefix_len < 1 || prefix_len > 128 */
        return NULL;
    }

    mutex_lock(&entry->mutex);

    res = _add_addr_to_entry(entry, addr, prefix_len, flags);

    mutex_unlock(&entry->mutex);

    return res;
}

/* 8051 implementation */
void _remove_addr_from_entry(gnrc_ipv6_netif_t *entry, ipv6_addr_t *addr)
{
    int i = 0;
    mutex_lock(&entry->mutex);

    //for (i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
    for (i = 0; i < 8; i++) {
        if (ipv6_addr_equal(&(entry->addrs[i].addr), addr)) {
            DEBUG("ipv6 netif: Remove %s to interface %" PRIkernel_pid "\n",
                  ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)), entry->pid);
            ipv6_addr_set_unspecified(&(entry->addrs[i].addr));
            entry->addrs[i].flags = 0;
#ifdef MODULE_GNRC_NDP_ROUTER
            /* Removal of prefixes MAY allow the router to retransmit up to
             * GNRC_NDP_MAX_INIT_RTR_ADV_NUMOF unsolicited RA
             * (see https://tools.ietf.org/html/rfc4861#section-6.2.4) */
            if ((entry->flags & GNRC_IPV6_NETIF_FLAGS_ROUTER) &&
                (entry->flags & GNRC_IPV6_NETIF_FLAGS_RTR_ADV) &&
                (!ipv6_addr_is_multicast(addr) &&
                 !ipv6_addr_is_link_local(addr))) {
                entry->rtr_adv_count = GNRC_NDP_MAX_INIT_RTR_ADV_NUMOF;
                mutex_unlock(&entry->mutex);    /* function below relocks the mutex */
                gnrc_ndp_router_retrans_rtr_adv(entry);
                return;
            }
#endif
#ifdef MODULE_GNRC_SIXLOWPAN_ND_BORDER_ROUTER
            gnrc_sixlowpan_nd_router_abr_t *abr = gnrc_sixlowpan_nd_router_abr_get();
            gnrc_sixlowpan_nd_router_abr_rem_prf(abr, entry, &entry->addrs[i]);
#endif

            mutex_unlock(&entry->mutex);
            return;
        }
    }

    mutex_unlock(&entry->mutex);
}

/* 8051 implementation */
void gnrc_ipv6_netif_remove_addr(kernel_pid_t pid, ipv6_addr_t *addr)
{
    int i = 0;
    //if (pid == KERNEL_PID_UNDEF) {
    if (pid == 0){
        //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
	for (i = 0; i < 1; i++) {
            //if (ipv6_ifs[i].pid == KERNEL_PID_UNDEF) {
	    if (ipv6_ifs[i].pid == 0) {
                continue;
            }

            _remove_addr_from_entry(ipv6_ifs + i, addr);
        }
    }
    else {
        gnrc_ipv6_netif_t *entry = gnrc_ipv6_netif_get(pid);

        _remove_addr_from_entry(entry, addr);
    }
}

void gnrc_ipv6_netif_reset_addr(kernel_pid_t pid)
{
    gnrc_ipv6_netif_t *entry = gnrc_ipv6_netif_get(pid);

    if (entry == NULL) {
        return;
    }

    mutex_lock(&entry->mutex);

    _reset_addr_from_entry(entry);

    mutex_unlock(&entry->mutex);
}

/* 8051 implementation */
kernel_pid_t gnrc_ipv6_netif_find_by_addr(ipv6_addr_t **out, const ipv6_addr_t *addr)
{
    int i = 0;
    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        if (out != NULL) {
            *out = gnrc_ipv6_netif_find_addr(ipv6_ifs[i].pid, addr);

            if (*out != NULL) {
                DEBUG("ipv6 netif: Found %s on interface %" PRIkernel_pid "\n",
                      ipv6_addr_to_str(addr_str, *out, sizeof(addr_str)),
                      ipv6_ifs[i].pid);
                return ipv6_ifs[i].pid;
            }
        }
        else {
            if (gnrc_ipv6_netif_find_addr(ipv6_ifs[i].pid, addr) != NULL) {
                DEBUG("ipv6 netif: Found :: on interface %" PRIkernel_pid "\n",
                      ipv6_ifs[i].pid);
                return ipv6_ifs[i].pid;
            }
        }
    }

    if (out != NULL) {
        *out = NULL;
    }

    ///return KERNEL_PID_UNDEF;
    return 0;
}

/* 8051 implementation */
ipv6_addr_t *gnrc_ipv6_netif_find_addr(kernel_pid_t pid, const ipv6_addr_t *addr)
{
    int i = 0;
    gnrc_ipv6_netif_t *entry = gnrc_ipv6_netif_get(pid);

    if (entry == NULL) {
        return NULL;
    }

    mutex_lock(&entry->mutex);

    //for (i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
    for (i = 0; i < 8; i++) {
        if (ipv6_addr_equal(&(entry->addrs[i].addr), addr)) {
            mutex_unlock(&entry->mutex);
            DEBUG("ipv6 netif: Found %s on interface %" PRIkernel_pid "\n",
                  ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)),
                  pid);
            return &(entry->addrs[i].addr);
        }
    }

    mutex_unlock(&entry->mutex);

    return NULL;
}

uint8_t _find_by_prefix_unsafe(ipv6_addr_t **res, gnrc_ipv6_netif_t *iface,
                                      const ipv6_addr_t *addr, uint8_t *only)
{   
    int i = 0;
    uint8_t best_match = 0;

    //for (i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
    for (i = 0; i < 8; i++) {
        uint8_t match;

        if ((only != NULL) && !(bf_isset(only, i))) {
            continue;
        }

        if (((only != NULL) &&
             gnrc_ipv6_netif_addr_is_non_unicast(&(iface->addrs[i].addr))) ||
            ipv6_addr_is_unspecified(&(iface->addrs[i].addr))) {
            continue;
        }

        match = ipv6_addr_match_prefix(&(iface->addrs[i].addr), addr);

        if ((only == NULL) && !ipv6_addr_is_multicast(addr) &&
            (match < iface->addrs[i].prefix_len)) {
            /* match but not of same subnet */
            continue;
        }

        if (match > best_match) {
            if (res != NULL) {
                *res = &(iface->addrs[i].addr);
            }

            best_match = match;
        }
    }

#if ENABLE_DEBUG
    if (*res != NULL) {
        DEBUG("ipv6 netif: Found %s on interface %" PRIkernel_pid " matching ",
              ipv6_addr_to_str(addr_str, *res, sizeof(addr_str)),
              iface->pid);
        DEBUG("%s by %" PRIu8 " bits (used as source address = %s)\n",
              ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)),
              best_match,
              (only != NULL) ? "true" : "false");
    }
    else {
        DEBUG("ipv6 netif: Did not found any address on interface %" PRIkernel_pid
              " matching %s (used as source address = %s)\n",
              iface->pid,
              ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)),
              (only != NULL) ? "true" : "false");
    }
#endif

    return best_match;
}

/* 8051 implementation */
kernel_pid_t gnrc_ipv6_netif_find_by_prefix(ipv6_addr_t **out, const ipv6_addr_t *prefix)
{
    int i = 0;
    uint8_t best_match = 0;
    ipv6_addr_t *tmp_res = NULL;
    //kernel_pid_t res = KERNEL_PID_UNDEF;
    kernel_pid_t res = 0;

    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        uint8_t match;

        mutex_lock(&(ipv6_ifs[i].mutex));

        match = _find_by_prefix_unsafe(&tmp_res, ipv6_ifs + i, prefix, NULL);

        if (match > best_match) {
            *out = tmp_res;
            res = ipv6_ifs[i].pid;
            best_match = match;
        }

        mutex_unlock(&(ipv6_ifs[i].mutex));
    }

#if ENABLE_DEBUG
    if (res != KERNEL_PID_UNDEF) {
        DEBUG("ipv6 netif: Found %s on interface %" PRIkernel_pid " globally matching ",
              ipv6_addr_to_str(addr_str, *out, sizeof(addr_str)),
              res);
        DEBUG("%s by %" PRIu8 " bits\n",
              ipv6_addr_to_str(addr_str, prefix, sizeof(addr_str)),
              best_match);
    }
    else {
        DEBUG("ipv6 netif: Did not found any address globally matching %s\n",
              ipv6_addr_to_str(addr_str, prefix, sizeof(addr_str)));
    }
#endif

    return res;
}

/**
 * @brief selects potential source address candidates
 * @see <a href="http://tools.ietf.org/html/rfc6724#section-4">
 *      RFC6724, section 4
 *      </a>
 * @param[in]  iface            the interface used for sending
 * @param[in]  dst              the destination address
 * @param[out] candidate_set    a bitfield representing all addresses
 *                              configured to @p iface, potential candidates
 *                              will be marked as 1
 *
 * @return false if no candidates were found
 * @return true otherwise
 *
 * @pre the interface entry and its set of addresses must not be changed during
 *      runtime of this function
 */
/* 8051 implementation */
int _create_candidate_set(gnrc_ipv6_netif_t *iface, const ipv6_addr_t *dst,
                                 uint8_t *candidate_set, bool link_local_only)
{
    int i = 0;
    int res = -1;
    (void) dst;
    DEBUG("gathering candidates\n");

    /* currently this implementation supports only addresses as source address
     * candidates assigned to this interface. Thus we assume all addresses to be
     * on interface @p iface */
    //(void) dst;

    //for (i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
    for (i = 0; i < 8; i++) {
        gnrc_ipv6_netif_addr_t *iter = &(iface->addrs[i]);

        DEBUG("Checking address: %s\n",
              ipv6_addr_to_str(addr_str, &(iter->addr), sizeof(addr_str)));

        /* "In any case, multicast addresses and the unspecified address MUST NOT
         *  be included in a candidate set."
         */
        if (ipv6_addr_is_multicast(&(iter->addr)) ||
            ipv6_addr_is_unspecified(&(iter->addr))) {
            continue;
        }

        /* Check if we only want link local addresses */
        if (link_local_only && !ipv6_addr_is_link_local(&(iter->addr))) {
            continue;
        }

        /* "For all multicast and link-local destination addresses, the set of
         *  candidate source addresses MUST only include addresses assigned to
         *  interfaces belonging to the same link as the outgoing interface."
         *
         * "For site-local unicast destination addresses, the set of candidate
         *  source addresses MUST only include addresses assigned to interfaces
         *  belonging to the same site as the outgoing interface."
         *  -> we should also be fine, since we're only iterating addresses of
         *     the sending interface
         */

        /* put all other addresses into the candidate set */
        DEBUG("add to candidate set\n");
        bf_set(candidate_set, i);
        res = i;
    }

    return res;
}

ipv6_addr_t *gnrc_ipv6_netif_match_prefix(kernel_pid_t pid, const ipv6_addr_t *prefix)
{
    ipv6_addr_t *res = NULL;
    gnrc_ipv6_netif_t *iface = gnrc_ipv6_netif_get(pid);

    mutex_lock(&(iface->mutex));

    if (_find_by_prefix_unsafe(&res, iface, prefix, NULL) > 0) {
        mutex_unlock(&(iface->mutex));
        return res;
    }

    mutex_unlock(&(iface->mutex));

    return NULL;
}

/**
 * @brief Determines the scope of the given address.
 *
 * @param[in] addr              The IPv6 address to check.
 * @param[in] maybe_multicast   False if @p addr is definitely no multicast
 *                              address, true otherwise.
 *
 * @return The scope of the address.
 *
 * @pre address is not loopback or unspecified.
 * see http://tools.ietf.org/html/rfc6724#section-4
 */
uint8_t _get_scope(const ipv6_addr_t *addr, const bool maybe_multicast)
{
    if (maybe_multicast && ipv6_addr_is_multicast(addr)) {
        return (addr->u8[1] & 0x0f);
    }
    else if (ipv6_addr_is_link_local(addr)) {
        //return IPV6_ADDR_MCAST_SCP_LINK_LOCAL;
	return 0x2;
    }
    else if (ipv6_addr_is_site_local(addr)) {
        //return IPV6_ADDR_MCAST_SCP_SITE_LOCAL;
	return 0x5;
    }
    else {
        //return IPV6_ADDR_MCAST_SCP_GLOBAL;
	return 0xe;
    }
}

/** @brief Find the best candidate among the configured addresses
 *          for a certain destination address according to the 8 rules
 *          specified in RFC 6734, section 5.
 * @see <a href="http://tools.ietf.org/html/rfc6724#section-5">
 *      RFC6724, section 5
 *      </a>
 *
 * @param[in] iface              The interface for sending.
 * @param[in] dst                The destination IPv6 address.
 * @param[in, out] candidate_set The preselected set of candidate addresses as
 *                               a bitfield.
 *
 * @pre @p dst is not unspecified.
 *
 * @return The best matching candidate found on @p iface, may be NULL if none
 *         is found.
 */
/* 8051 implementation */
ipv6_addr_t *_source_address_selection(gnrc_ipv6_netif_t *iface, const ipv6_addr_t *dst,
                                              uint8_t *candidate_set)
{
    /* create temporary set for assigning "points" to candidates wining in the
     * corresponding rules.
     */
    uint8_t max_pts = 0;
    uint8_t dst_scope = 0;
    int i = 0;
    uint8_t candidate_scope = 0;
    ipv6_addr_t *res = NULL;
    uint8_t winner_set[8];
    memset(winner_set, 0, 8);
    //uint8_t winner_set[GNRC_IPV6_NETIF_ADDR_NUMOF];
    //memset(winner_set, 0, GNRC_IPV6_NETIF_ADDR_NUMOF);


    /* _create_candidate_set() assures that `dest` is not unspecified and if
     * `dst` is loopback rule 1 will fire anyway.  */
    dst_scope = _get_scope(dst, true);
    DEBUG("finding the best match within the source address candidates\n");

    //for (i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
    for (i = 0; i < 8; i++) {
        gnrc_ipv6_netif_addr_t *iter = &(iface->addrs[i]);
        DEBUG("Checking address: %s\n",
              ipv6_addr_to_str(addr_str, &(iter->addr), sizeof(addr_str)));
        /* entries which are not  part of the candidate set can be ignored */
        if (!(bf_isset(candidate_set, i))) {
            DEBUG("Not part of the candidate set - skipping\n");
            continue;
        }

        /* Rule 1: if we have an address configured that equals the destination
         * use this one as source */
        if (ipv6_addr_equal(&(iter->addr), dst)) {
            DEBUG("Ease one - rule 1\n");
            return &(iter->addr);
        }

        /* Rule 2: Prefer appropriate scope. */
        /* both link local */
        candidate_scope = _get_scope(&(iter->addr), false);
        if (candidate_scope == dst_scope) {
            DEBUG("winner for rule 2 (same scope) found\n");
            winner_set[i] += 4;//RULE_2A_PTS;
            if (winner_set[i] > max_pts) {
                max_pts = 4; //RULE_2A_PTS;
            }
        }
        else if (candidate_scope < dst_scope) {
            DEBUG("winner for rule 2 (smaller scope) found\n");
            winner_set[i] += 2; //RULE_2B_PTS;
            if (winner_set[i] > max_pts) {
                max_pts = winner_set[i];
            }
        }

        /* Rule 3: Avoid deprecated addresses. */
        if (iter->preferred > 0) {
            DEBUG("winner for rule 3 found\n");
            winner_set[i] += 1;//RULE_3_PTS;
            if (winner_set[i] > max_pts) {
                max_pts = winner_set[i];
            }
        }

        /* Rule 4: Prefer home addresses.
         * Does not apply, gnrc does not support Mobile IP.
         * TODO: update as soon as gnrc supports Mobile IP
         */

        /* Rule 5: Prefer outgoing interface.
         * RFC 6724 says:
         * "It is RECOMMENDED that the candidate source addresses be the set of
         *  unicast addresses assigned to the interface that will be used to
         *  send to the destination (the "outgoing" interface).  On routers,
         *  the candidate set MAY include unicast addresses assigned to any
         *  interface that forwards packets, subject to the restrictions
         *  described below."
         *  Currently this implementation uses ALWAYS source addresses assigned
         *  to the outgoing interface. Hence, Rule 5 is always fulfilled.
         */

        /* Rule 6: Prefer matching label.
         * Flow labels are currently not supported by gnrc.
         * TODO: update as soon as gnrc supports flow labels
         */

        /* Rule 7: Prefer temporary addresses.
         * Temporary addresses are currently not supported by gnrc.
         * TODO: update as soon as gnrc supports temporary addresses
         */
    }

    /* reset candidate set to mark winners */
    memset(candidate_set, 0, 15/8);
    //memset(candidate_set, 0, (GNRC_IPV6_NETIF_ADDR_NUMOF + 7) / 8);
    /* check if we have a clear winner */
    /* collect candidates with maximum points */
    //for (i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
    for (i = 0; i < 8; i++) {
        if (winner_set[i] == max_pts) {
            bf_set(candidate_set, i);
        }
    }

    /* otherwise apply rule 8: Use longest matching prefix. */
    
    _find_by_prefix_unsafe(&res, iface, dst, candidate_set);
    return res;
}

/* 8051 implementation */
ipv6_addr_t *gnrc_ipv6_netif_find_best_src_addr(kernel_pid_t pid, const ipv6_addr_t *dst, bool ll_only)
{
    int first_candidate = 0;
    ipv6_addr_t *best_src = NULL;
    BITFIELD(candidate_set, 8);
    //BITFIELD(candidate_set, GNRC_IPV6_NETIF_ADDR_NUMOF);

    gnrc_ipv6_netif_t *iface = gnrc_ipv6_netif_get(pid);    
    mutex_lock(&(iface->mutex));
    //BITFIELD(candidate_set, GNRC_IPV6_NETIF_ADDR_NUMOF);
    memset(candidate_set, 0, sizeof(candidate_set));

    first_candidate = _create_candidate_set(iface, dst, candidate_set, ll_only);
    if (first_candidate >= 0) {
        best_src = _source_address_selection(iface, dst, candidate_set);
        if (best_src == NULL) {
            best_src = &(iface->addrs[first_candidate].addr);
        }
    }
    mutex_unlock(&(iface->mutex));

    return best_src;
}

bool gnrc_ipv6_netif_addr_is_non_unicast(const ipv6_addr_t *addr)
{
    //return (bool)(container_of(addr, gnrc_ipv6_netif_addr_t, addr)->flags &
                  //GNRC_IPV6_NETIF_ADDR_FLAGS_NON_UNICAST);
    return (bool)(gnrc_ipv6_netif_addr_get(addr)->flags & 0x01);
}

/* 8051 implementation */
void gnrc_ipv6_netif_init_by_dev(void)
{
    //kernel_pid_t ifs[GNRC_NETIF_NUMOF];
    kernel_pid_t ifs[1];
    uint32_t ifnum = gnrc_netif_get(ifs);
#ifdef MODULE_GNRC_SIXLOWPAN_ND_BORDER_ROUTER
    bool abr_init = false;
#endif
    uint32_t i = 0;
    for (i = 0; i < ifnum; i++) {
        ipv6_addr_t addr;
        eui64_t iid;
        uint16_t tmp;
        gnrc_ipv6_netif_t *ipv6_if = gnrc_ipv6_netif_get(ifs[i]);

        if (ipv6_if == NULL) {
            continue;
        }

        mutex_lock(&ipv6_if->mutex);

#ifdef MODULE_GNRC_SIXLOWPAN
        gnrc_nettype_t if_type = GNRC_NETTYPE_UNDEF;

        if ((gnrc_netapi_get(ifs[i], NETOPT_PROTO, 0, &if_type,
                             sizeof(if_type)) != -ENOTSUP) &&
            (if_type == GNRC_NETTYPE_SIXLOWPAN)) {
            uint16_t src_len = 8;
            uint16_t max_frag_size = UINT16_MAX;

            DEBUG("ipv6 netif: Set 6LoWPAN flag\n");
            ipv6_ifs[i].flags |= GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN;

            /* the router flag must be set early here, because otherwise
             * _add_addr_to_entry() wouldn't set the solicited node address.
             * However, addresses have to be configured before calling
             * gnrc_ipv6_netif_set_router().
             */
#ifdef MODULE_GNRC_SIXLOWPAN_ND_ROUTER
            DEBUG("ipv6 netif: Set router flag\n");
            ipv6_ifs[i].flags |= GNRC_IPV6_NETIF_FLAGS_ROUTER;
#endif
            /* use EUI-64 (8-byte address) for IID generation and for sending
             * packets */
            gnrc_netapi_set(ifs[i], NETOPT_SRC_LEN, 0, &src_len,
                            sizeof(src_len)); /* don't care for result */

            if (gnrc_netapi_get(ifs[i], NETOPT_MAX_PACKET_SIZE,
                                0, &max_frag_size, sizeof(max_frag_size)) < 0) {
                /* if error we assume it works */
                DEBUG("ipv6 netif: Can not get max packet size from interface %"
                      PRIkernel_pid "\n", ifs[i]);
            }

            gnrc_sixlowpan_netif_add(ifs[i], max_frag_size);
        }
#endif

        /* set link-local address */
        if (gnrc_netapi_get(ifs[i], NETOPT_IPV6_IID, 0, &iid,
                            sizeof(eui64_t)) >= 0) {
            ipv6_addr_set_aiid(&addr, iid.uint8);
            ipv6_addr_set_link_local_prefix(&addr);
            _add_addr_to_entry(ipv6_if, &addr, 64, 0);

        }

        /* set link MTU */
        if ((gnrc_netapi_get(ifs[i], NETOPT_MAX_PACKET_SIZE, 0, &tmp,
                             sizeof(uint16_t)) >= 0)) {
            //if (tmp >= IPV6_MIN_MTU) {
	    if (tmp >= 1280) {
                ipv6_if->mtu = tmp;
            }
            /* otherwise leave at GNRC_IPV6_NETIF_DEFAULT_MTU as initialized in
             * gnrc_ipv6_netif_add() */
        }

        if (gnrc_netapi_get(ifs[i], NETOPT_IS_WIRED, 0, NULL, 0) > 0) {
            //ipv6_if->flags |= GNRC_IPV6_NETIF_FLAGS_IS_WIRED;
	    ipv6_if->flags |= 0x0080;
        }
        else {
            //ipv6_if->flags &= ~GNRC_IPV6_NETIF_FLAGS_IS_WIRED;
	    ipv6_if->flags &= ~0x0080;
        }

        mutex_unlock(&ipv6_if->mutex);
#if (defined(MODULE_GNRC_NDP_ROUTER) || defined(MODULE_GNRC_SIXLOWPAN_ND_ROUTER))
        gnrc_ipv6_netif_set_router(ipv6_if, true);
#endif
#ifdef MODULE_GNRC_SIXLOWPAN_ND
        if (ipv6_if->flags & GNRC_IPV6_NETIF_FLAGS_SIXLOWPAN) {
#ifdef MODULE_GNRC_SIXLOWPAN_ND_BORDER_ROUTER
            /* first interface wins */
            if (!abr_init) {
                gnrc_sixlowpan_nd_router_abr_create(&addr, 0);
                /* XXX should be set to true if there ever is an hard-coded
                 * prefix set */
                gnrc_ipv6_netif_set_rtr_adv(ipv6_if, false);
                abr_init = true;
            }
#endif
            gnrc_sixlowpan_nd_init(ipv6_if);
            continue;   /* skip gnrc_ndp_host_init() */
        }
#endif
#ifdef MODULE_GNRC_NDP_HOST
        /* start periodic router solicitations */
        gnrc_ndp_host_init(ipv6_if);
#endif
    }
}

#ifdef MODULE_NETSTATS_IPV6
netstats_t *gnrc_ipv6_netif_get_stats(kernel_pid_t pid)
{
    gnrc_ipv6_netif_t *iface = gnrc_ipv6_netif_get(pid);
    return &(iface->stats);
}
#endif

gnrc_ipv6_netif_addr_t *gnrc_ipv6_netif_addr_get(const ipv6_addr_t *addr)
{
    //return container_of(addr, gnrc_ipv6_netif_addr_t, addr); 
    return ((gnrc_ipv6_netif_addr_t*) ((char*) (addr) - offsetof(gnrc_ipv6_netif_addr_t, addr)));
}

/**
 * @}
 */
