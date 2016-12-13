/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     core_internal
 * @{
 *
 * @file
 * @brief       Platform-independent kernel initilization
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */
//8051 implementation
#include "riotbuild.h" //riot version
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "kernel_init.h"
#include "sched.h"
#include "thread.h"
#include "lpm.h"
#include "irq.h"
#include "log.h"
//#include "board.h" //for testing purposes
/*#ifdef MODULE_SCHEDSTATISTICS
#include "sched.h"
#endif

#define ENABLE_DEBUG (0)
#include "debug.h"
*/
#ifdef MODULE_AUTO_INIT
#include <auto_init.h>
#endif

volatile int lpm_prevent_sleep = 0;

extern int main(void);
void main_trampoline(void* arg)
{
    (void) arg;

#ifdef MODULE_AUTO_INIT
    auto_init();
#endif

#ifdef MODULE_SCHEDSTATISTICS
    schedstat *stat = &sched_pidlist[thread_getpid()];
    stat->laststart = 0;
#endif

    LOG_INFO("main(): This is RIOT! (Version: " RIOT_VERSION ")\n");

    main();
    //return NULL;
}

void idle_thread(void *arg)
{
    (void) arg;

    while (1) {
        if (lpm_prevent_sleep) {
            lpm_set(LPM_IDLE);
        }
        else {
            lpm_set(LPM_IDLE);
            /* lpm_set(LPM_SLEEP); */
            /* lpm_set(LPM_POWERDOWN); */
        }
    }

    //return NULL;
}

const char* CODE main_name = "main";
const char* CODE idle_name = "idle";

char XDATA main_stack[THREAD_STACKSIZE_MAIN];
char XDATA idle_stack[THREAD_STACKSIZE_IDLE];

void kernel_init(void)
{
    (void) irq_disable();
    thread_create(idle_stack, sizeof(idle_stack),
            15,
            4 | 8,
            idle_thread, NULL, idle_name);
       
    thread_create(main_stack, sizeof(main_stack),
            7,
            4 | 8,
            main_trampoline, NULL, main_name);

    cpu_switch_context_exit();
}
