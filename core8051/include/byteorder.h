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

#ifndef BYTEORDER_H_
#define BYTEORDER_H_

#include <stdint.h>

/*#if defined(__MACH__)
#   include "clang_compat.h"
#endif*/

/*#ifdef __cplusplus
extern "C" {
#endif*/

/* ******************************* INTERFACE ******************************* */


/**
 * @brief          A 16 bit integer in little endian.
 * @details        This is a wrapper around an uint16_t to catch missing conversions
 *                 between different byte orders at compile time.
 */
typedef union {
    uint16_t    u16;    /**< 16 bit representation */
    uint8_t      u8[2]; /**< 8 bit representation */
} le_uint16_t;

/**
 * @brief          A 32 bit integer in little endian.
 * @details        This is a wrapper around an uint32_t to catch missing conversions
 *                 between different byte orders at compile time.
 */
typedef union {
    uint32_t    u32;    /**< 32 bit representation */
    uint8_t      u8[4]; /**< 8 bit representation */
    uint16_t    u16[2]; /**< 16 bit representation */
    le_uint16_t l16[2]; /**< little endian 16 bit representation */
} le_uint32_t;

/**
 * @brief          A 64 bit integer in little endian.
 * @details        This is a wrapper around an uint64_t to catch missing conversions
 *                 between different byte orders at compile time.
 */
typedef union {
    //uint64_t    u64;    /**< 64 bit representation */
    uint8_t      u8[8]; /**< 8 bit representation */
    uint16_t    u16[4]; /**< 16 bit representation */
    uint32_t    u32[2]; /**< 32 bit representation */
    le_uint16_t l16[4]; /**< little endian 16 bit representation */
    le_uint32_t l32[2]; /**< little endian 32 bit representation */
} le_uint64_t;

/**
 * @brief          A 16 bit integer in big endian aka network byte order.
 * @details        This is a wrapper around an uint16_t to catch missing conversions
 *                 between different byte orders at compile time.
 */
typedef union {
    uint16_t    u16;    /**< 16 bit representation */
    uint8_t      u8[2]; /**< 8 bit representation */
} be_uint16_t;

/**
 * @brief          A 32 bit integer in big endian aka network byte order.
 * @details        This is a wrapper around an uint32_t to catch missing conversions
 *                 between different byte orders at compile time.
 */
typedef union {
    uint32_t    u32;    /**< 32 bit representation */
    uint8_t      u8[4]; /**< 8 bit representation */
    uint16_t    u16[2]; /**< 16 bit representation */
    be_uint16_t b16[2]; /**< big endian 16 bit representation */
} be_uint32_t;

/**
 * @brief          A 64 bit integer in big endian aka network byte order.
 * @details        This is a wrapper around an uint64_t to catch missing conversions
 *                 between different byte orders at compile time.
 */
typedef union {
    //uint64_t    u64;    /**< 64 bit representation */
    uint8_t      u8[8]; /**< 8 bit representation */
    uint16_t    u16[4]; /**< 16 bit representation */
    uint32_t    u32[2]; /**< 32 bit representation */
    be_uint16_t b16[4]; /**< big endian 16 bit representation */
    be_uint32_t b32[2]; /**< big endian 32 bit representation */
} be_uint64_t;

/**
 * @brief A 16 bit integer in network byte order.
 */
typedef be_uint16_t network_uint16_t;

/**
 * @brief A 32 bit integer in network byte order.
 */
typedef be_uint32_t network_uint32_t;

/**
 * @brief A 64 bit integer in network byte order.
 */
typedef be_uint64_t network_uint64_t;

/**
 * @brief          Convert from little endian to big endian, 16 bit.
 * @param[in]      v   The integer in little endian.
 * @returns        `v` converted to big endian.
 */
/* 8051 implementation */
static be_uint16_t *byteorder_ltobs(le_uint16_t *v);

/**
 * @brief          Convert from little endian to big endian, 32 bit.
 * @param[in]      v   The integer in little endian.
 * @returns        `v` converted to big endian.
 */
/* 8051 implementation */
static be_uint32_t *byteorder_ltobl(le_uint32_t *v);

/**
 * @brief          Convert from little endian to big endian, 64 bit.
 * @param[in]      v   The integer in little endian.
 * @returns        `v` converted to big endian.
 */
/* 8051 implementation */
static be_uint64_t *byteorder_ltobll(le_uint64_t *v);

/**
 * @brief          Convert from big endian to little endian, 16 bit.
 * @param[in]      v   The integer in big endian.
 * @returns        `v` converted to little endian.
 */
/* 8051 implementation */
static le_uint16_t *byteorder_btols(be_uint16_t *v);

