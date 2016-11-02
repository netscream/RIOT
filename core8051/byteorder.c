/*
 * Copyright (C) 2014 René Kijewski
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup     core_util
 * @{
 *
 * @file
 * @brief          Functions to work with different byte orders.
 *
 * @author         René Kijewski <rene.kijewski@fu-berlin.de>
 */

#include "byteorder.h"

/* **************************** IMPLEMENTATION ***************************** */

/* 8051 implementation */
//#ifdef HAVE_NO_BUILTIN_BSWAP16
static unsigned short builtin_bswap16(unsigned short a)
{
    return (a<<8)|(a>>8);
}
/* 8051 implementation */
static unsigned long builtin_bswap32(unsigned long a)
{
    return (a<<24)|(a<<16)|(a>>24)|(a>>16);
}

//#endif

static uint16_t byteorder_swaps(uint16_t v)
{
#ifndef MODULE_MSP430_COMMON
    return builtin_bswap16(v);
#else
    network_uint16_t result = { v };
    uint8_t tmp = result.u8[0];
    result.u8[0] = result.u8[1];
    result.u8[1] = tmp;
    return result.u16;
#endif
}

static uint32_t byteorder_swapl(uint32_t v)
{
    return builtin_bswap32(v);
}

/* 8051 implementation */
/*static inline uint64_t byteorder_swapll(uint64_t v)
{
    return __builtin_bswap64(v);
}*/
/* 8051 implementation */
static be_uint16_t *byteorder_ltobs(le_uint16_t *v)
{
    be_uint16_t result = { byteorder_swaps(v->u16) };
    return &result;
}
/* 8051 implementation */
static be_uint32_t *byteorder_ltobl(le_uint32_t *v)
{
    be_uint32_t result = { byteorder_swapl(v->u32) };
    return &result;
}

/* 8051 implementation */
/*static inline be_uint64_t byteorder_ltobll(le_uint64_t v)
{
    be_uint64_t result = { byteorder_swapll(v.u64) };
    return result;
}*/
/* 8051 implementation */
static le_uint16_t *byteorder_btols(be_uint16_t *v)
{
    le_uint16_t result = { byteorder_swaps(v->u16) };
    return &result;
}
/* 8051 implementation */
static le_uint32_t *byteorder_btoll(be_uint32_t *v)
{
    le_uint32_t result = { byteorder_swapl(v->u32) };
    return &result;
}
/* 8051 implementation */
/*static inline le_uint64_t *byteorder_btolll(be_uint64_t *v)
{
    le_uint64_t result = { byteorder_swapll(v->u64) };
    return &result;
}*/

/**
 * @brief Swaps the byteorder according to the endianess
 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#   define _byteorder_swap(V, T) (byteorder_swap##T((V)))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#   define _byteorder_swap(V, T) (V)
#else
#   error "Byte order is neither little nor big!"
#endif

static network_uint16_t *byteorder_htons(uint16_t v)
{
    network_uint16_t result = { _byteorder_swap(v, s) };
    return &result;
}
static network_uint32_t *byteorder_htonl(uint32_t v)
{
    network_uint32_t result = { _byteorder_swap(v, l) };
    return &result;
}
/* 8051 implementation */
/*static inline network_uint64_t byteorder_htonll(uint64_t v)
{
    network_uint64_t result = { _byteorder_swap(v, ll) };
    return result;
}*/
/* 8051 implementation */
static uint16_t byteorder_ntohs(network_uint16_t *v)
{
    return _byteorder_swap(v->u16, s);
}
/* 8051 implementation */
static uint32_t byteorder_ntohl(network_uint32_t *v)
{
    return _byteorder_swap(v->u32, l);
}
/* 8051 implementation */
/*static inline uint64_t byteorder_ntohll(network_uint64_t *v)
{
    return _byteorder_swap(v->u64, ll);
}*/
/* 8051 implementation */
static uint16_t HTONS(uint16_t v)
{
    return byteorder_htons(v)->u16;
}
/* 8051 implementation */
static uint32_t HTONL(uint32_t v)
{
    return byteorder_htonl(v)->u32;
}
/* 8051 implementation */
/*static inline uint64_t HTONLL(uint64_t v)
{
    return byteorder_htonll(v).u64;
}*/
/* 8051 implementation */
static uint16_t NTOHS(uint16_t v)
{
    network_uint16_t input = { v };
    return byteorder_ntohs(&input);
}
/* 8051 implementation */
static uint32_t NTOHL(uint32_t v)
{
    network_uint32_t input = { v };
    return byteorder_ntohl(&input);
}
/* 8051 implementation */
/*static inline uint64_t NTOHLL(uint64_t v)
{
    network_uint64_t input = { v };
    return byteorder_ntohll(input);
}*/

