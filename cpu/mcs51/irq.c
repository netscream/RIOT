/*
 * @ingroup cpu
 * @{
 *
 * @file 
 * @brief ISR related functions
 * 
 * @author Hlynur Hansen
 */

#include "cpu.h"
#include "irq.h"

/*unsigned irq_disable(void)
{
    return 0;
}

unsigned irq_enable(void)
{
    return 0;
}

void irq_restore(unsigned state)
{
}

int irq_is_in(void)
{
    return 0;
}*/

int XDATA interrupts_enabled = 1;
int XDATA _in_isr = 0;



//char XDATA __isr_stack[256];

//uint8 XDATA _saved_eip;

//static _callback_t native_irq_handlers[255];
//char XDATA sigalt_stk[128];

void* thread_arch_isr_stack_pointer(void)
{
    //return native_isr_context.uc_stack.ss_sp;
    return 0;
}

void* thread_arch_isr_stack_start(void)
{
    //return __isr_stack;
}

/**
 * block signals
 */
unsigned irq_disable(void)
{
    unsigned int prev_state;

    //_syscall_enter();
    //DEBUG("irq_disable()\n");

    //if (_in_isr == 1) {
    //    DEBUG("irq_disable + _in_isr\n");
   // }

    /*if (sigprocmask(SIG_SETMASK, &_sig_set_dint, NULL) == -1) {
        err(EXIT_FAILURE, "irq_disable: sigprocmask");
    }*/

    prev_state = interrupts_enabled;
    interrupts_enabled = 0;

    //DEBUG("irq_disable(): return\n");
    //_syscall_leave();

    return prev_state;
}

/**
 * unblock signals
 */
unsigned irq_enable(void)
{
    unsigned int prev_state;
    //_syscall_enter();

    /* Mark the IRQ as enabled first since sigprocmask could call the handler
     * before returning to userspace.
     */

    prev_state = interrupts_enabled;
    interrupts_enabled = 1;

    /*if (sigprocmask(SIG_SETMASK, &_sig_set, NULL) == -1) {
        err(EXIT_FAILURE, "irq_enable: sigprocmask");
    }*/

    //_syscall_leave();


    return prev_state;
}

void irq_restore(unsigned state)
{

    if (state == 1) {
        irq_enable();
    }
    else {
        irq_disable();
    }

    return;
}

int irq_is_in(void)
{
    return _in_isr;
}

