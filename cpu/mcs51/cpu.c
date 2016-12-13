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
#include <stdio.h>
#include "cpu.h"
#include "irq.h"
#include "sched.h"
#include "thread.h"


/*
 *
 *
 */
void thread_yield_higher(void)
{
	//char *XDATA sp;
	//void (*foo)(void* arg);
	if (_in_isr == 0) {
		_in_isr = 1;
		irq_disable();
		sched_run();
		//sp = sched_active_thread->sp;
		sched_active_thread->function(1);	
		irq_enable();
	}	
}

/*void thread_yield_higher(void)
{
    // __asm__("push ar2");
   return;
}*/

/* This function calculates the ISR_usage */
//int thread_arch_isr_stack_usage(void)
//{
    /* TODO */
    //return -1;
//    return 0;
//}

//void *thread_arch_isr_stack_pointer(void)
//{
    /* TODO */
    //return (void *)-1;
  //  return NULL;
//}

/*void *thread_arch_isr_stack_start(void)
{
  */  /* TODO */
    //return (void *)-1;
    /*return NULL;
}*/


/*
 * stack initilization for CPU (wixel)
 *
 *
 */
char *thread_stack_init(void (*task_func)(void *arg), void* XDATA arg, void* XDATA stack_start, int XDATA stack_size)
{
    unsigned int* XDATA stk;
    char* XDATA stkptr;
    stk = (unsigned int*)((uintptr_t) stack_start+stack_size);
    //stk &= 0xfe; //stack alignment 8 bit
    stkptr = (char*) stk;
    --stkptr;
    *stkptr = (char) sched_task_exit;
    --stkptr;
    *stkptr = (char) task_func;
    --stkptr;
    *stkptr = (char) arg;
    return stkptr;
}

void reboot(void)
{
 
}

NORETURN void cpu_switch_context_exit(void)
{
    //sched_active_thread = sched_threads[0];
    //sched_run();

    //restore_context();

    //UNREACHABLE();
}

void cpu_print_last_instruction(void)
{
   //puts("n/a");
}
