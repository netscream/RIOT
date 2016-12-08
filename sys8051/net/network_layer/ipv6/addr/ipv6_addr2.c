#include "net/ipv6/addr.h"

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

bool ipv6_addr_is_global(const ipv6_addr_t *addr)
{
    /* first check for multicast with global scope */
    if (ipv6_addr_is_multicast(addr)) {
        //return ((addr->u8[1] & 0x0f) == IPV6_ADDR_MCAST_SCP_GLOBAL);
        return ((addr->u8[1] & 0x0f) == 0xe);
    }
    else {
        return !(ipv6_addr_is_link_local(addr) ||
                 ipv6_addr_is_unspecified(addr) ||
                 ipv6_addr_is_loopback(addr));
    }
}

bool ipv6_addr_is_solicited_node(const ipv6_addr_t *addr)
{
    return (memcmp(addr, &ipv6_addr_solicited_node_prefix,
                   sizeof(ipv6_addr_t) - 3) == 0);
}

void ipv6_addr_set_unspecified(ipv6_addr_t *addr)
{
    memset(addr, 0, sizeof(ipv6_addr_t));
}

void ipv6_addr_set_loopback(ipv6_addr_t *addr)
{
    memset(addr, 0, sizeof(ipv6_addr_t));
    addr->u8[15] = 1;
}

void ipv6_addr_set_multicast(ipv6_addr_t *addr, unsigned int flags, unsigned int scope)
{
    addr->u8[0] = 0xff;
    addr->u8[1] = (((uint8_t)flags) << 4) | (((uint8_t) scope) & 0x0f);
}

void ipv6_addr_set_all_nodes_multicast(ipv6_addr_t *addr, unsigned int scope)
{
    memcpy(addr, &ipv6_addr_all_nodes_if_local, sizeof(ipv6_addr_t));
    addr->u8[1] = (uint8_t)scope;
}

void ipv6_addr_set_all_routers_multicast(ipv6_addr_t *addr, unsigned int scope)
{
    memcpy(addr, &ipv6_addr_all_routers_if_local, sizeof(ipv6_addr_t));
    addr->u8[1] = (uint8_t)scope;
}

int ipv6_addr_split_prefix(char *addr_str)
{
    return ipv6_addr_split(addr_str, '/', 128);
}

int ipv6_addr_split_iface(char *addr_str)
{
    return ipv6_addr_split(addr_str, '%', -1);
}

