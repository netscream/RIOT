/*
 * Copyright (C) 2013 Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @file
 * @brief       shows how to set up own and use the system shell commands.
 *              By typing help in the serial console, all the supported commands
 *              are listed.
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Zakaria Kasmi <zkasmi@inf.fu-berlin.de>
 *
 */

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "usb_com.h"
#include "shell_commands.h"
#include "shell.h"
/*  parameters */
/* CONSTS */
int32 CODE param_report_period_ms = 40;

/* variables */
uint8 XDATA report[2048];
uint8 XDATA input[512];
uint16 XDATA reportLength = 0;
uint16 XDATA reportBytesSent = 0;
//char snd_thread_stack[THREAD_STACKSIZE_MAIN];
char XDATA snd_thread_stack[128];
uint8 XDATA st = 0;
uint16 XDATA inputLength = 0;


void putchar(char c)
{
    report[reportLength] = c;
    reportLength++;
}

void readChar()
{
	 char XDATA a = usbComRxReceiveByte();
	 input[inputLength] = a;
	 inputLength++;
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
void resetInput()
{
	int i = 0;
	for (i = 0; i < inputLength; i++)
	{
		input[i] = '\0';
	}
	inputLength = 0;
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

static int print_teststart(char* argv)
{
    (void) argv;
    printf("[TEST_START]\n");

    return 0;
}


static int print_ech(char* argv)
{
    printf(argv);
    printf("\n");
    return 0;
}

static const CODE shell_command_t shell_commands[] = {
    { "start_test", "starts a test", print_teststart },
    { "echo", "prints the input command", print_ech },
    { NULL, NULL, NULL }
};

void shell_run(const shell_command_t *shell_commands, char *line_buf, int len)
{
    int res = 0;
    //print_prompt();
    while (1) {
        updateLeds();
	usbComService();
	
	if (usbComTxAvailable()) {
		sendReport();
	}
	
        while (usbComRxAvailable()) {
        	readChar();
        }

	if (inputLength > 0) {
		handle_input_line(shell_commands, input);
		//print_help(shell_commands);
		input[inputLength] = '\0';
		resetInput();
	}
    }
}

int main(void)
{

    /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    systemInit();
    printf("test_shell.\n");
    
    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    
    /* or use only system shell commands */
    /*
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    */

    return 0;
}
