#ifndef __L2CAP_VAL_H
#define __L2CAP_VAL_H

/*
 *  l2cap.h
 *
 *  Logical Link Control and Adaption Protocl (L2CAP)
 *
 *  Created by Matthias Ringwald on 5/16/09.
 */

#include "l2cap.h"
#include "hci.h"
#include "hci_dump.h"
#include "bluetooth_sdp.h"
#include "btstack_debug.h"
#include "btstack_event.h"
#include "btstack_memory.h"

#include <stdarg.h>
#include <string.h>

#include <stdio.h>

// nr of buffered acl packets in outgoing queue to get max performance 
#define NR_BUFFERED_ACL_PACKETS 3

// used to cache l2cap rejects, echo, and informational requests
#define NR_PENDING_SIGNALING_RESPONSES 3

// nr of credits provided to remote if credits fall below watermark
#define L2CAP_LE_DATA_CHANNELS_AUTOMATIC_CREDITS_WATERMARK 5
#define L2CAP_LE_DATA_CHANNELS_AUTOMATIC_CREDITS_INCREMENT 5

// offsets for L2CAP SIGNALING COMMANDS
#define L2CAP_SIGNALING_COMMAND_CODE_OFFSET   0
#define L2CAP_SIGNALING_COMMAND_SIGID_OFFSET  1
#define L2CAP_SIGNALING_COMMAND_LENGTH_OFFSET 2
#define L2CAP_SIGNALING_COMMAND_DATA_OFFSET   4

#if defined(ENABLE_LE_DATA_CHANNELS) || defined(ENABLE_CLASSIC)
#define L2CAP_USES_CHANNELS
#endif

     
// l2cap_fixed_channel_t entries
#ifdef ENABLE_BLE
extern  l2cap_fixed_channel_t l2cap_fixed_channel_att;
extern  l2cap_fixed_channel_t l2cap_fixed_channel_sm;
#endif
#ifdef ENABLE_CLASSIC
extern  l2cap_fixed_channel_t l2cap_fixed_channel_connectionless;
#endif

#ifdef ENABLE_CLASSIC
extern  btstack_linked_list_t l2cap_services;
extern  uint8_t require_security_level2_for_outgoing_sdp;
extern  bd_addr_t l2cap_outgoing_classic_addr;
#endif

#ifdef ENABLE_LE_DATA_CHANNELS
extern  btstack_linked_list_t l2cap_le_services;
#endif

// single list of channels for Classic Channels, LE Data Channels, Classic Connectionless, ATT, and SM
extern btstack_linked_list_t l2cap_channels;

// used to cache l2cap rejects, echo, and informational requests
extern  l2cap_signaling_response_t signaling_responses[NR_PENDING_SIGNALING_RESPONSES];
extern  int signaling_responses_pending;
extern  btstack_packet_callback_registration_t hci_event_callback_registration;

#ifdef ENABLE_BLE
// only used for connection parameter update events
extern btstack_packet_handler_t l2cap_event_packet_handler;
extern  uint16_t l2cap_le_custom_max_mtu;
#endif




#endif // __L2CAP_VAL_H