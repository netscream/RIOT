/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *               2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include "clist.h"
/* 8051 implementation */
void clist_rpush(clist_node_t* list, clist_node_t* new_node)
{
    if (list->next) {
        new_node->next = list->next->next;
        list->next->next = new_node;
    }
    else {
        new_node->next = new_node;
    }
    list->next = new_node;
}

void clist_lpush(clist_node_t* list, clist_node_t* new_node)
{
    if (list->next) {
        new_node->next = list->next->next;
        list->next->next = new_node;
    }
    else {
        new_node->next = new_node;
        list->next = new_node;
    }
}

clist_node_t *clist_lpop(clist_node_t* list)
{
    if (list->next) {
        clist_node_t *first = list->next->next;
        if (list->next == first) {
            list->next = NULL;
        }
        else {
            list->next->next = first->next;
        }
        return first;
    }
    else {
        return NULL;
    }
}

void clist_lpoprpush(clist_node_t* list)
{
    if (list->next) {
        list->next = list->next->next;
    }
}

clist_node_t *clist_lpeek(const clist_node_t* list)
{
    if (list->next) {
        return list->next->next;
    }
    return NULL;
}

clist_node_t *clist_rpeek(const clist_node_t* list)
{
    return list->next;
}

clist_node_t *clist_rpop(clist_node_t* list)
{
    if (list->next) {
        list_node_t *last = list->next;
        while (list->next->next != last) {
            clist_lpoprpush(list);
        }
        return clist_lpop(list);
    }
    else {
        return NULL;
    }
}

clist_node_t *clist_find_before(const clist_node_t* list, const clist_node_t* node)
{
    clist_node_t *pos = list->next;
    if (!pos) {
        return NULL;
    }
    do {
        pos = pos->next;
        if (pos->next == node) {
            return pos;
        }
    } while (pos != list->next);

    return NULL;
}

clist_node_t *clist_find(const clist_node_t* list, const clist_node_t* node)
{
    clist_node_t *tmp = clist_find_before(list, node);
    if (tmp) {
        return tmp->next;
    }
    else {
        return NULL;
    }
}

clist_node_t *clist_remove(clist_node_t* list, clist_node_t* node)
{
    if (list->next) {
        if (list->next->next == node) {
            return clist_lpop(list);
        }
        else {
            clist_node_t *tmp = clist_find_before(list, node);
            if (tmp) {
                tmp->next = tmp->next->next;
                if (node == list->next) {
                    list->next = tmp;
                }
                return node;
            }
        }
    }

    return NULL;
}

void clist_foreach(clist_node_t* list, int(*func)(clist_node_t *))
{
    clist_node_t *node = list->next;
    if (! node) {
        return;
    }
    do {
        node = node->next;
        if (func(node)) {
            return;
        }
    } while (node != list->next);
}

