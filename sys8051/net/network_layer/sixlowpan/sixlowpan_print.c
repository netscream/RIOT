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

#include <stdio.h>
#include <inttypes.h>

#include "od.h"
#include "net/ipv6/hdr.h"
#include "net/sixlowpan.h"

void sixlowpan_print(uint8_t *data, uint32_t size)
{
    uint8_t offset = 0;
    //if (data[0] == SIXLOWPAN_UNCOMP) {
    if (data[0] == 0x41) {
        puts("Uncompressed IPv6 packet");

        /* might just be the dispatch (or fragmented) so better check */
        if (size > sizeof(ipv6_hdr_t)) {
            ipv6_hdr_print((ipv6_hdr_t *)(data + 1));
            od_hex_dump(data + sizeof(ipv6_hdr_t) + 1,
                        size - sizeof(ipv6_hdr_t) - 1,
                        OD_WIDTH_DEFAULT);
        }
    }
    else if (sixlowpan_nalp(data[0])) {
        puts("Not a LoWPAN (NALP) frame");
        od_hex_dump(data, size, OD_WIDTH_DEFAULT);
    }
    else if ((data[0] & 0xf8) == 0xc0) {
    //else if ((data[0] & SIXLOWPAN_FRAG_DISP_MASK) == SIXLOWPAN_FRAG_1_DISP) {
        sixlowpan_frag_t *hdr = (sixlowpan_frag_t *)data;

        puts("Fragmentation Header (first)\n");
        //printf("datagram size: %" PRIu16 "\n",
	//printf("datagram size: %d\n", (byteorder_ntohs(hdr->disp_size) & 0x07ff));
        //printf("tag: 0x%" PRIu16 "\n", byteorder_ntohs(hdr->tag));
	//printf("tag: 0x%x\n", byteorder_ntohs(hdr->tag));
        /* Print next dispatch */
        sixlowpan_print(data + sizeof(sixlowpan_frag_t),
                           size - sizeof(sixlowpan_frag_t));
    }
    else if ((data[0] & 0xf8) == 0xe0) {
    //else if ((data[0] & SIXLOWPAN_FRAG_DISP_MASK) == SIXLOWPAN_FRAG_N_DISP) {
        sixlowpan_frag_n_t *hdr = (sixlowpan_frag_n_t *)data;

        puts("Fragmentation Header (subsequent)");
        //printf("datagram size: %" PRIu16 "\n",
	//printf("datagram size: %d\n", (uint16_t) (byteorder_ntohs(hdr->disp_size) & 0x07ff));
        //printf("tag: 0x%d\n", byteorder_ntohs(hdr->tag));
        printf("offset: 0x%d\n", hdr->offset);

        od_hex_dump(data + sizeof(sixlowpan_frag_n_t),
                    size - sizeof(sixlowpan_frag_n_t),
                    OD_WIDTH_DEFAULT);
    }
    else if ((data[0] & 0xe0) == 0x60) {
    //else if ((data[0] & SIXLOWPAN_IPHC1_DISP_MASK) == SIXLOWPAN_IPHC1_DISP) {
        //uint8_t offset = SIXLOWPAN_IPHC_HDR_LEN;
	offset = 2;
        puts("IPHC dispatch");

        //switch (data[0] & SIXLOWPAN_IPHC1_TF) {
	switch (data[0] & 0x18) {
            case 0x00:
                puts("TF: ECN + DSCP + Flow Label (4 bytes)");
                break;

            case 0x08:
                puts("TF: ECN + Flow Label (3 bytes)");
                break;

            case 0x10:
                puts("TF: ECN + DSCP (1 bytes)");
                break;

            case 0x18:
                puts("TF: traffic class and flow label elided");
                break;
        }

        //switch (data[0] & SIXLOWPAN_IPHC1_NH) {
	switch (data[0] & 0x18) {
            case 0x00:
                puts("NH: inline");
                break;

            case 0x04:
                puts("NH: LOWPAN_NHC");
                break;
        }

        //switch (data[0] & SIXLOWPAN_IPHC1_HL) {
	switch (data[0] & 0x03) {
            case 0x00:
                puts("HLIM: inline");
                break;

            case 0x01:
                puts("HLIM: 1");
                break;

            case 0x02:
                puts("HLIM: 64");
                break;

            case 0x03:
                puts("HLIM: 255");
                break;
        }

        //if (data[1] & SIXLOWPAN_IPHC2_SAC) {
	if (data[1] & 0x40) {
            printf("Stateful source address compression: ");

            //switch (data[1] & SIXLOWPAN_IPHC2_SAM) {
	    switch (data[1] & 0x30) {
                case 0x00:
                    puts("unspecified address (::)");
                    break;

                case 0x10:
                    puts("64 bits inline");
                    break;

                case 0x20:
                    puts("16 bits inline");
                    break;

                case 0x30:
                    puts("elided (use L2 address)");
                    break;
            }
        }
        else {
            printf("Stateless source address compression: ");

            //switch (data[1] & SIXLOWPAN_IPHC2_SAM) {
	    switch (data[1] & 0x30) {
                case 0x00:
                    puts("128 bits inline");
                    break;

                case 0x10:
                    puts("64 bits inline");
                    break;

                case 0x20:
                    puts("16 bits inline");
                    break;

                case 0x30:
                    puts("elided (use L2 address)");
                    break;
            }
        }

        //if (data[1] & sixlowpan_iphc2_m) {
	if (data[1] & 0x08) {
            //if (data[1] & SIXLOWPAN_IPHC2_DAC) {
	    if (data[1] & 0x04) {
                puts("Stateful destinaton multicast address compression:");

                //switch (data[1] & SIXLOWPAN_IPHC2_DAM) {
		  switch (data[1] & 0x03) {
                    case 0x00:
                        puts("    48 bits carried inline (Unicast-Prefix-based)");
                        break;

                    case 0x01:
                    case 0x02:
                    case 0x03:
                        puts("    reserved");
                        break;
                }
            }
            else {
                puts("Stateless destinaton multicast address compression:");

                //switch (data[1] & SIXLOWPAN_IPHC2_DAM) {
		switch (data[1] & 0x03) {
                    case 0x00:
                        puts("    128 bits carried inline");
                        break;

                    case 0x01:
                        puts("    48 bits carried inline");
                        break;

                    case 0x02:
                        puts("    32 bits carried inline");
                        break;

                    case 0x03:
                        puts("    8 bits carried inline");
                        break;
                }
            }
        }
        else {
            //if (data[1] & SIXLOWPAN_IPHC2_DAC) {
	    if (data[1] & 0x04) {
                printf("Stateful destinaton address compression: ");

                //switch (data[1] & SIXLOWPAN_IPHC2_DAM) {
		switch (data[1] & 0x03) {
                    case 0x00:
                        puts("reserved");
                        break;

                    case 0x10:
                        puts("64 bits inline");
                        break;

                    case 0x20:
                        puts("16 bits inline");
                        break;

                    case 0x30:
                        puts("elided (use L2 address)");
                        break;
                }
            }
            else {
                printf("Stateless destinaton address compression: ");

                //switch (data[1] & SIXLOWPAN_IPHC2_DAM) {
		switch (data[1] & 0x03) {
                    case 0x00:
                        puts("128 bits inline");
                        break;

                    case 0x10:
                        puts("64 bits inline");
                        break;

                    case 0x20:
                        puts("16 bits inline");
                        break;

                    case 0x30:
                        puts("elided (use L2 address)");
                        break;
                }
            }
        }

        //if (data[1] & SIXLOWPAN_IPHC2_CID_EXT) {
	if (data[1] & 0x80) {
            //offset += SIXLOWPAN_IPHC_CID_EXT_LEN;
	    offset += 0x80;
            //printf("SCI: 0x%" PRIx8 "; DCI: 0x%" PRIx8 "\n",
	    printf("SCI: 0x%d; DCI: 0x%x\n",

                   (uint8_t) (data[2] >> 4), (uint8_t) (data[2] & 0xf));
        }

        od_hex_dump(data + offset, size - offset, OD_WIDTH_DEFAULT);
    }
}

bool sixlowpan_nalp(uint8_t disp)
{
    return ((disp & 0xc0) == 0);
}

/** @} */
