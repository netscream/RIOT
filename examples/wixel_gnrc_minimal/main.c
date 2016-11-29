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
#include "kernel_init.h"
#include "riotbuild.h"
#include "usb_com.h"
#include "msg.h"
#include "net/ipv6/addr.h"
//#include "net/gnrc/netif.h"
#include "net/gnrc/ipv6/netif.h"

int8 CODE param_report_period_ms = 40;
char XDATA report[2048];
uint32 XDATA reportLength;
uint16 XDATA reportBytesSent = 0;
int8 XDATA FirstC = 0;
kernel_pid_t XDATA ifs[1];

void updateLeds()
{
    usbShowStatusWithGreenLed();
    LED_YELLOW(0);
    LED_RED(0);
}

/*void putc(char* c)
{
   int i = 0;
   for (i = 0; i < sizeof(c); i++)
   {
	putchar(c[i]);
   }
}*/
void putchar(char c)
{
    report[reportLength] = c;
    reportLength++;
}


void sendReport()
{
   uint8 XDATA bytesToSend = 0;
   int16 XDATA i = 0;
   size_t XDATA numof = 1;
   char XDATA ipv6_addr[40];
   gnrc_ipv6_netif_t *entry; //= gnrc_ipv6_netif_get(ifs[0]);
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
	  if (FirstC != 1){
 	     printf("RIOT network stack example application\n");                           	    
	     FirstC = 1;
	  }
	  else
	  {
		//numof = gnrc_netif_get(ifs);
		/*if (numof > 0) {
        		gnrc_ipv6_netif_t *entry = gnrc_ipv6_netif_get(ifs[0]);
		        for (i = 0; i < 8; i++) {
			    if ((ipv6_addr_is_link_local(&entry->addrs[i].addr)) && !(entry->addrs[i].flags & 0x01)) {
		 		//ipv6_addr_to_str(ipv6_addr, &entry->addrs[i].addr, 40);
            	            	//printf("My address is %s\n", ipv6_addr);
            	            }
        	        }
    		}*/

	  }
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
