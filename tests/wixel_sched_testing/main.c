/*
 * Copyright (C) 2014 Oliver Hahm <oliver.hahm@inria.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 * @file
 * @brief       Test thread_yield()
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      René Kijewski <rene.kijewski@fu-berlin.de>
 * @}
 */

#include <stdio.h>

#include "board.h"
#include "thread.h"
#include "usb_com.h"
/*  parameters */
/* CONSTS */
int32 CODE param_report_period_ms = 40;

/* variables */
uint8 XDATA report[2048];
uint16 XDATA reportLength = 0;
char XDATA snd_thread_stack[128];
uint8 XDATA first = 0;
uint8 XDATA st = 0;


void snd_thread(void* arg)
{
    printf("snd_thread running\n");
}

void putchar(char c)
{
    report[reportLength] = c;
    reportLength++;
}

void resetOutput()
{
	int i = 0;
	for (i = 0; i < reportLength; i++)
	{
		report[i] = '\0';
	}
	reportLength = 0;
}


void sendReport()
{
   if (getMs() >= param_report_period_ms)
   {
       if(reportLength > 0)
       {
            if(usbComTxAvailable() && reportLength > 0)
            {
                 usbComTxSend(report, reportLength);
                 resetOutput();
            }
       }
   }
}


int main(void)
{
    printf("The output should be: yield 1, snd_thread running, yield 2, done\n\n");

    systemInit();
    thread_create(snd_thread_stack, sizeof(snd_thread_stack), 7, 4, (void*)snd_thread, NULL, "snd");
    
    printf("yield 1\n");
    thread_yield();
    printf("yield 2\n");
    thread_yield();
    printf("done\n");

   while(1){
    	updateLeds();
	usbComService();
	sendReport();
    }
      
    return 0;
}
