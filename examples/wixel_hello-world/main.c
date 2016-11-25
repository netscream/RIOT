/*
 * Copyright (C) 2014 Freie Universität Berlin
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
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "board.h"
#include "riotbuild.h"
#include "usb_com.h"

/*  parameters */
/* CONSTS */
int32 CODE param_report_period_ms = 40;

/* variables */
uint8 XDATA report[2048];
uint16 XDATA reportLength = 0;
uint16 XDATA reportBytesSent = 0;

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
   uint8 XDATA bytesToSend = 0;
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
       if(reportLength == 0)
       {
	   printf("Hello world Program\n\n");
	   printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
           printf("This board features a(n) %s MCU.\n\n\n", RIOT_MCU);
       }
   }
}

int main(void)
{ 
    systemInit(); 
    kernel_init();
         
    while(1) { 
	updateLeds();	
	usbComService();       
	sendReport();
    }
    return 0;
}
