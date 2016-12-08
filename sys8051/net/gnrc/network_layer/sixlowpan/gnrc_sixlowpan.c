/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
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

#include "kernel_types.h"
#include "net/gnrc.h"
#include "thread.h"
#include "utlist.h"
#include "net/gnrc/ipv6/hdr.h"
#include "net/gnrc/sixlowpan.h"
#include "net/gnrc/sixlowpan/frag.h"
#include "net/gnrc/sixlowpan/iphc.h"
#include "net/gnrc/sixlowpan/netif.h"
#include "net/sixlowpan.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

//#if ENABLE_DEBUG
/* For PRIu16 etc. */
//#include <inttypes.h>
//#endif

kernel_pid_t XDATA _pid = 0; //KERNEL_PID_UNDEF

#ifdef MODULE_GNRC_SIXLOWPAN_FRAG
gnrc_sixlowpan_msg_frag_t XDATA fragment_msg = {0, NULL, 0, 0}; //{KERNEL_PID_UNDEF, NULL, 0, 0};
#endif

/* 8051 implementation */
//#if ENABLE_DEBUG
//static char _stack[GNRC_SIXLOWPAN_STACK_SIZE + THREAD_EXTRA_STACKSIZE_PRINTF];
//#else
//static char _stack[GNRC_SIXLOWPAN_STACK_SIZE];
char XDATA _stack[THREAD_STACKSIZE_DEFAULT];
//#endif


/* handles GNRC_NETAPI_MSG_TYPE_RCV commands */
//void _receive(gnrc_pktsnip_t *pkt);
/* handles GNRC_NETAPI_MSG_TYPE_SND commands */
//void _send(gnrc_pktsnip_t *pkt);
/* Main event loop for 6LoWPAN */
//void *_event_loop(void *args);

kernel_pid_t gnrc_sixlowpan_init(void)
{
    if (_pid > 0) { //KERNEL_PID_UNDEF
        return _pid;
    }

    _pid = thread_create(_stack, sizeof(_stack), 3, 8, _event_loop, NULL, "6lo");//GNRC_SIXLOWPAN_PRIO, //THREAD_CREATE_STACKTEST, _event_loop, NULL, "6lo");


    return _pid;
}

