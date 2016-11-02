/*
 * Basic definitions and forwards for 8051 arch
 *
 * @ingroup 8051
 * @{
 * @file
 * @brief Cpu implementation for 8051 architecture
 * @author  Hlynur Hansen <hlynur@tolvur.net>
 * @}
 */

#include "cpu.h"
#include "irq.h"
#include "sched.h"
#include "thread.h"

/* defined for the wixel workaround from sched.h */
volatile unsigned int sched_context_switch_request;
volatile thread_t *sched_threads[MAXTHREADS];
volatile thread_t *sched_active_thread;
volatile int sched_num_threads;
volatile kernel_pid_t sched_active_pid;
clist_node_t sched_runqueues[SCHED_PRIO_LEVELS];

/*
 *
 *
 */
void thread_yield_higher(void)
{
   // __asm("push r2");
    //disable_irq();
    //irq_disable();
    //save_context();
    sched_run();
    //restore_context();
    //UNREACHABLE();
}

/* This function calculates the ISR_usage */
int thread_arch_isr_stack_usage(void)
{
    /* TODO */
    return -1;
}

void *thread_arch_isr_stack_pointer(void)
{
    /* TODO */
    return (void *)-1;
}

void *thread_arch_isr_stack_start(void)
{
    /* TODO */
    return (void *)-1;
}


/*
 * stack initilization for CPU (wixel)
 *
 *
 */
char *thread_stack_init(thread_task_func_t *task_func, void *arg, void *stack_start, int stack_size)
{
    char stk;
    char *stkptr;
    stk = (char)((uintptr_t) stack_start+stack_size);
    stk &= 0xfe; //stack alignment 8 bit
    stkptr = (char*) stk;
    --stkptr;
    *stkptr = (char) sched_task_exit;
    --stkptr;
    *stkptr = (char) task_func;
    --stkptr;
    *stkptr = (char) arg;
    return (char *) stkptr;	
}

void reboot(void)
{
 
}

NORETURN void cpu_switch_context_exit(void)
{
    sched_active_thread = sched_threads[0];
    sched_run();

    //restore_context();

    //UNREACHABLE();
}

static void cpu_print_last_instruction(void)
{
   //puts("n/a");
}
