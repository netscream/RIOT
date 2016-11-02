/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     core_thread
 * @{
 *
 * @file
 * @brief       Threading implementation
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <errno.h>
#include <stdio.h>

#include "assert.h"
#include "thread.h"
#include "irq.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"
#include "bitarithm.h"
#include "sched.h"

volatile thread_t *thread_get(kernel_pid_t pid)
{
    if (pid_is_valid(pid)) {
        return sched_threads[pid];
    }
    return NULL;
}

int thread_getstatus(kernel_pid_t pid)
{
    volatile thread_t *t = thread_get(pid);
    return t ? (int) t->status : STATUS_NOT_FOUND;
}

/*#ifdef DEVELHELP
const char *thread_getname(kernel_pid_t pid)
{
    volatile thread_t *t = thread_get(pid);
    return t ? t->name : NULL;
}
#endif*/

void thread_sleep(void)
{
    unsigned state;
    if (irq_is_in()) {
        return;
    }
    //8051 implementation
    //unsigned state = irq_disable();
    state = irq_disable();
    sched_set_status((thread_t *)sched_active_thread, STATUS_SLEEPING);
    irq_restore(irq_disable());
    thread_yield_higher();
}

//8051 implementation
int thread_wakeup(kernel_pid_t pid)
{
    thread_t *other_thread;
    unsigned old_state;
    DEBUG("thread_wakeup: Trying to wakeup PID %" PRIkernel_pid "...\n", pid);

    //unsigned old_state = irq_disable();
    old_state = irq_disable();

    other_thread = (thread_t *) thread_get(pid);
      
    if (!other_thread) {
        DEBUG("thread_wakeup: Thread does not exist!\n");
    }
    else if (other_thread->status == STATUS_SLEEPING) {
        DEBUG("thread_wakeup: Thread is sleeping.\n");

        sched_set_status(other_thread, STATUS_RUNNING);

        irq_restore(old_state);
        //irq_restore(irq_disable());
	//sched_switch(thread_get(pid)->priority);
        sched_switch(other_thread->priority);

        return 1;
    }
    else {
        DEBUG("thread_wakeup: Thread is not sleeping!\n");
    }

    irq_restore(old_state);
    return STATUS_NOT_FOUND;
}

/* 8051 implementation */
static kernel_pid_t thread_getpid(void)
{
    extern volatile kernel_pid_t sched_active_pid;
    return sched_active_pid;
}

void thread_yield(void)
{
    unsigned old_state = irq_disable();
    thread_t *me = (thread_t *)sched_active_thread;
    if (me->status >= STATUS_ON_RUNQUEUE) {
        clist_lpoprpush(&sched_runqueues[me->priority]);
    }
    irq_restore(old_state);

    thread_yield_higher();
}

void thread_add_to_list(list_node_t *list, thread_t *thread)
{
    assert (thread->status < STATUS_ON_RUNQUEUE);
    //8051 implementation
    //uint16_t my_prio = thread->priority;
    //list_node_t *new_node = (list_node_t*)&thread->rq_entry;

    while (list->next) {
        //thread_t *list_entry = container_of((clist_node_t*)list->next, thread_t, rq_entry);
        thread_t *list_entry = ((thread_t*) ((char*) ((clist_node_t*)list->next) - offsetof(thread_t, rq_entry)));
        //if (list_entry->priority > my_prio) {
        if (list_entry->priority > thread->priority) {
            break;
        }
        list = list->next;
    }

    //new_node->next = list->next;
    thread->rq_entry.next = list->next;
    list->next = (list_node_t*) thread->rq_entry;
    //list->next = new_node;
}

/*#ifdef DEVELHELP
uintptr_t thread_measure_stack_free(char *stack)
{
    uintptr_t *stackp = (uintptr_t *)stack;
*/
    /* assume that the comparison fails before or after end of stack */
    /* assume that the stack grows "downwards" */
  /*  while (*stackp == (uintptr_t) stackp) {
        stackp++;
    }

    uintptr_t space_free = (uintptr_t) stackp - (uintptr_t) stack;
    return space_free;
}
#endif*/

//8051 implementation
kernel_pid_t thread_create(char *stack, int stacksize, char priority, int flags, thread_task_func_t function, void *arg, const char *name)
{
    thread_t *cb;
    unsigned state;
    kernel_pid_t pid;
    kernel_pid_t i;
    uintptr_t misalignment;
    if (priority >= SCHED_PRIO_LEVELS) {
	//8051 implementation EINVAL = 22
        //return -EINVAL;
        return -22;
    }

/*#ifdef DEVELHELP
    int total_stacksize = stacksize;
#else
    (void) name;
#endif*/

    /* align the stack on a 16/32bit boundary */
    //8051 implementation
    //uintptr_t misalignment = (uintptr_t) stack % ALIGN_OF(void *);
    misalignment = (uintptr_t) stack % ALIGN_OF(void *);     
    if (misalignment) {
        misalignment = ALIGN_OF(void *) - misalignment;
        stack += misalignment;
        stacksize -= misalignment;
    }
    /*if (stack % ALIGN_OF(void *)) {
	stack += ALIGN_OF(void *) - (stack % ALIGN_OF(void *));
	stacksize -= ALIGN_OF(void *) - (stack % ALIGN_OF(void *));
    }*/
	

    /* make room for the thread control block */
    stacksize -= sizeof(thread_t);

    /* round down the stacksize to a multiple of thread_t alignments (usually 16/32bit) */
    stacksize -= stacksize % ALIGN_OF(thread_t);

    if (stacksize < 0) {
        DEBUG("thread_create: stacksize is too small!\n");
    }
    /* allocate our thread control block at the top of our stackspace */
    //8051 implementation
    //thread_t *cb = (thread_t *) (stack + stacksize);
    cb = (thread_t *) (stack + stacksize);    

#if defined(DEVELHELP) || defined(SCHED_TEST_STACK)
    if (flags & THREAD_CREATE_STACKTEST) {
        /* assign each int of the stack the value of it's address */
        uintptr_t *stackmax = (uintptr_t *) (stack + stacksize);
        uintptr_t *stackp = (uintptr_t *) stack;

        while (stackp < stackmax) {
            *stackp = (uintptr_t) stackp;
            stackp++;
        }
    }
    else {
        /* create stack guard */
        *(uintptr_t *) stack = (uintptr_t) stack;
    }
#endif

    state = irq_disable();

    //kernel_pid_t pid = KERNEL_PID_UNDEF;
    pid = KERNEL_PID_UNDEF;
    //for (kernel_pid_t i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; ++i) {
    //for (i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; ++i) {
    for (i = 1; i <= (MAXTHREADS-1); ++i) {
        if (sched_threads[i] == NULL) {
            pid = i;
            break;
        }
    }
    if (pid == KERNEL_PID_UNDEF) {
        DEBUG("thread_create(): too many threads!\n");

        irq_restore(state);
	//8051 implementation EOVERFLOW = 75
        //return -EOVERFLOW;
        return -75;
    }

    sched_threads[pid] = cb;

    cb->pid = pid;
    cb->sp = thread_stack_init(function, arg, stack, stacksize);

#if defined(DEVELHELP) || defined(SCHED_TEST_STACK)
    cb->stack_start = stack;
#endif

#ifdef DEVELHELP
    cb->stack_size = total_stacksize;
    cb->name = name;
#endif

    cb->priority = priority;
    cb->status = 0;

    cb->rq_entry.next = NULL;

#ifdef MODULE_CORE_MSG
    cb->wait_data = NULL;
    cb->msg_waiters.next = NULL;
    cib_init(&(cb->msg_queue), 0);
    cb->msg_array = NULL;
#endif

    sched_num_threads++;

    DEBUG("Created thread %s. PID: %" PRIkernel_pid ". Priority: %u.\n", name, cb->pid, priority);

    //if (flags & THREAD_CREATE_SLEEPING) {
    if (flags & 1) {
        sched_set_status(cb, STATUS_SLEEPING);
    }
    else {
        sched_set_status(cb, STATUS_PENDING);

        //if (!(flags & THREAD_CREATE_WOUT_YIELD)) {
        if (!(flags & 4)) {
            irq_restore(state);
            sched_switch(priority);
            return pid;
        }
    }

    irq_restore(state);

    return pid;
}
