/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     core_mbox
 * @{
 *
 * @file
 * @brief       mailbox implementation
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <string.h>

#include "mbox.h"
#include "irq.h"
#include "sched.h"
#include "thread.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#ifdef MODULE_CORE_MBOX
/* 8051 implementation */
void mbox_init(mbox_t* XDATA mbox, msg_t* XDATA queue, unsigned int XDATA queue_size)
{
    mbox->msg_array->sender_pid = queue->sender_pid;
    mbox->msg_array->type = queue->type;
    mbox->msg_array->content.ptr = queue->content.ptr;
    mbox->msg_array->content.value = queue->content.value;
}
/* 8051 implementation */
void mbox_put(mbox_t* XDATA mbox, msg_t* XDATA msg)
{
    _mbox_put(mbox, msg, BLOCKING);
}
/* 8051 implementation */
int mbox_try_put(mbox_t* XDATA mbox, msg_t* XDATA msg)
{
    return _mbox_put(mbox, msg, NON_BLOCKING);
}
/* 8051 implementation */
void mbox_get(mbox_t* XDATA mbox, msg_t* XDATA msg)
{
    _mbox_get(mbox, msg, BLOCKING);
}
/* 8051 implementation */
int mbox_try_get(mbox_t* XDATA mbox, msg_t* XDATA msg)
{
    return _mbox_get(mbox, msg, NON_BLOCKING);
}

void _wake_waiter(thread_t* XDATA thread, unsigned XDATA irqstate)
{
    sched_set_status(thread, STATUS_PENDING);

    DEBUG("mbox: Thread %"PRIkernel_pid": _wake_waiter(): waking up "
            "%"PRIkernel_pid".\n", sched_active_pid, thread->pid);

    uint16_t process_priority = thread->priority;
    irq_restore(irqstate);
    sched_switch(process_priority);
}

void _wait(list_node_t* XDATA wait_list, unsigned XDATA irqstate)
{
    DEBUG("mbox: Thread %"PRIkernel_pid" _wait(): going blocked.\n",
            sched_active_pid);

    thread_t *me = (thread_t*) sched_active_thread;
    sched_set_status(me, STATUS_MBOX_BLOCKED);
    thread_add_to_list(wait_list, me);
    irq_restore(irqstate);
    thread_yield();

    DEBUG("mbox: Thread %"PRIkernel_pid" _wait(): woke up.\n",
            sched_active_pid);
}

int _mbox_put(mbox_t* XDATA mbox, msg_t* XDATA msg, int XDATA blocking)
{
    unsigned irqstate = irq_disable();

    list_node_t *next = (list_node_t*) list_remove_head(&mbox->readers);
    if (next) {
        DEBUG("mbox: Thread %"PRIkernel_pid" mbox 0x%08x: _tryput(): "
                "there's a waiter.\n", sched_active_pid, (unsigned)mbox);
        thread_t *thread = container_of((clist_node_t*)next, thread_t, rq_entry);
        *(msg_t *)thread->wait_data = *msg;
        _wake_waiter(thread, irqstate);
        return 1;
    }
    else {
        if (cib_full(&mbox->cib)) {
            if (blocking) {
                _wait(&mbox->writers, irqstate);
                irqstate = irq_disable();
            }
            else {
                irq_restore(irqstate);
                return 0;
            }
        }

        DEBUG("mbox: Thread %"PRIkernel_pid" mbox 0x%08x: _tryput(): "
                "queued message.\n", sched_active_pid, (unsigned)mbox);
        msg->sender_pid = sched_active_pid;
        /* copy msg into queue */
        mbox->msg_array[cib_put_unsafe(&mbox->cib)] = *msg;
        irq_restore(irqstate);
        return 1;
    }
}

int _mbox_get(mbox_t* XDATA mbox, msg_t* XDATA msg, int XDATA blocking)
{
    unsigned irqstate = irq_disable();

    if (cib_avail(&mbox->cib)) {
        DEBUG("mbox: Thread %"PRIkernel_pid" mbox 0x%08x: _tryget(): "
                "got queued message.\n", sched_active_pid, (unsigned)mbox);
        /* copy msg from queue */
        *msg = mbox->msg_array[cib_get_unsafe(&mbox->cib)];
        list_node_t *next = (list_node_t*) list_remove_head(&mbox->writers);
        if (next) {
            thread_t *thread = container_of((clist_node_t*)next, thread_t, rq_entry);
            _wake_waiter(thread, irqstate);
        }
        else {
            irq_restore(irqstate);
        }
        return 1;
    }
    else if (blocking) {
        sched_active_thread->wait_data = (void*)msg;
        _wait(&mbox->readers, irqstate);
        /* sender has copied message */
        return 1;
    }
    else {
        irq_restore(irqstate);
        return 0;
    }
}

#endif /* MODULE_CORE_MBOX */
