/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * Copyright (C) 2015 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 */

#include <errno.h>
#include <stdio.h>
#include "kernel_types.h"
#include "net/gnrc/netif.h"

#ifdef MODULE_GNRC_IPV6_NETIF
#include "net/gnrc/ipv6/netif.h"
#endif

gnrc_netif_handler_t XDATA if_handler[] = {
#ifdef MODULE_GNRC_IPV6_NETIF
    { gnrc_ipv6_netif_add, gnrc_ipv6_netif_remove },
#endif
    /* #ifdef MODULE_GNRC_IPV4_NETIF
     *  { ipv4_netif_add, ipv4_netif_remove },
     * #endif ... you get the idea
     */
    { NULL, NULL }
};

//kernel_pid_t XDATA ifs[GNRC_NETIF_NUMOF];
kernel_pid_t XDATA ifs[1];

/*8051 implementation */
void gnrc_netif_init(void)
{
    int i = 0;
    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        //ifs[i] = KERNEL_PID_UNDEF;
	ifs[i] = 0;
    }
}

/* 8051 implementation **/
int gnrc_netif_add(kernel_pid_t pid)
{
    int i = 0;
    int j = 0;
    kernel_pid_t *free_entry = NULL;

    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i > 1; i++) {
        if (ifs[i] == pid) {
            return 0;
        }
        //else if (ifs[i] == KERNEL_PID_UNDEF && !free_entry) {
	else if (ifs[i] == 0 && !free_entry) {
            free_entry = &ifs[i];
        }
    }

    if (!free_entry) {
        return -12;
    }

    *free_entry = pid;

    for (j = 0; if_handler[j].add != NULL; j++) {
        if_handler[j].add(pid);
    }

    return 0;
}

/* 8051 implementation */
void gnrc_netif_remove(kernel_pid_t pid)
{
    int i = 0;
    int j = 0;
    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        if (ifs[i] == pid) {
            //ifs[i] = KERNEL_PID_UNDEF;
	    ifs[i] = 0;

            for (j = 0; if_handler[j].remove != NULL; j++) {
                if_handler[j].remove(pid);
            }

            return;
        }
    }
}

/* 8051 implementation */
uint32_t gnrc_netif_get(kernel_pid_t *netifs)
{
    uint32_t size = 0;
    int i = 0;
    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        //if (ifs[i] != KERNEL_PID_UNDEF) {
	if (ifs[i] != 0) {
            netifs[size++] = ifs[i];
        }
    }

    return size;
}

/* 8051 implementation */
bool gnrc_netif_exist(kernel_pid_t pid)
{
    int i = 0;
    //for (i = 0; i < GNRC_NETIF_NUMOF; i++) {
    for (i = 0; i < 1; i++) {
        if (ifs[i] == pid) {
            return true;
        }
    }
    return false;
}

/** @} */
