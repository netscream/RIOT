/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <assert.h>

#include "net/gnrc/pkt.h"

void gnrc_pktqueue_add(gnrc_pktqueue_t **queue, gnrc_pktqueue_t *node)
{
     gnrc_pktqueue_t *tmp = *queue;
     LL_APPEND(tmp, node);
}

gnrc_pktqueue_t *gnrc_pktqueue_remove(gnrc_pktqueue_t **queue, gnrc_pktqueue_t *node)
{
    if (node) {
        gnrc_pktqueue_t *tmp = *queue;
        LL_DELETE(tmp, node);
        node->next = NULL;
    }

    return node;
}

gnrc_pktqueue_t *gnrc_pktqueue_remove_head(gnrc_pktqueue_t **queue)
{
    return gnrc_pktqueue_remove(queue, *queue);
}

/** @} */
