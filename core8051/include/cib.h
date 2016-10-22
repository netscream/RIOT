/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *               2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

 /**
 * @addtogroup  core_util
 * @{
 *
 * @file
 * @brief       Circular integer buffer interface
 * @details     This structure provides an organizational interface
 *              and combined with an memory array forms a circular buffer.
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 */

#ifndef CIB_H
#define CIB_H

#include "assert.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief circular integer buffer structure
 */
typedef struct cib_t{
    unsigned int read_count;    /**< number of (successful) read accesses */
    unsigned int write_count;   /**< number of (successful) write accesses */
    unsigned int mask;          /**< Size of buffer -1, i.e. mask of the bits */
} cib_t;

/**
 * @brief   Initialize cib_t to a given size.
 */
//#define CIB_INIT(SIZE) { 0, 0, (SIZE) - 1 }
#define CIB_INIT(SIZE) ( 0, 0, (SIZE) - 1 )
/**
 * @brief Initialize @p cib to 0 and set buffer size to @p size.
 *
 * @param[out] cib      Buffer to initialize.
 *                      Must not be NULL.
 * @param[in]  size     Size of the buffer, must not exceed MAXINT/2.
 *                      Should be equal to 0 or power of 2.
 */
static inline void cib_init(cib_t *restrict cib, unsigned int size)
{
    /* check if size is a power of 2 by comparing it to its complement */
    assert(!(size & (size - 1)));

    cib->read_count = 0;
    cib->write_count = 0;
    cib->mask = (size)-1;
    /*cib_t c = CIB_INIT(size);
    *cib = c;*/
}

/**
 * @brief Calculates difference between cib_put() and cib_get() accesses.
 *
 * @param[in] cib       the cib_t to check.
 *                      Must not be NULL.
 * @return How often cib_get() can be called before @p cib is empty.
 */
static inline unsigned int cib_avail(const cib_t *cib)
{
    return cib->write_count - cib->read_count;
}

/**
 * @brief Check if cib is full.
 *
 * @param[in] cib       the cib_t to check.
 *                      Must not be NULL.
 * @return      1 if cib_put() would return "-1", 0 otherwise
 */
static inline unsigned int cib_full(const cib_t *cib)
{
    return ((int) cib_avail(cib)) > ((int) cib->mask);
}

/**
 * @brief Get the index of the next item in buffer.
 *
 * @param[in,out] cib   corresponding *cib* to buffer.
 *                      Must not be NULL.
 * @return index of next item, -1 if the buffer is empty
 */
static inline int cib_get(cib_t *restrict cib)
{
    if (cib->write_count > cib->read_count) {
        return (int) (cib->read_count++ & cib->mask);
    }

    return -1;
}

/**
 * @brief Get the index of the next item in buffer without removing it.
 *
 * @param[in,out] cib   corresponding *cib* to buffer.
 *                      Must not be NULL.
 * @return index of next item, -1 if the buffer is empty
 */
static inline int cib_peek(cib_t *restrict cib)
{
    if (cib->write_count > cib->read_count) {
        return (int) (cib->read_count & cib->mask);
    }

    return -1;
}

/**
 * @brief Get the index of the next item in buffer.
 *
 * Unsafe version, *must not* be called if buffer is empty!
 *
 * @param[in,out] cib   corresponding *cib* to buffer.
 *                      Must not be NULL.
 * @return index of next item
 */
static inline int cib_get_unsafe(cib_t *cib)
{
        return (int) (cib->read_count++ & cib->mask);
}

/**
 * @brief Get index for item in buffer to put to.
 *
 * @param[in,out] cib   corresponding *cib* to buffer.
 *                      Must not be NULL.
 * @return index of item to put to, -1 if the buffer is full
 */
static inline int cib_put(cib_t *restrict cib)
{
    unsigned int avail = cib_avail(cib);

    /* We use a signed compare, because the mask is -1u for an empty CIB. */
    if ((int) avail <= (int) cib->mask) {
        return (int) (cib->write_count++ & cib->mask);
    }

    return -1;
}

/**
 * @brief Get index for item in buffer to put to.
 *
 * Unsafe version, *must not* be called if buffer is full!
 *
 * @param[in,out] cib   corresponding *cib* to buffer.
 *                      Must not be NULL.
 * @return index of item to put to
 */
static inline int cib_put_unsafe(cib_t *cib)
{
    return (int) (cib->write_count++ & cib->mask);
}

#ifdef __cplusplus
}
#endif

#endif /* CIB_H */
/** @} */