void _receive(gnrc_pktsnip_t* XDATA pkt)
{
    gnrc_pktsnip_t* XDATA payload;
    uint8_t* XDATA dispatch;

    /* seize payload as a temporary variable */
    payload = gnrc_pktbuf_start_write(pkt); /* need to duplicate since pkt->next
                                             * might get replaced */

    if (payload == NULL) {
        //DEBUG("6lo: can not get write access on received packet\n");
/*#if defined(DEVELHELP) && ENABLE_DEBUG
        gnrc_pktbuf_stats();
#endif*/
        gnrc_pktbuf_release(pkt);
        return;
    }

    pkt = payload;  /* reset pkt from temporary variable */

    payload = gnrc_pktsnip_search_type(pkt, 3);//GNRC_NETTYPE_SIXLOWPAN);

    if ((payload == NULL) || (payload->size < 1)) {
        //DEBUG("6lo: Received packet has no 6LoWPAN payload\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    dispatch = payload->data;

    if (dispatch[0] == 0x41) {//SIXLOWPAN_UNCOMP) {
        gnrc_pktsnip_t *sixlowpan;
        //DEBUG("6lo: received uncompressed IPv6 packet\n");
        payload = gnrc_pktbuf_start_write(payload);

        if (payload == NULL) {
           // DEBUG("6lo: can not get write access on received packet\n");
/*#if defined(DEVELHELP) && ENABLE_DEBUG
            gnrc_pktbuf_stats();
#endif*/
            gnrc_pktbuf_release(pkt);
            return;
        }

        /* packet is uncompressed: just mark and remove the dispatch */
        sixlowpan = gnrc_pktbuf_mark(payload, sizeof(uint8_t), 3);//GNRC_NETTYPE_SIXLOWPAN);

        if (sixlowpan == NULL) {
            //DEBUG("6lo: can not mark 6LoWPAN dispatch\n");
            gnrc_pktbuf_release(pkt);
            return;
        }

        pkt = gnrc_pktbuf_remove_snip(pkt, sixlowpan);
        payload->type = GNRC_NETTYPE_IPV6;
    }
#ifdef MODULE_GNRC_SIXLOWPAN_FRAG
    else if (sixlowpan_frag_is((sixlowpan_frag_t *)dispatch)) {
        //DEBUG("6lo: received 6LoWPAN fragment\n");
        gnrc_sixlowpan_frag_handle_pkt(pkt);
        return;
    }
#endif
#ifdef MODULE_GNRC_SIXLOWPAN_IPHC
    else if (sixlowpan_iphc_is(dispatch)) {
        uint32_t dispatch_size, nh_len;
        gnrc_pktsnip_t *sixlowpan;
        gnrc_pktsnip_t *dec_hdr = gnrc_pktbuf_add(NULL, NULL, sizeof(ipv6_hdr_t),
                                                  GNRC_NETTYPE_IPV6);
        if ((dec_hdr == NULL) ||
            (dispatch_size = gnrc_sixlowpan_iphc_decode(&dec_hdr, pkt, 0, 0,
                                                        &nh_len)) == 0) {
            //DEBUG("6lo: error on IPHC decoding\n");
            if (dec_hdr != NULL) {
                gnrc_pktbuf_release(dec_hdr);
            }
            gnrc_pktbuf_release(pkt);
            return;
        }
        sixlowpan = gnrc_pktbuf_mark(pkt, dispatch_size, 3);//GNRC_NETTYPE_SIXLOWPAN);
        if (sixlowpan == NULL) {
            //DEBUG("6lo: error on marking IPHC dispatch\n");
            gnrc_pktbuf_release(dec_hdr);
            gnrc_pktbuf_release(pkt);
            return;
        }

        /* Remove IPHC dispatches */
        /* Insert decoded header instead */
        pkt = gnrc_pktbuf_replace_snip(pkt, sixlowpan, dec_hdr);
        payload->type = 2;//GNRC_NETTYPE_UNDEF;
    }
#endif
    else {
        //DEBUG("6lo: dispatch %02" PRIx8 " ... is not supported\n",
        //      dispatch[0]);
        gnrc_pktbuf_release(pkt);
        return;
    }
    //if (!gnrc_netapi_dispatch_receive(GNRC_NETTYPE_IPV6, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
    if (!gnrc_netapi_dispatch_receive(GNRC_NETTYPE_IPV6, 0xffff0000, pkt)) {
        //DEBUG("6lo: No receivers for this packet found\n");
        gnrc_pktbuf_release(pkt);
    }
}

bool _add_uncompr_disp(gnrc_pktsnip_t* XDATA pkt)
{
    gnrc_pktsnip_t* XDATA sixlowpan;
    uint8_t* XDATA disp;

    //DEBUG("6lo: Send uncompressed\n");

    sixlowpan = gnrc_pktbuf_add(NULL, NULL, sizeof(uint8_t), 3);//GNRC_NETTYPE_SIXLOWPAN);

    if (sixlowpan == NULL) {
        return false;
    }
    sixlowpan->next = pkt->next;
    pkt->next = sixlowpan;
    disp = sixlowpan->data;
    disp[0] = 0x41;//SIXLOWPAN_UNCOMP;
    return true;
}

void _send(gnrc_pktsnip_t* XDATA pkt)
{
    gnrc_netif_hdr_t* XDATA hdr;
    gnrc_pktsnip_t* XDATA pkt2;
    gnrc_sixlowpan_netif_t* XDATA iface;
    /* datagram_size: pure IPv6 packet without 6LoWPAN dispatches or compression */
    uint32_t XDATA datagram_size;

    if ((pkt == NULL) || (pkt->size < sizeof(gnrc_netif_hdr_t))) {
        //DEBUG("6lo: Sending packet has no netif header\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    if ((pkt->next == NULL) || (pkt->next->type != GNRC_NETTYPE_IPV6)) {
        //DEBUG("6lo: Sending packet has no IPv6 header\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    pkt2 = gnrc_pktbuf_start_write(pkt);

    if (pkt2 == NULL) {
        //DEBUG("6lo: no space left in packet buffer\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    hdr = pkt2->data;
    iface = gnrc_sixlowpan_netif_get(hdr->if_pid);
    datagram_size = gnrc_pkt_len(pkt2->next);

    if (iface == NULL) {
        //DEBUG("6lo: Can not get 6LoWPAN specific interface information.\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

#ifdef MODULE_GNRC_SIXLOWPAN_IPHC
    if (iface->iphc_enabled) {
        if (!gnrc_sixlowpan_iphc_encode(pkt2)) {
            //DEBUG("6lo: error on IPHC encoding\n");
            gnrc_pktbuf_release(pkt2);
            return;
        }
        /* IPHC dispatch does not count on dispatch length since it _shortens_
         * the datagram */
    }
    else {
        if (!_add_uncompr_disp(pkt2)) {
            /* adding uncompressed dispatch failed */
            //DEBUG("6lo: no space left in packet buffer\n");
            gnrc_pktbuf_release(pkt2);
            return;
        }
    }
#else
    /* suppress clang-analyzer report about iface being not read */
    (void) iface;
    if (!_add_uncompr_disp(pkt2)) {
        /* adding uncompressed dispatch failed */
        //DEBUG("6lo: no space left in packet buffer\n");
        gnrc_pktbuf_release(pkt2);
        return;
    }
#endif
    //DEBUG("6lo: iface->max_frag_size = %" PRIu16 " for interface %"
    //      PRIkernel_pid "\n", iface->max_frag_size, hdr->if_pid);

    /* IP should not send anything here if it is not a 6LoWPAN interface,
     * so we don't need to check for NULL pointers.
     * Note, that datagram_size cannot be used here, because the header size
     * might be changed by IPHC. */
    if (gnrc_pkt_len(pkt2->next) <= iface->max_frag_size) {
        //DEBUG("6lo: Send SND command for %p to %" PRIu16 "\n",
        //      (void *)pkt2, hdr->if_pid);
        if (gnrc_netapi_send(hdr->if_pid, pkt2) < 1) {
        //    DEBUG("6lo: unable to send %p over %" PRIu16 "\n", (void *)pkt, hdr->if_pid);
            gnrc_pktbuf_release(pkt2);
        }

        return;
    }
#ifdef MODULE_GNRC_SIXLOWPAN_FRAG
    else if (fragment_msg.pkt != NULL) {
        //DEBUG("6lo: Fragmentation already ongoing. Dropping packet\n");
        gnrc_pktbuf_release(pkt2);
        return;
    }
    else if (datagram_size <= SIXLOWPAN_FRAG_MAX_LEN) {
        //DEBUG("6lo: Send fragmented (%u > %" PRIu16 ")\n",
        //      (unsigned int)datagram_size, iface->max_frag_size);
        msg_t msg;

        fragment_msg.pid = hdr->if_pid;
        fragment_msg.pkt = pkt2;
        fragment_msg.datagram_size = datagram_size;
        /* Sending the first fragment has an offset==0 */
        fragment_msg.offset = 0;

        /* set the outgoing message's fields */
        msg.type = GNRC_SIXLOWPAN_MSG_FRAG_SND;
        msg.content.ptr = &fragment_msg;
        /* send message to self */
        msg_send_to_self(&msg);
    }
    else {
        //DEBUG("6lo: packet too big (%u > %" PRIu16 ")\n",
        //      (unsigned int)datagram_size, (uint16_t)SIXLOWPAN_FRAG_MAX_LEN);
        gnrc_pktbuf_release(pkt2);
    }
#else
    (void) datagram_size;
    //DEBUG("6lo: packet too big (%u > %" PRIu16 ")\n",
    //      (unsigned int)datagram_size, iface->max_frag_size);
    gnrc_pktbuf_release(pkt2);
#endif
}

void *_event_loop(void* args)
{
    msg_t XDATA msg, reply, msg_q[8U];//msg_q[GNRC_SIXLOWPAN_MSG_QUEUE_SIZE];
    gnrc_netreg_entry_t XDATA me_reg;

    (void)args;
    msg_init_queue(msg_q, 8U);//GNRC_SIXLOWPAN_MSG_QUEUE_SIZE);

   // me_reg.demux_ctx = GNRC_NETREG_DEMUX_CTX_ALL;
    me_reg.demux_ctx = 0xffff0000;
    me_reg.pid = thread_getpid();

    /* register interest in all 6LoWPAN packets */
    gnrc_netreg_register(3, &me_reg);//GNRC_NETTYPE_SIXLOWPAN, &me_reg);

    /* preinitialize ACK */
    //reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
    reply.type = 0x0205;

    /* start event loop */
    while (1) {
        //DEBUG("6lo: waiting for incoming message.\n");
        msg_receive(&msg);

        switch (msg.type) {
            //case GNRC_NETAPI_MSG_TYPE_RCV:
	    case 0x0201:
                //DEBUG("6lo: GNRC_NETDEV_MSG_TYPE_RCV received\n");
                _receive(msg.content.ptr);
                break;

            //case GNRC_NETAPI_MSG_TYPE_SND:
	    case 0x0202:
                //DEBUG("6lo: GNRC_NETDEV_MSG_TYPE_SND received\n");
                _send(msg.content.ptr);
                break;

            //case GNRC_NETAPI_MSG_TYPE_GET:
            //case GNRC_NETAPI_MSG_TYPE_SET:
	    case 0x0204:
	    case 0x0203:
                //DEBUG("6lo: reply to unsupported get/set\n");
		reply.content.value = 48;//-ENOTSUP;
		msg_reply(&msg, &reply);
                break;
#ifdef MODULE_GNRC_SIXLOWPAN_FRAG
            case GNRC_SIXLOWPAN_MSG_FRAG_SND:
                //DEBUG("6lo: send fragmented event received\n");
                gnrc_sixlowpan_frag_send(msg.content.ptr);
                break;
#endif

            default:
                //DEBUG("6lo: operation not supported\n");
                break;
        }
    }

    return NULL;
}

bool sixlowpan_frag_is(sixlowpan_frag_t *hdr)
{
    //return ((hdr->disp_size.u8[0] & SIXLOWPAN_FRAG_DISP_MASK) ==
    return ((hdr->disp_size.u8[0] & 0xf8) == 0xc0) ||
            ((hdr->disp_size.u8[0] & 0xf8) == 0xe0);
            //SIXLOWPAN_FRAG_1_DISP) ||
           //((hdr->disp_size.u8[0] & SIXLOWPAN_FRAG_DISP_MASK) ==
            //SIXLOWPAN_FRAG_N_DISP);
}

bool sixlowpan_iphc_is(uint8_t *data)
{
    //return ((*data & SIXLOWPAN_IPHC1_DISP_MASK) == SIXLOWPAN_IPHC1_DISP);
    return ((*data & 0xe0) == 0x60);
}

/** @} */
