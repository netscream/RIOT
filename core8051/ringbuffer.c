/**
 * Ringbuffer implementation
 *
 * Copyright (C) 2013 Freie Universität Berlin
 * Copyright (C) 2013 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * @ingroup  core_util
 * @{
 * @file
 * @author Kaspar Schleiser <kaspar@schleiser.de>
 * @author René Kijewski <rene.kijewski@fu-berlin.de>
 * @}
 */

#include "ringbuffer.h"

#include <string.h>
/* 8051 implementation */
static void ringbuffer_init(ringbuffer_t *restrict rb, char *buffer, unsigned bufsize)
{
    rb->buf = buffer;
    rb->size = bufsize;
    rb->start = 0;
    rb->avail = 0;
}

/**
 * @brief           Add an element to the end of the ringbuffer.
 * @details         This helper function does not check the pre-requirements for adding,
 *                  i.e. the caller has to ensure that ringbuffer_full() is false.
 * @param[in,out]   rb   Ringbuffer to operate on.
 * @param[in]       c    Element to add.
 */
static void add_tail(ringbuffer_t *restrict rb, char c)
{
    unsigned pos = rb->start + rb->avail++;
    if (pos >= rb->size) {
        pos -= rb->size;
    }
    rb->buf[pos] = c;
}

/**
 * @brief           Remove an element from the start of the ringbuffer.
 * @details         This helper function does not check the pre-requirements for reading,
 *                  i.e. the caller has to ensure that ringbuffer_empty() is false.
 * @param[in,out]   rb   Ringbuffer to operate on.
 * @returns         The removed element.
 */
static char get_head(ringbuffer_t *restrict rb)
{
    char result = rb->buf[rb->start];
    if ((--rb->avail == 0) || (++rb->start == rb->size)) {
        rb->start = 0;
    }
    return result;
}

unsigned ringbuffer_add(ringbuffer_t *restrict rb, const char *buf, unsigned n)
{
    unsigned i = 0;
    for (i = 0; i < n; i++) {
        //if (ringbuffer_full(rb)) {
        if (rb->avail == rb->size) {
            break;
        }
        add_tail(rb, buf[i]);
    }
    return i;
}

int ringbuffer_add_one(ringbuffer_t *restrict rb, char c)
{
    int result = -1;
    //if (ringbuffer_full(rb)) {
    if (rb->avail == rb->size) {
        result = (unsigned char) get_head(rb);
    }
    add_tail(rb, c);
    return result;
}

int ringbuffer_get_one(ringbuffer_t *restrict rb)
{
    //if (!ringbuffer_empty(rb)) {
    if (!(rb->avail == 0)) {
        return (unsigned char) get_head(rb);
    }
    else {
        return -1;
    }
}

unsigned ringbuffer_get(ringbuffer_t *restrict rb, char *buf, unsigned n)
{
    if (n > rb->avail) {
        n = rb->avail;
    }
    if (n > 0) {
        unsigned bytes_till_end = rb->size - rb->start;
        if (bytes_till_end >= n) {
            memcpy(buf, rb->buf + rb->start, n);
            if (bytes_till_end == n) {
                rb->start = 0;
            }
            else {
                rb->start += n;
            }
        }
        else {
            memcpy(buf, rb->buf + rb->start, bytes_till_end);
            rb->start = n - bytes_till_end;
            memcpy(buf + bytes_till_end, rb->buf, rb->start);
        }
        rb->avail -= n;
    }
    return n;
}

unsigned ringbuffer_remove(ringbuffer_t *restrict rb, unsigned n)
{
    if (n > rb->avail) {
        n = rb->avail;
        rb->start = rb->avail = 0;
    }
    else {
        rb->start -= n;
        rb->avail -= n;

        /* compensate underflow */
        if (rb->start > rb->size) {
            rb->start += rb->size;
        }
    }

    return n;
}

/* 8051 implementation */
/*static int ringbuffer_empty(const ringbuffer_t *restrict rb)
{
    return rb->avail == 0;
}

static int ringbuffer_full(const ringbuffer_t *restrict rb)
{
    return rb->avail == rb->size;
}

static unsigned int ringbuffer_get_free(const ringbuffer_t *restrict rb)
{
    return rb->size - rb->avail;
}


int ringbuffer_peek_one(const ringbuffer_t *restrict rb_)
{
    return ringbuffer_get_one((ringbuffer_t*)&rb_);
}

unsigned ringbuffer_peek(const ringbuffer_t *restrict rb_, char *buf, unsigned n)
{
    return ringbuffer_get((ringbuffer_t*) &rb_, buf, n);
}*/
