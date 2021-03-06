/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */


/**
 * @ingroup     core_util
 * @{
 *
 * @file
 * @brief       Generic implementation of the kernel's atomic interface
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 *
 * @}
 */

#include "irq.h"
#include "cpu.h"
#include "atomic.h"

/* Set ARCH_HAS_ATOMIC_COMPARE_AND_SWAP within cpu.h to override this function */
#if (ARCH_HAS_ATOMIC_COMPARE_AND_SWAP == 0)
int atomic_cas(atomic_int_t *var, int old, int now)
{
    unsigned int mask = irq_disable();

    if (ATOMIC_VALUE(var) != old) {
        irq_restore(mask);
        return 0;
    }

    //ATOMIC_VALUE(var) = now;
    /* 8051 implementation */
    var->value = now;
    irq_restore(mask);
    return 1;
}

/* 8051 implementation */
int atomic_inc(atomic_int_t *var)
{
    int old = 0;
    do {
        old = var->value;
    } while (!atomic_cas(var, old, old + 1));

    return old;
}

/* 8051 implementation */
int atomic_dec(atomic_int_t *var)
{
    int old = 0;
    do {
        old = var->value;
    } while (!atomic_cas(var, old, old - 1));

    return old;
}

/* 8051 implementation */
int atomic_set_to_one(atomic_int_t *var)
{
    do {
        if (var->value != 0) {
            return 0;
        }
    } while (!atomic_cas(var, 0, 1));

    return 1;
}

/* 8051 implementation */
int atomic_set_to_zero(atomic_int_t *var)
{
    int old = 0;
    do {
        old = var->value;

        if (old == 0) {
            return 0;
        }
    } while (!atomic_cas(var, old, 0));

    return 1;
}

/* 8051 implementation */
int ATOMIC_VALUE(atomic_int_t *var)
{
        return var->value;
}

#endif
