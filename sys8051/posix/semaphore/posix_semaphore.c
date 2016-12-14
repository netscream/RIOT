/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 *
 * @author  Christian Mehlis <mehlis@inf.fu-berlin.de>
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 * @author  René Kijewski <kijewski@inf.fu-berlin.de>
 */

#include <errno.h>
#include <inttypes.h>

#include "irq.h"
#include "sched.h"
#include "sema.h"
#include "timex.h"
#include "thread.h"
#include "xtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#include "semaphore.h"

int sem_timedwait(sem_t* XDATA sem, struct timespec* XDATA abstime)
{
    uint32_t now, timeout = (((uint32_t)abstime->tv_sec) * SEC_IN_USEC) +
                            (abstime->tv_nsec / USEC_IN_NS);
    int res;
    now = xtimer_now();
    if (now > timeout) {
        errno = 110;//ETIMEDOUT;
        return -1;
    }
    timeout = timeout - now;
    res = sema_wait_timed((sema_t *)sem, timeout);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return 0;
}

int sem_trywait(sem_t* XDATA sem)
{
    unsigned int old_state, value;
    int result;
    if (sem == NULL) {
        //errno = EINVAL;
	errno = 22;
        return -1;
    }
    old_state = irq_disable();
    value = sem->value;
    if (value == 0) {
        //errno = EAGAIN;
	errno = 11;
        result = -1;
    }
    else {
        result = 0;
        sem->value = value - 1;
    }

    irq_restore(old_state);
    return result;
}

int sem_init(sem_t* XDATA sem, int XDATA pshared, unsigned XDATA value)
{
    int res = sema_create((sema_t *)sem, value);
    (void)pshared;
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return 0;
}

int sem_destroy(sem_t* XDATA sem)
{
    int res = sema_destroy((sema_t *)sem);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return 0;
}

int sem_post(sem_t* XDATA sem)
{
    int res = sema_post((sema_t *)sem);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return 0;
}

int sem_wait(sem_t* XDATA sem)
{
    int res = sema_wait((sema_t *)sem);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return 0;
}

sem_t *sem_open(const char* name, int oflag, ...)
{
    (void)name;
    (void)oflag;
    errno = 12;
    return SEM_FAILED;
}

int sem_close(sem_t* XDATA sem)
{
    (void)sem;
    errno = 22;
    return -1;
}

int sem_unlink(const char* XDATA name)
{
    (void)name;
    errno = 2;
    return -1;
}

int sem_getvalue(sem_t* XDATA sem, int* XDATA sval)
{
    if (sem != NULL) {
        *sval = (int)sem->value;
        return 0;
    }
    errno = 22;
    return -1;
}

/** @} */
