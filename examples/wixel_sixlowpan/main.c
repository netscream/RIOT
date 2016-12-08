/*
 * Copyright (C) 2015 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Showing minimum memory footprint of gnrc network stack
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 *
 * @}
 */

#include <stdio.h>

#include "board.h"
#include "msg.h"
#include "riotbuild.h"
//#include "net/sixlowpan.h"
#include "usb_com.h"
#include "net/gnrc/sixlowpan.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/ipv6/netif.h"
/*  parameters */
/* CONSTS */
int8 CODE param_report_period_ms = 40;

/* variables */
char XDATA report[1024];
uint32 XDATA reportLength;
uint16 XDATA reportBytesSent;
uint8 XDATA first = 0;
//uint8 XDATA bytesToSend = 0;
void updateLeds()
{
    usbShowStatusWithGreenLed();
    LED_YELLOW(0);
    LED_RED(0);
}   

void putchar(char c)
{
    report[reportLength] = c;
    reportLength++;
}   

void sendReport()
{
   uint16 XDATA bytesToSend = 0;
   reportBytesSent = 0;

   if (getMs() >= param_report_period_ms)
   {
       if(reportLength > 0)
       {
            bytesToSend = usbComTxAvailable();
            if(bytesToSend > reportLength - reportBytesSent)
            {
                 usbComTxSend(report+reportBytesSent, reportLength - reportBytesSent);
                 reportLength = 0;
            }
            else
            {
                 usbComTxSend(report+reportBytesSent, bytesToSend);
                 reportBytesSent += bytesToSend;
            }
       }
       else
       if(reportLength == 0 && first == 0)
       {
           //printf("Sixlowpan implementation\n\n");
	   //printf("This is a 6LoWPAN implementation\n");
           printf("You are running RIOT on a(n) %s board. Implemented with 6LoWPAN\n", RIOT_BOARD);
	   first = 1;
       }
   }
}


int main(void)
{
    kernel_pid_t pid = gnrc_sixlowpan_init();
    systemInit();
    kernel_init();
    /* main thread exits */
    while(1) {
	updateLeds();   
        usbComService(); 
	sendReport();	
    }
    return 0;
}
