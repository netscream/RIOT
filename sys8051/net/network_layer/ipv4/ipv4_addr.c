#include "include/net/ipv4/addr.h"

bool ipv4_addr_equal(ipv4_addr_t *a, ipv4_addr_t *b)
{
    return (a->u32.u32 == b->u32.u32);
}

