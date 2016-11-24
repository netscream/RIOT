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
//#include "kernel_init.h"
#include "usb.h"
#include "usb_com.h"

/*  parameters */
char XDATA stuff[5];

void putchar(char c)
{
	
}

int main(void)
{
    //systemInit(); 
    kernel_init();
    systemInit();
    stuff[0] = 'h';
    stuff[1] = 'e'; 
    stuff[2] = 'l';
    stuff[3] = 'l';
    stuff[4] = 'o';

    LED_RED(1); 
    while(1) { 
	    int CODE i = 0;
	    boardService();
	    usbComService();
	    puts("Hello World!");
	    i = usbComTxAvailable();
	    if (i > 1)
	    { 
		usbComTxSend(stuff, 5);
	    }
	    //LED_RED(0);
	    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
	    //printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    }
    return 0;
}
