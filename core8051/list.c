/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

 /**
 * @addtogroup  core_util
 * @{
 *
 * @file
 * @brief       Intrusive linked list
 *
 * Lists are represented as element pointing to the first actual list element.
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 */
#include "list.h"
/* 8051 implementation */
static void list_add(list_node_t *node, list_node_t *new_node) 
{
    new_node->next = node->next;
    node->next = new_node;
}

static list_node_t* list_remove_head(list_node_t *list) {
    list_node_t* head = list->next;
    if (head) {
        list->next = head->next;
    }
    return head;
}

