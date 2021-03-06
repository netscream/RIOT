/*
 * Copyright (C) 2009, Freie Universitaet Berlin (FUB).
 * Copyright (C) 2013, INRIA.
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_shell
 * @{
 *
 * @file
 * @brief       Implementation of a very simple command interpreter.
 *              For each command (i.e. "echo"), a handler can be specified.
 *              If the first word of a user-entered command line matches the
 *              name of a handler, the handler will be called with the whole
 *              command line as parameter.
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      René Kijewski <rene.kijewski@fu-berlin.de>
 *
 * @}
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "shell.h"
#include "shell_commands.h"

//#ifndef SHELL_NO_ECHO
//#ifdef MODULE_NEWLIB
/* use local copy of putchar, as it seems to be inlined,
 * enlarging code by 50% */
static void _putchar(int c) {
    putchar(c);
}
//#else
//#define _putchar putchar
//#endif
//#endif

shell_command_handler_t find_handler(const shell_command_t *command_list, char* command)
{
    /*const shell_command_t *command_lists[] = {
        command_list,
#ifdef MODULE_SHELL_COMMANDS
        _shell_command_list,
#endif
    };*/

    int i = 0;
    /* iterating over command_lists */
    for (i = 0; i < sizeof(command_list); i++){
	//printf("%s %s \n", command, command_list[i].name);
	if (strcmp(command_list[i].name, command) == 0)
	{
		return command_list[i].handler;
	}
        /*if ((entry = command_lists[i])) {
            while (entry->name != NULL) {
                if (strcmp(entry->name, command) == 0) {
                    return entry->handler;
                }
                else {
                    entry++;
                }
            }
        }*/
    }

    return NULL;
}

void print_help(const shell_command_t *command_list)
{
   // shell_command_t* command_lists[] = {NULL};
   // shell_command_t* entry = NULL;
 
    unsigned i = 0;
    printf("Command           Description\n");
    printf("---------------------------------------\n");

    /*command_lists = {
        command_list,
#ifdef MODULE_SHELL_COMMANDS
        _shell_command_list,
#endif
    };*/
 
    /* iterating over command_lists */
    for (i = 0; i < sizeof(command_list); i++) {
	if (command_list[i].name != NULL) {
		printf("%s %s\n", command_list[i].name, command_list[i].desc);
	}
        /*if ((entry = command_lists[i])) {
                    while (entry->name != NULL) {
                printf("%s %s", entry->name, entry->desc);
                entry++;
            }
        }*/
    }
}

void handle_input_line(const shell_command_t *command_list, char* line)
{
    const char *INCORRECT_QUOTING = "shell: incorrect quoting";

    /* first we need to calculate the number of arguments */
    unsigned argc = 0;
    char *pos = line;
    int contains_esc_seq = 0;
    char* argv;
    char* lin = "";
    char **arg = NULL;
    char *c = NULL;
    char *d = NULL;
    unsigned i = 0;
    int (*handler)(char** argv); 

    /*while (1) {
        if ((unsigned char) *pos > ' ') {
        
            if (*pos == '"' || *pos == '\'') {
                
                const char quote_char = *pos;
                do {
                    ++pos;
                    if (!*pos) {
                        printf(INCORRECT_QUOTING);
                        return;
                    }
                    else if (*pos == '\\') {
                        
                        ++contains_esc_seq;
                        ++pos;
                        if (!*pos) {
                            printf(INCORRECT_QUOTING);
                            return;
                        }
                        continue;
                    }
                } while (*pos != quote_char);
                if ((unsigned char) pos[1] > ' ') {
                    printf(INCORRECT_QUOTING);
                    return;
                }
            }
            else {
               
                do {
                    if (*pos == '\\') {
                        
                        ++contains_esc_seq;
                        ++pos;
                        if (!*pos) {
                            printf(INCORRECT_QUOTING);
                            return;
                        }
                    }
                    ++pos;
                    if (*pos == '"') {
                        printf(INCORRECT_QUOTING);
                        return;
                    }
                } while ((unsigned char) *pos > ' ');
            }

            
            ++argc;
        }

        if (*pos > 0) {
            *pos = 0;
            ++pos;
        }
        else {
            break;
        }
    }
    if (!argc) {
	printf("No arguments\n");
        return;
    }*/
    
    /* then we fill the argv array */
    //argv[argc + 1];
    //argv[argc] = NULL;
    /*pos = line;
    for (i = 0; i < argc; ++i) {
        while (!*pos) {
            ++pos;
        }
        if (*pos == '"' || *pos == '\'') {
            ++pos;
        }
        argv[i] = *pos;
        while (*pos) {
            ++pos;
        }
    }
    for (**arg = *argv; contains_esc_seq && *arg; ++arg) {
        for (*c = **arg; *c; ++c) {
            if (*c != '\\') {
                continue;
            }
            for (*d = *c; *d; ++d) {
                *d = d[1];
            }
            if (--contains_esc_seq == 0) {
                break;
            }
        }
    }*/ 
    
    for (i = 0; i < 30; i++)
    {
	if (*line == 32)
	{
	   *line = '\0';
	   //pos[i] = '\0';
	   *line++;
	   //*pos  = '\0';
	   //pos++;
	   //argv[argc] = *pos;
	   //argc++;
	   break;
	}
	else
	{
		*line++;
	}
    }
     
    /* then we call the appropriate handler */
    
    handler =  find_handler(command_list, pos);
    if (handler != NULL) {
	handler(line);
    }
    else {
        if (strcmp("help", pos) == 0) {
            print_help(command_list);
        }
        else {
            printf("shell: command not found: %s\n", pos);
        }
    }
    *pos = 0;
}

int readline(char *buf, size_t size)
{
    char *line_buf_ptr = buf;
    int c = 0;
    while (1) {
        if ((line_buf_ptr - buf) >= ((int) size) - 1) {
            return -1;
        }

        c = getchar();
        if (c < 0) {
            return 1;
        }

        /* We allow Unix linebreaks (\n), DOS linebreaks (\r\n), and Mac linebreaks (\r). */
        /* QEMU transmits only a single '\r' == 13 on hitting enter ("-serial stdio"). */
        /* DOS newlines are handled like hitting enter twice, but empty lines are ignored. */
        if (c == '\r' || c == '\n') {
            *line_buf_ptr = '\0';
/*#ifndef SHELL_NO_ECHO
            _putchar('\r');
            _putchar('\n');
#endif*/

            /* return 1 if line is empty, 0 otherwise */
            return line_buf_ptr == buf;
        }
        /* QEMU uses 0x7f (DEL) as backspace, while 0x08 (BS) is for most terminals */
        else if (c == 0x08 || c == 0x7f) {
            if (line_buf_ptr == buf) {
                /* The line is empty. */
                continue;
            }

            *--line_buf_ptr = '\0';
            /* white-tape the character */
/*#ifndef SHELL_NO_ECHO
            _putchar('\b');
            _putchar(' ');
            _putchar('\b');
#endif*/
        }
        else {
            *line_buf_ptr++ = c;
/*#ifndef SHELL_NO_ECHO
            _putchar(c);
#endif*/
        }
    }
}

void print_prompt(void)
{
//#ifndef SHELL_NO_PROMPT
    putchar('> ');
//#endif

/*#ifdef MODULE_NEWLIB
    fflush(stdout);
#endif*/
}

/*void shell_run(const shell_command_t *shell_commands, char *line_buf, int len)
{
    print_prompt();

    while (1) {
        int res = readline(line_buf, len);

        if (!res) {
            handle_input_line(shell_commands, line_buf);
        }

        print_prompt();
    }
}*/
