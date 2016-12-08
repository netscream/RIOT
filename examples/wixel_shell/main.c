/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
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
 * @brief       Example application for demonstrating the RIOT's POSIX sockets
 *
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "board.h"
#include "riotbuild.h"
#include "usb_com.h"
//#include "msg.h"
//#include "ps.h"
#include "shell.h"

/*  parameters */
/* CONSTS */
int32 CODE param_report_period_ms = 40;

/* variables */
uint8 XDATA report[2048];
uint8 XDATA geport[1024];
uint16 XDATA reportLength = 0;
uint16 XDATA geportLength = 0;
uint16 XDATA reportBytesSent = 0;
uint8 XDATA first = 0;
uint8 XDATA line_buf[SHELL_DEFAULT_BUFSIZE];
uint8 XDATA sendAvail = 0;

shell_command_t CODE shell_commands[] = {
    { "ps" , "Get process information", 1},
    { NULL, NULL, NULL }
};

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

uint8 getchar()
{
    geport[geportLength] = usbComRxReceiveByte();
    geportLength++;
    return geport[geportLength-1];
}

/*void sendReport()
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
       if(reportLength == 0 && first)
       {
           printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
           printf("This board features a(n) %s MCU.\n\n\n", RIOT_MCU);
           first = 0;
           print_prompt();
	   sendAvail = 1;
       }
       else
       if(reportLength == 0 && sendAvail == 0 && first == 0)
       {
	   printf(">\n");
       }
       else
       if(reportLength == 0 && first == 1)
       {
	  res = readline(line_buf, SHELL_DEFAULT_BUFSIZE);
	  printf(line_buf);
          if (!res) 
	  {
              handle_input_line(shell_commands, line_buf);
          }
       }
   }
}*/

void getReport()
{
	uint8 XDATA bytesToRecv = 0;
	uint8 XDATA res = 0;
	if (getMs() >= param_report_period_ms)
	{
	    bytesToRecv = usbComRxAvailable();
	    if (bytesToRecv > 0)
	    {
		getchar();
	    }
	    else
	    {
		sendAvail = 0;
	    }
	}
}
	

int main(void)
{
    systemInit();
    kernel_init();
    while(1) 
    {
        updateLeds();
        usbComService();
	if (sendAvail)
	{
		getReport();
	}
	else
        {
		sendReport();
	}
    }
    return 0;
}
