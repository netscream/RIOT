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

unsigned irq_disable(void)
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
}
