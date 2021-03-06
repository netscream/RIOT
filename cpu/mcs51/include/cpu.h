/*
 * Basic definitions and forwards for 8051 arch
 *
 * @defgroup 8051
 * @ingroup cpu
 * @{
 * @file
 * @brief Cpu header implementation for 8051 architecture
 * @author  Hlynur Hansen <hlynur@tolvur.net>
 * @}
 */

#ifndef MCS51_CPU_H
#define MCS51_CPU_H

#include <cc2511_types.h>
#include <cc2511_map.h>
#include <time.h>
#include <dma.h>
#include <adc.h>
#include <gpio.h>
//#include <random.h> //Some problem with random, will fix
//#define THREAD_STACKSIZE_MAIN 128
/**
 *  * @brief   Prints the last instruction's address
 *    @todo: Not supported
 *   */
//extern void cpu_print_last_instruction(void);
/*{
   //puts("n/a");
}*/
extern int XDATA _in_isr;
void cpu_print_last_instruction(void);
#endif