/**
 * @brief          Convert from big endian to little endian, 32 bit.
 * @param[in]      v   The integer in big endian.
 * @returns        `v` converted to little endian.
 */
/* 8051 implementation */
static le_uint32_t *byteorder_btoll(be_uint32_t *v);

/**
 * @brief          Convert from big endian to little endian, 64 bit.
 * @param[in]      v   The integer in big endian.
 * @returns        `v` converted to little endian.
 */
/* 8051 implementation */
static le_uint64_t *byteorder_btolll(be_uint64_t *v);

/**
 * @brief          Convert from host byte order to network byte order, 16 bit.
 * @param[in]      v   The integer in host byte order.
 * @returns        `v` converted to network byte order.
 */
/* 8051 implementation */
static network_uint16_t *byteorder_htons(uint16_t v);

/**
 * @brief          Convert from host byte order to network byte order, 32 bit.
 * @param[in]      v   The integer in host byte order.
 * @returns        `v` converted to network byte order.
 */
/* 8051 implementation */
static network_uint32_t *byteorder_htonl(uint32_t v);

/**
 * @brief          Convert from host byte order to network byte order, 64 bit.
 * @param[in]      v   The integer in host byte order.
 * @returns        `v` converted to network byte order.
 */
/* 8051 implementation */
//static inline network_uint64_t *byteorder_htonll(uint64_t v);

/**
 * @brief          Convert from network byte order to host byte order, 16 bit.
 * @param[in]      v   The integer in network byte order.
 * @returns        `v` converted to host byte order.
 */
/* 8051 implementation */
static uint16_t byteorder_ntohs(network_uint16_t *v);

/**
 * @brief          Convert from network byte order to host byte order, 32 bit.
 * @param[in]      v   The integer in network byte order.
 * @returns        `v` converted to host byte order.
 */
/* 8051 implementation */
static uint32_t byteorder_ntohl(network_uint32_t *v);

/**
 * @brief          Convert from network byte order to host byte order, 64 bit.
 * @param[in]      v   The integer in network byte order.
 * @returns        `v` converted to host byte order.
 */
/* 8051 implementation */
//static inline uint64_t byteorder_ntohll(network_uint64_t *v);

/**
 * @brief          Swap byte order, 16 bit.
 * @param[in]      v   The integer to swap.
 * @returns        The swapped integer.
 */
/* 8051 implementation */
static uint16_t byteorder_swaps(uint16_t v);

/**
 * @brief          Swap byte order, 32 bit.
 * @param[in]      v   The integer to swap.
 * @returns        The swapped integer.
 */
/* 8051 implementation */
static uint32_t byteorder_swapl(uint32_t v);

/**
 * @brief          Swap byte order, 64 bit.
 * @param[in]      v   The integer to swap.
 * @returns        The swapped integer.
 */
/* 8051 implementation */
//static inline uint64_t byteorder_swapll(uint64_t v);

/**
 * @brief          Convert from host byte order to network byte order, 16 bit.
 * @see            byteorder_htons()
 * @param[in]      v   The integer to convert.
 * @returns        Converted integer.
 */
/* 8051 implementation */
static uint16_t HTONS(uint16_t v);

/**
 * @brief          Convert from host byte order to network byte order, 32 bit.
 * @see            byteorder_htonl()
 * @param[in]      v   The integer to convert.
 * @returns        Converted integer.
 */
static uint32_t HTONL(uint32_t v);

/**
 * @brief          Convert from host byte order to network byte order, 64 bit.
 * @see            byteorder_htonll()
 * @param[in]      v   The integer to convert.
 * @returns        Converted integer.
 */
/* 8051 implementation */
//static inline uint64_t HTONLL(uint64_t v);

/**
 * @brief          Convert from network byte order to host byte order, 16 bit.
 * @see            byteorder_ntohs()
 * @param[in]      v   The integer to convert.
 * @returns        Converted integer.
 */
/* 8051 implementation */
static uint16_t NTOHS(uint16_t v);

/**
 * @brief          Convert from network byte order to host byte order, 32 bit.
 * @see            byteorder_ntohl()
 * @param[in]      v   The integer to convert.
 * @returns        Converted integer.
 */
/* 8051 implementation */
static uint32_t NTOHL(uint32_t v);

/**
 * @brief          Convert from network byte order to host byte order, 64 bit.
 * @see            byteorder_ntohll()
 * @param[in]      v   The integer to convert.
 * @returns        Converted integer.
 */
/* 8051 implementation */
//static inline uint64_t NTOHLL(uint64_t v);

/*#ifdef __cplusplus
}
#endif*/

#endif /* BYTEORDER_H_ */
/** @} */
