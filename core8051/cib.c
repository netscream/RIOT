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
#include "cib.h"
/* 8051 implementation */
static void cib_init(cib_t *restrict cib, unsigned int size)
{
    /* check if size is a power of 2 by comparing it to its complement */
    assert(!(size & (size - 1)));

    cib->read_count = 0;
    cib->write_count = 0;
    cib->mask = (size)-1;
    /*cib_t c = CIB_INIT(size);
    *cib = c;*/
}

static unsigned int cib_avail(const cib_t *cib)
{
    return cib->write_count - cib->read_count;
}

static unsigned int cib_full(const cib_t *cib)
{
    return ((int) cib_avail(cib)) > ((int) cib->mask);
}

static int cib_get(cib_t *restrict cib)
{
    if (cib->write_count > cib->read_count) {
        return (int) (cib->read_count++ & cib->mask);
    }

    return -1;
}

static int cib_peek(cib_t *restrict cib)
{
    if (cib->write_count > cib->read_count) {
        return (int) (cib->read_count & cib->mask);
    }

    return -1;
}

static int cib_get_unsafe(cib_t *cib)
{
        return (int) (cib->read_count++ & cib->mask);
}

static int cib_put(cib_t *restrict cib)
{
    unsigned int avail = cib_avail(cib);

    /* We use a signed compare, because the mask is -1u for an empty CIB. */
    if ((int) avail <= (int) cib->mask) {
        return (int) (cib->write_count++ & cib->mask);
    }

    return -1;
}
/* 8051 implementation */ 
static int cib_put_unsafe(cib_t *cib)
{
    return (int) (cib->write_count++ & cib->mask);
}

