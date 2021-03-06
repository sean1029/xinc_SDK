/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#define __BTSTACK_FILE__ "att_server.c"


//
// ATT Server Globals
//

#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "btstack_config.h"

#include "att_dispatch.h"
#include "ble/att_db.h"
#include "ble/att_server.h"
#include "ble/core.h"
#include "ble/le_device_db.h"
#include "ble/sm.h"
#include "btstack_debug.h"
#include "btstack_event.h"
#include "btstack_memory.h"
#include "btstack_run_loop.h"
#include "gap.h"
#include "hci.h"
#include "hci_dump.h"
#include "l2cap.h"
#include "btstack_tlv.h"

#include "patch_function_id.h"

#ifdef ENABLE_LE_SIGNED_WRITE
#include "ble/sm.h"
#endif

#ifndef NVN_NUM_GATT_SERVER_CCC
#define NVN_NUM_GATT_SERVER_CCC 20
#endif

static void att_run_for_context(att_server_t * att_server);
static att_write_callback_t att_server_write_callback_for_handle(uint16_t handle);
static btstack_packet_handler_t att_server_packet_handler_for_handle(uint16_t handle);
static void att_server_persistent_ccc_restore(att_server_t * att_server);
static void att_server_persistent_ccc_clear(att_server_t * att_server);

typedef enum {
    ATT_SERVER_RUN_PHASE_1_REQUESTS,
    ATT_SERVER_RUN_PHASE_2_INDICATIONS,
    ATT_SERVER_RUN_PHASE_3_NOTIFICATIONS,
} att_server_run_phase_t;

//
typedef struct {
    uint32_t seq_nr;
    uint16_t att_handle;
    uint8_t  value;
    uint8_t  device_index;
} persistent_ccc_entry_t;

// global
extern btstack_packet_callback_registration_t hci_event_callback_registration1;
extern btstack_packet_callback_registration_t sm_event_callback_registration;
extern btstack_packet_handler_t               att_client_packet_handler ;
extern btstack_linked_list_t                  service_handlers;
extern btstack_context_callback_registration_t att_client_waiting_for_can_send_registration;

extern att_read_callback_t                    att_server_client_read_callback;
extern att_write_callback_t                   att_server_client_write_callback;

// round robin
extern hci_con_handle_t att_server_last_can_send_now ;

static att_server_t * att_server_for_handle(hci_con_handle_t con_handle){
    hci_connection_t * hci_connection = hci_connection_for_handle(con_handle);
    if (!hci_connection) return NULL;
    return &hci_connection->att_server;
}

#ifdef ENABLE_LE_SIGNED_WRITE
static att_server_t * att_server_for_state(att_server_state_t state){
    btstack_linked_list_iterator_t it;
    hci_connections_get_iterator(&it);
    while(btstack_linked_list_iterator_has_next(&it)){
        hci_connection_t * connection = (hci_connection_t *) btstack_linked_list_iterator_next(&it);
        att_server_t * att_server = &connection->att_server;
        if (att_server->state == state) return att_server;
    }
    return NULL;
}
#endif

static void att_handle_value_indication_notify_client(uint8_t status, uint16_t client_handle, uint16_t attribute_handle){
	
	if(PATCH_FUN[ATT_HANDLE_VALUE_INDICATION_NOTIFY_CLIENT_ID]){
         ((void (*)(uint8_t status, uint16_t client_handle, uint16_t attribute_handle))PATCH_FUN[ATT_HANDLE_VALUE_INDICATION_NOTIFY_CLIENT_ID])(status,   client_handle,   attribute_handle);
         return ;
    }

    btstack_packet_handler_t packet_handler = att_server_packet_handler_for_handle(attribute_handle);
    if (!packet_handler) return;
    
    uint8_t event[7];
    int pos = 0;
    event[pos++] = ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE;
    event[pos++] = sizeof(event) - 2;
    event[pos++] = status;
    little_endian_store_16(event, pos, client_handle);
    pos += 2;
    little_endian_store_16(event, pos, attribute_handle);
    (*att_client_packet_handler)(HCI_EVENT_PACKET, 0, &event[0], sizeof(event));
}

static void att_emit_event_to_all(const uint8_t * event, uint16_t size){
	
	if(PATCH_FUN[ATT_EMIT_EVENT_TO_ALL_ID]){
         ((void (*)(const uint8_t * event, uint16_t size))PATCH_FUN[ATT_EMIT_EVENT_TO_ALL_ID])(event,   size);
         return ;
    }

    // dispatch to app level handler
    if (att_client_packet_handler){
        (*att_client_packet_handler)(HCI_EVENT_PACKET, 0, (uint8_t*) event, size);
    }

    // dispatch to service handlers
    btstack_linked_list_iterator_t it;
    btstack_linked_list_iterator_init(&it, &service_handlers);
    while (btstack_linked_list_iterator_has_next(&it)){
        att_service_handler_t * handler = (att_service_handler_t*) btstack_linked_list_iterator_next(&it);
        if (!handler->packet_handler) continue;
        (*handler->packet_handler)(HCI_EVENT_PACKET, 0, (uint8_t*) event, size);
    }
}

static void att_emit_mtu_event(hci_con_handle_t con_handle, uint16_t mtu){
	if(PATCH_FUN[ATT_EMIT_MTU_EVENT_ID]){
         ((void (*)(hci_con_handle_t con_handle, uint16_t mtu))PATCH_FUN[ATT_EMIT_MTU_EVENT_ID])(con_handle,   mtu);
         return ;
    }

    uint8_t event[6];
    int pos = 0;
    event[pos++] = ATT_EVENT_MTU_EXCHANGE_COMPLETE;
    event[pos++] = sizeof(event) - 2;
    little_endian_store_16(event, pos, con_handle);
    pos += 2;
    little_endian_store_16(event, pos, mtu);

    // also dispatch to GATT Clients
    att_dispatch_server_mtu_exchanged(con_handle, mtu);

    // dispatch to app level handler and service handlers
    att_emit_event_to_all(&event[0], sizeof(event));
}
#ifndef REDUCE_ROM2///
static void att_emit_can_send_now_event(void * context){
    UNUSED(context);
    if (!att_client_packet_handler) return;

    uint8_t event[] = { ATT_EVENT_CAN_SEND_NOW, 0};
    (*att_client_packet_handler)(HCI_EVENT_PACKET, 0, &event[0], sizeof(event));
}
#endif
#ifndef REDUCE_ROM2///
static void att_handle_value_indication_timeout(btstack_timer_source_t *ts){
	
	if(PATCH_FUN[ATT_HANDLE_VALUE_INDICATION_TIMEOUT_ID]){
         ((void (*)(btstack_timer_source_t *ts))PATCH_FUN[ATT_HANDLE_VALUE_INDICATION_TIMEOUT_ID])(ts);
         return ;
    }

    void * context = btstack_run_loop_get_timer_context(ts);
    hci_con_handle_t con_handle = (hci_con_handle_t) (uintptr_t) context;
    att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server) return;
    // @note: after a transcation timeout, no more requests shall be sent over this ATT Bearer 
    // (that's why we don't reset the value_indication_handle)
    uint16_t att_handle = att_server->value_indication_handle;
    att_handle_value_indication_notify_client(ATT_HANDLE_VALUE_INDICATION_TIMEOUT, att_server->connection.con_handle, att_handle);
}
#endif
static void att_event_packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){

	if(PATCH_FUN[ATT_EVENT_PACKET_HANDLER_ID]){
         ((void (*) (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size))PATCH_FUN[ATT_EVENT_PACKET_HANDLER_ID])(packet_type,   channel,  packet,   size);
         return ;
    }

	
    UNUSED(channel); // ok: there is no channel
    UNUSED(size);    // ok: handling own l2cap events
    
    att_server_t * att_server;
    hci_con_handle_t con_handle;

    switch (packet_type) {
            
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                
                case HCI_EVENT_LE_META:
                    switch (packet[2]) {
                        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                            con_handle = little_endian_read_16(packet, 4);
                            att_server = att_server_for_handle(con_handle);
                            if (!att_server) break;
                        	// store connection info 
                        	att_server->peer_addr_type = packet[7];
                            reverse_bd_addr(&packet[8], att_server->peer_address);
                            att_server->connection.con_handle = con_handle;
                            // reset connection properties
                            att_server->state = ATT_SERVER_IDLE;
                            att_server->connection.mtu = ATT_DEFAULT_MTU;
                            att_server->connection.max_mtu = l2cap_max_le_mtu();
                            if (att_server->connection.max_mtu > ATT_REQUEST_BUFFER_SIZE){
                                att_server->connection.max_mtu = ATT_REQUEST_BUFFER_SIZE;
                            }
                            att_server->connection.encryption_key_size = 0;
                            att_server->connection.authenticated = 0;
		                	att_server->connection.authorized = 0;
                            // workaround: identity resolving can already be complete, at least store result
                            att_server->ir_le_device_db_index = sm_le_device_index(con_handle);
                            att_server->pairing_active = 0;
                            // notify all
                            att_emit_event_to_all(packet, size);
                            break;

                        default:
                            break;
                    }
                    break;

                case HCI_EVENT_ENCRYPTION_CHANGE: 
                case HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE: 
                	// check handle
                    con_handle = little_endian_read_16(packet, 3);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) break;
                    att_server->connection.encryption_key_size = gap_encryption_key_size(con_handle);
                    att_server->connection.authenticated = gap_authenticated(con_handle);
                    att_server->connection.secure_connection = gap_secure_connection(con_handle);
                    //*//*log_info("encrypted key size %u, authenticated %u, secure connectipon %u",
                    //    att_server->connection.encryption_key_size, att_server->connection.authenticated, att_server->connection.secure_connection);
                    if (hci_event_packet_get_type(packet) == HCI_EVENT_ENCRYPTION_CHANGE){
                        // restore CCC values when encrypted
                        if (hci_event_encryption_change_get_encryption_enabled(packet)){
                            att_server_persistent_ccc_restore(att_server);
                        } 
                    }
                    att_run_for_context(att_server);
                    break;

                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    // check handle
                    con_handle = hci_event_disconnection_complete_get_connection_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) break;
                    att_clear_transaction_queue(&att_server->connection);
                    att_server->connection.con_handle = 0;
                    att_server->pairing_active = 0;
                    att_server->state = ATT_SERVER_IDLE;
                    if (att_server->value_indication_handle){
                        btstack_run_loop_remove_timer(&att_server->value_indication_timer);
                        uint16_t att_handle = att_server->value_indication_handle;
                        att_server->value_indication_handle = 0; // reset error state
                        att_handle_value_indication_notify_client(ATT_HANDLE_VALUE_INDICATION_DISCONNECT, att_server->connection.con_handle, att_handle);
                    }
                    // notify all
                    att_emit_event_to_all(packet, size);
                    break;
                    
                // Identity Resolving
                case SM_EVENT_IDENTITY_RESOLVING_STARTED:
                    con_handle = sm_event_identity_resolving_started_get_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) break;
                    //*//*log_info("SM_EVENT_IDENTITY_RESOLVING_STARTED");
                    att_server->ir_lookup_active = 1;
                    break;
                case SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED:
                    con_handle = sm_event_identity_created_get_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) return;
                    att_server->ir_lookup_active = 0;
                    att_server->ir_le_device_db_index = sm_event_identity_resolving_succeeded_get_index(packet);
                    //*//*log_info("SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED");
                    att_run_for_context(att_server);
                    break;
                case SM_EVENT_IDENTITY_RESOLVING_FAILED:
                    con_handle = sm_event_identity_resolving_failed_get_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) break;
                    //*//*log_info("SM_EVENT_IDENTITY_RESOLVING_FAILED");
                    att_server->ir_lookup_active = 0;
                    att_server->ir_le_device_db_index = -1;
                    att_run_for_context(att_server);
                    break;

                // Pairing started - delete stored CCC values
                // - assumes pairing indicates either new device or re-pairing, in both cases there should be no stored CCC values
                // - assumes that all events have the con handle as the first field
                case SM_EVENT_JUST_WORKS_REQUEST:
                case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
                case SM_EVENT_PASSKEY_INPUT_NUMBER:
                case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
                    con_handle = sm_event_just_works_request_get_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) break;
                    att_server->pairing_active = 1;
                    //*//*log_info("SM Pairing started");
                    if (att_server->ir_le_device_db_index < 0) break;
                    att_server_persistent_ccc_clear(att_server);
                    // index not valid anymore
                    att_server->ir_le_device_db_index = -1;
                    break;

                // Bonding completed
                case SM_EVENT_IDENTITY_CREATED:
                    con_handle = sm_event_identity_created_get_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) return;
                    att_server->pairing_active = 0;
                    att_server->ir_le_device_db_index = sm_event_identity_created_get_index(packet);
                    att_run_for_context(att_server);
                    break;

                // Pairing complete (with/without bonding=storing of pairing information)
                case SM_EVENT_PAIRING_COMPLETE:
                    con_handle = sm_event_pairing_complete_get_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) return;
                    att_server->pairing_active = 0;
                    att_run_for_context(att_server);
                    break;

                // Authorization
                case SM_EVENT_AUTHORIZATION_RESULT: {
                    con_handle = sm_event_authorization_result_get_handle(packet);
                    att_server = att_server_for_handle(con_handle);
                    if (!att_server) break;
                    att_server->connection.authorized = sm_event_authorization_result_get_authorization_result(packet);
                    att_dispatch_server_request_can_send_now_event(con_handle);
                	break;
                }
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

#ifdef ENABLE_LE_SIGNED_WRITE
static void att_signed_write_handle_cmac_result(uint8_t hash[8]){
    
    att_server_t * att_server = att_server_for_state(ATT_SERVER_W4_SIGNED_WRITE_VALIDATION);
    if (!att_server) return;

    uint8_t hash_flipped[8];
    reverse_64(hash, hash_flipped);
    if (memcmp(hash_flipped, &att_server->request_buffer[att_server->request_size-8], 8)){
        //*//*log_info("ATT Signed Write, invalid signature");
        att_server->state = ATT_SERVER_IDLE;
        return;
    }
    //*//*log_info("ATT Signed Write, valid signature");

    // update sequence number
    uint32_t counter_packet = little_endian_read_32(att_server->request_buffer, att_server->request_size-12);
    le_device_db_remote_counter_set(att_server->ir_le_device_db_index, counter_packet+1);
    att_server->state = ATT_SERVER_REQUEST_RECEIVED_AND_VALIDATED;
    att_dispatch_server_request_can_send_now_event(att_server->connection.con_handle);
}
#endif

// pre: att_server->state == ATT_SERVER_REQUEST_RECEIVED_AND_VALIDATED
// pre: can send now
// returns: 1 if packet was sent
static int att_server_process_validated_request(att_server_t * att_server){

	
	if(PATCH_FUN[ATT_SERVER_PROCESS_VALIDATED_REQUEST_ID]){

         return ((int (*)(att_server_t * att_server))PATCH_FUN[ATT_SERVER_PROCESS_VALIDATED_REQUEST_ID])(att_server);
    }


    l2cap_reserve_packet_buffer();
    uint8_t * att_response_buffer = l2cap_get_outgoing_buffer();
    uint16_t  att_response_size   = att_handle_request(&att_server->connection, att_server->request_buffer, att_server->request_size, att_response_buffer);

#ifdef ENABLE_ATT_DELAYED_RESPONSE
    if (att_response_size == ATT_READ_RESPONSE_PENDING || att_response_size == ATT_INTERNAL_WRITE_RESPONSE_PENDING){
        // update state
        att_server->state = ATT_SERVER_RESPONSE_PENDING;

        // callback with handle ATT_READ_RESPONSE_PENDING for reads
        if (att_response_size == ATT_READ_RESPONSE_PENDING){
            att_server_client_read_callback(att_server->connection.con_handle, ATT_READ_RESPONSE_PENDING, 0, NULL, 0);
        }

        // free reserved buffer
        l2cap_release_packet_buffer();
        return 0;
    }
#endif

    // intercept "insufficient authorization" for authenticated connections to allow for user authorization
    if ((att_response_size     >= 4)
    && (att_response_buffer[0] == ATT_ERROR_RESPONSE)
    && (att_response_buffer[4] == ATT_ERROR_INSUFFICIENT_AUTHORIZATION)
    && (att_server->connection.authenticated)){

        switch (gap_authorization_state(att_server->connection.con_handle)){
            case AUTHORIZATION_UNKNOWN:
                l2cap_release_packet_buffer();
                sm_request_pairing(att_server->connection.con_handle);
                return 0;
            case AUTHORIZATION_PENDING:
                l2cap_release_packet_buffer();
                return 0;
            default:
                break;
        }
    }

    att_server->state = ATT_SERVER_IDLE;
    if (att_response_size == 0) {
        l2cap_release_packet_buffer();
        return 0;
    }

    l2cap_send_prepared_connectionless(att_server->connection.con_handle, L2CAP_CID_ATTRIBUTE_PROTOCOL, att_response_size);

    // notify client about MTU exchange result
    if (att_response_buffer[0] == ATT_EXCHANGE_MTU_RESPONSE){
        att_emit_mtu_event(att_server->connection.con_handle, att_server->connection.mtu);
    }
    return 1;
}

#ifdef ENABLE_ATT_DELAYED_RESPONSE
int att_server_response_ready(hci_con_handle_t con_handle){
    att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server)                                        return ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER;
    if (att_server->state != ATT_SERVER_RESPONSE_PENDING)   return ERROR_CODE_COMMAND_DISALLOWED;

    att_server->state = ATT_SERVER_REQUEST_RECEIVED_AND_VALIDATED;
    att_dispatch_server_request_can_send_now_event(con_handle);
    return ERROR_CODE_SUCCESS;
}
#endif

static void att_run_for_context(att_server_t * att_server){
	
	if(PATCH_FUN[ATT_RUN_FOR_CONTEXT_ID]){
         ((void (*)(att_server_t * att_server))PATCH_FUN[ATT_RUN_FOR_CONTEXT_ID])(att_server);
         return ;
    }

    switch (att_server->state){
        case ATT_SERVER_REQUEST_RECEIVED:

            // wait until re-encryption as central is complete
            if (gap_reconnect_security_setup_active(att_server->connection.con_handle)) break;

            // wait until pairing is complete
            if (att_server->pairing_active) break;

#ifdef ENABLE_LE_SIGNED_WRITE
            if (att_server->request_buffer[0] == ATT_SIGNED_WRITE_COMMAND){
                //*//*log_info("ATT Signed Write!");
                if (!sm_cmac_ready()) {
                    //*//*log_info("ATT Signed Write, sm_cmac engine not ready. Abort");
                    att_server->state = ATT_SERVER_IDLE;
                    return;
                }  
                if (att_server->request_size < (3 + 12)) {
                    //*//*log_info("ATT Signed Write, request to short. Abort.");
                    att_server->state = ATT_SERVER_IDLE;
                    return;
                }
                if (att_server->ir_lookup_active){
                    return;
                }
                if (att_server->ir_le_device_db_index < 0){
                    //*//*log_info("ATT Signed Write, CSRK not available");
                    att_server->state = ATT_SERVER_IDLE;
                    return;
                }

                // check counter
                uint32_t counter_packet = little_endian_read_32(att_server->request_buffer, att_server->request_size-12);
                uint32_t counter_db     = le_device_db_remote_counter_get(att_server->ir_le_device_db_index);
                //*//*log_info("ATT Signed Write, DB counter %"PRIu32", packet counter %"PRIu32, counter_db, counter_packet);
                if (counter_packet < counter_db){
                    //*//*log_info("ATT Signed Write, db reports higher counter, abort");
                    att_server->state = ATT_SERVER_IDLE;
                    return;
                }

                // signature is { sequence counter, secure hash }
                sm_key_t csrk;
                le_device_db_remote_csrk_get(att_server->ir_le_device_db_index, csrk);
                att_server->state = ATT_SERVER_W4_SIGNED_WRITE_VALIDATION;
                //*//*log_info("Orig Signature: ");
                //*log_info_hexdump( &att_server->request_buffer[att_server->request_size-8], 8);
                uint16_t attribute_handle = little_endian_read_16(att_server->request_buffer, 1);
                sm_cmac_signed_write_start(csrk, att_server->request_buffer[0], attribute_handle, att_server->request_size - 15, &att_server->request_buffer[3], counter_packet, att_signed_write_handle_cmac_result);
                return;
            } 
#endif
            // move on
            att_server->state = ATT_SERVER_REQUEST_RECEIVED_AND_VALIDATED;
            att_dispatch_server_request_can_send_now_event(att_server->connection.con_handle);
            break;

        default:
            break;
    }   
}

static int att_server_data_ready_for_phase(att_server_t * att_server,  att_server_run_phase_t phase){
    if(PATCH_FUN[ATT_SERVER_DATA_READY_FOR_PHASE_ID]){
         
         return ((int (*)(att_server_t * att_server,  att_server_run_phase_t phase))PATCH_FUN[ATT_SERVER_DATA_READY_FOR_PHASE_ID])(att_server,    phase);
    }

	switch (phase){
        case ATT_SERVER_RUN_PHASE_1_REQUESTS:
            return att_server->state == ATT_SERVER_REQUEST_RECEIVED_AND_VALIDATED;
        case ATT_SERVER_RUN_PHASE_2_INDICATIONS:
             return (!btstack_linked_list_empty(&att_server->indication_requests) && att_server->value_indication_handle == 0);
        case ATT_SERVER_RUN_PHASE_3_NOTIFICATIONS:
            return (!btstack_linked_list_empty(&att_server->notification_requests));
    }
    // avoid warning
    return 0;
}

static void att_server_trigger_send_for_phase(att_server_t * att_server,  att_server_run_phase_t phase){
    
	if(PATCH_FUN[ATT_SERVER_TRIGGER_SEND_FOR_PHASE_ID]){
         ((void (*)(att_server_t * att_server,  att_server_run_phase_t phase))PATCH_FUN[ATT_SERVER_TRIGGER_SEND_FOR_PHASE_ID])(att_server,    phase); 
         return ;
    }

	btstack_context_callback_registration_t * client;
    switch (phase){
        case ATT_SERVER_RUN_PHASE_1_REQUESTS:
            att_server_process_validated_request(att_server);
            break;
        case ATT_SERVER_RUN_PHASE_2_INDICATIONS:
            client = (btstack_context_callback_registration_t*) att_server->indication_requests;
            btstack_linked_list_remove(&att_server->indication_requests, (btstack_linked_item_t *) client);
            client->callback(client->context);
            break;
       case ATT_SERVER_RUN_PHASE_3_NOTIFICATIONS:
            client = (btstack_context_callback_registration_t*) att_server->notification_requests;
            btstack_linked_list_remove(&att_server->notification_requests, (btstack_linked_item_t *) client);
            client->callback(client->context);
            break;
    }
}

static void att_server_handle_can_send_now(void){
    if(PATCH_FUN[ATT_SERVER_HANDLE_CAN_SEND_NOW_ID]){
         ((void (*)(void))PATCH_FUN[ATT_SERVER_HANDLE_CAN_SEND_NOW_ID])(); 
         return ;
    }

    hci_con_handle_t request_con_handle   = HCI_CON_HANDLE_INVALID;
    hci_con_handle_t last_send_con_handle = HCI_CON_HANDLE_INVALID;
    int can_send_now = 1;
    int phase_index;

    for (phase_index = ATT_SERVER_RUN_PHASE_1_REQUESTS; phase_index <= ATT_SERVER_RUN_PHASE_3_NOTIFICATIONS; phase_index++){
        att_server_run_phase_t phase = (att_server_run_phase_t) phase_index;
        hci_con_handle_t skip_connections_until = att_server_last_can_send_now;
        while (1){
            btstack_linked_list_iterator_t it;
            hci_connections_get_iterator(&it);
            while(btstack_linked_list_iterator_has_next(&it)){
                hci_connection_t * connection = (hci_connection_t *) btstack_linked_list_iterator_next(&it);
                att_server_t * att_server = &connection->att_server;

                int data_ready = att_server_data_ready_for_phase(att_server, phase);

                // //*log_debug("phase %u, handle 0x%04x, skip until 0x%04x, data ready %u", phase, att_server->connection.con_handle, skip_connections_until, data_ready);

                // skip until last sender found (which is also skipped)
                if (skip_connections_until != HCI_CON_HANDLE_INVALID){
                    if (data_ready && request_con_handle == HCI_CON_HANDLE_INVALID){
                        request_con_handle = att_server->connection.con_handle;
                    }
                    if (skip_connections_until == att_server->connection.con_handle){
                        skip_connections_until = HCI_CON_HANDLE_INVALID;
                    }
                    continue;
                };

                if (data_ready){
                    if (can_send_now){
                        att_server_trigger_send_for_phase(att_server, phase);
                        last_send_con_handle = att_server->connection.con_handle;
                        can_send_now = att_dispatch_server_can_send_now(att_server->connection.con_handle);
                        data_ready = att_server_data_ready_for_phase(att_server, phase);
                        if (data_ready && request_con_handle == HCI_CON_HANDLE_INVALID){
                            request_con_handle = att_server->connection.con_handle;
                        }
                    } else {
                        request_con_handle = att_server->connection.con_handle;
                        break;
                    }
                }
            }

            // stop skipping (handles disconnect by last send connection)
            skip_connections_until = HCI_CON_HANDLE_INVALID;

            // Exit loop, if we cannot send
            if (!can_send_now) break;

            // Exit loop, if we can send but there are also no further request
            if (request_con_handle == HCI_CON_HANDLE_INVALID) break;

            // Finally, if we still can send and there are requests, just try again
            request_con_handle = HCI_CON_HANDLE_INVALID;
        }
        // update last send con handle for round robin
        if (last_send_con_handle != HCI_CON_HANDLE_INVALID){
            att_server_last_can_send_now = last_send_con_handle;
        }
    }

    if (request_con_handle == HCI_CON_HANDLE_INVALID) return;
    att_dispatch_server_request_can_send_now_event(request_con_handle);
}

static void att_packet_handler(uint8_t packet_type, uint16_t handle, uint8_t *packet, uint16_t size){
	
	if(PATCH_FUN[ATT_PACKET_HANDLER_ID]){
         ((void (*)(uint8_t packet_type, uint16_t handle, uint8_t *packet, uint16_t size))PATCH_FUN[ATT_PACKET_HANDLER_ID])(packet_type,   handle,  packet,   size);
         return ;
    }

    att_server_t * att_server;

    switch (packet_type){
        case HCI_EVENT_PACKET:
            switch (packet[0]){
                case L2CAP_EVENT_CAN_SEND_NOW:
                    att_server_handle_can_send_now();
                    break;
                case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
                    // GATT client has negotiated the mtu for this connection
                    att_server = att_server_for_handle(handle);
                    if (!att_server) break;
                    att_server->connection.mtu = little_endian_read_16(packet, 4);
                    break;
                default:
                    break;
            }
            break;

        case ATT_DATA_PACKET:
            //*log_debug("ATT Packet, handle 0x%04x", handle);
            att_server = att_server_for_handle(handle);
            if (!att_server) break;

            // handle value indication confirms
            if (packet[0] == ATT_HANDLE_VALUE_CONFIRMATION && att_server->value_indication_handle){
                btstack_run_loop_remove_timer(&att_server->value_indication_timer);
                uint16_t att_handle = att_server->value_indication_handle;
                att_server->value_indication_handle = 0;    
                att_handle_value_indication_notify_client(0, att_server->connection.con_handle, att_handle);
                att_dispatch_server_request_can_send_now_event(att_server->connection.con_handle);
                return;
            }

            // directly process command
            // note: signed write cannot be handled directly as authentication needs to be verified
            if (packet[0] == ATT_WRITE_COMMAND){
                att_handle_request(&att_server->connection, packet, size, 0);
                return;
            }

            // check size
            if (size > sizeof(att_server->request_buffer)) {
                //*//*log_info("att_packet_handler: dropping att pdu 0x%02x as size %u > att_server->request_buffer %u", packet[0], size, (int) sizeof(att_server->request_buffer));
                return;
            }

            // last request still in processing?
            if (att_server->state != ATT_SERVER_IDLE){
                //*//*log_info("att_packet_handler: skipping att pdu 0x%02x as server not idle (state %u)", packet[0], att_server->state);
                return;
            }

            // store request
            att_server->state = ATT_SERVER_REQUEST_RECEIVED;
            att_server->request_size = size;
            memcpy(att_server->request_buffer, packet, size);
        
            att_run_for_context(att_server);
            break;
    }
}

// ---------------------
// persistent CCC writes
static uint32_t att_server_persistent_ccc_tag_for_index(uint8_t index){
    return 'B' << 24 | 'T' << 16 | 'C' << 8 | index;
}

static void att_server_persistent_ccc_write(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t value){
	
	if(PATCH_FUN[ATT_SERVER_PERSISTENT_CCC_WRITE_ID]){
         ((void (*)(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t value))PATCH_FUN[ATT_SERVER_PERSISTENT_CCC_WRITE_ID])(con_handle,   att_handle,   value);
         return ;
    }

    // lookup att_server instance
    att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server) return;
    int le_device_index = att_server->ir_le_device_db_index;
    //*//*log_info("Store CCC value 0x%04x for handle 0x%04x of remote %s, le device id %d", value, att_handle, bd_addr_to_str(att_server->peer_address), le_device_index);

    // check if bonded
    if (le_device_index < 0) return;

    // get btstack_tlv
    const btstack_tlv_t * tlv_impl = NULL;
    void * tlv_context;
    btstack_tlv_get_instance(&tlv_impl, &tlv_context);
    if (!tlv_impl) return;

    // update ccc tag
    int index;
    uint32_t highest_seq_nr = 0;
    uint32_t lowest_seq_nr = 0;
    uint32_t tag_for_lowest_seq_nr = 0;
    uint32_t tag_for_empty = 0;
    persistent_ccc_entry_t entry;
    for (index=0;index<NVN_NUM_GATT_SERVER_CCC;index++){
        uint32_t tag = att_server_persistent_ccc_tag_for_index(index);
        int len = tlv_impl->get_tag(tlv_context, tag, (uint8_t *) &entry, sizeof(persistent_ccc_entry_t));

        // empty/invalid tag
        if (len != sizeof(persistent_ccc_entry_t)){
            tag_for_empty = tag;
            continue;
        }
        // update highest seq nr
        if (entry.seq_nr > highest_seq_nr){
            highest_seq_nr = entry.seq_nr;
        }
        // find entry with lowest seq nr
        if ((tag_for_lowest_seq_nr == 0) || (entry.seq_nr < lowest_seq_nr)){
            tag_for_lowest_seq_nr = tag;
            lowest_seq_nr = entry.seq_nr;
        }

        if (entry.device_index != le_device_index) continue;
        if (entry.att_handle   != att_handle)      continue;

        // found matching entry
        if (value){
            // update
            if (entry.value == value) {
                //*//*//*log_info("CCC Index %u: Up-to-date", index);
                return;
            }
            entry.value = value;
            entry.seq_nr = highest_seq_nr + 1;
            //*//*log_info("CCC Index %u: Store", index);
            tlv_impl->store_tag(tlv_context, tag, (const uint8_t *) &entry, sizeof(persistent_ccc_entry_t));
        } else {
            // delete
            //*//*log_info("CCC Index %u: Delete", index);
            tlv_impl->delete_tag(tlv_context, tag);
        }
        return;
    }

    //*//*log_info("tag_for_empy %"PRIx32", tag_for_lowest_seq_nr %"PRIx32, tag_for_empty, tag_for_lowest_seq_nr);

    if (value == 0){
        // done
        return;
    }

    uint32_t tag_to_use = 0;
    if (tag_for_empty){
        tag_to_use = tag_for_empty;
    } else if (tag_for_lowest_seq_nr){
        tag_to_use = tag_for_lowest_seq_nr;
    } else {
        // should not happen
        return;
    }
    // store ccc tag
    entry.seq_nr       = highest_seq_nr + 1;
    entry.device_index = le_device_index;
    entry.att_handle   = att_handle;
    entry.value        = value;
    tlv_impl->store_tag(tlv_context, tag_to_use, (uint8_t *) &entry, sizeof(persistent_ccc_entry_t));
}

static void att_server_persistent_ccc_clear(att_server_t * att_server){
	
	if(PATCH_FUN[ATT_SERVER_PERSISTENT_CCC_CLEAR_ID]){
         ((void (*)(att_server_t * att_server))PATCH_FUN[ATT_SERVER_PERSISTENT_CCC_CLEAR_ID])(att_server);
         return ;
    }

    if (!att_server) return;
    int le_device_index = att_server->ir_le_device_db_index;
    //*//*log_info("Clear %s, le device id %d", bd_addr_to_str(att_server->peer_address), le_device_index);
    // check if bonded
    if (le_device_index < 0) return;
    // get btstack_tlv
    const btstack_tlv_t * tlv_impl = NULL;
    void * tlv_context;
    btstack_tlv_get_instance(&tlv_impl, &tlv_context);
    if (!tlv_impl) return;
    // get all ccc tag
    int index;
    persistent_ccc_entry_t entry;
    for (index=0;index<NVN_NUM_GATT_SERVER_CCC;index++){
        uint32_t tag = att_server_persistent_ccc_tag_for_index(index);
        int len = tlv_impl->get_tag(tlv_context, tag, (uint8_t *) &entry, sizeof(persistent_ccc_entry_t));
        if (len != sizeof(persistent_ccc_entry_t)) continue;
        if (entry.device_index != le_device_index) continue;
        // delete entry
        //*//*log_info("CCC Index %u: Delete", index);
        tlv_impl->delete_tag(tlv_context, tag);
    }  
}

static void att_server_persistent_ccc_restore(att_server_t * att_server){
	
	if(PATCH_FUN[ATT_SERVER_PERSISTENT_CCC_RESTORE_ID]){
         ((void (*)(att_server_t * att_server))PATCH_FUN[ATT_SERVER_PERSISTENT_CCC_RESTORE_ID])(att_server); 
         return ;
    }

    if (!att_server) return;
    int le_device_index = att_server->ir_le_device_db_index;
    //*//*log_info("Restore CCC values of remote %s, le device id %d", bd_addr_to_str(att_server->peer_address), le_device_index);
    // check if bonded
    if (le_device_index < 0) return;
    // get btstack_tlv
    const btstack_tlv_t * tlv_impl = NULL;
    void * tlv_context;
    btstack_tlv_get_instance(&tlv_impl, &tlv_context);
    if (!tlv_impl) return;
    // get all ccc tag
    int index;
    persistent_ccc_entry_t entry;
    for (index=0;index<NVN_NUM_GATT_SERVER_CCC;index++){
        uint32_t tag = att_server_persistent_ccc_tag_for_index(index);
        int len = tlv_impl->get_tag(tlv_context, tag, (uint8_t *) &entry, sizeof(persistent_ccc_entry_t));
        if (len != sizeof(persistent_ccc_entry_t)) continue;
        if (entry.device_index != le_device_index) continue;
        // simulate write callback
        uint16_t attribute_handle = entry.att_handle;
        uint8_t  value[2];
        little_endian_store_16(value, 0, entry.value);
        att_write_callback_t callback = att_server_write_callback_for_handle(attribute_handle);
        if (!callback) continue;
        //*//*log_info("CCC Index %u: Set Attribute handle 0x%04x to value 0x%04x", index, attribute_handle, entry.value );
        (*callback)(att_server->connection.con_handle, attribute_handle, ATT_TRANSACTION_MODE_NONE, 0, value, sizeof(value));
    }
}

// persistent CCC writes
// ---------------------

// gatt service management
static att_service_handler_t * att_service_handler_for_handle(uint16_t handle){
    btstack_linked_list_iterator_t it;
    btstack_linked_list_iterator_init(&it, &service_handlers);
    while (btstack_linked_list_iterator_has_next(&it)){
        att_service_handler_t * handler = (att_service_handler_t*) btstack_linked_list_iterator_next(&it);
        if (handler->start_handle > handle) continue;
        if (handler->end_handle   < handle) continue;
        return handler;
    }
    return NULL;
}
static att_read_callback_t att_server_read_callback_for_handle(uint16_t handle){
    att_service_handler_t * handler = att_service_handler_for_handle(handle);
    if (handler) return handler->read_callback;
    return att_server_client_read_callback;
}

static att_write_callback_t att_server_write_callback_for_handle(uint16_t handle){
    att_service_handler_t * handler = att_service_handler_for_handle(handle);
    if (handler) return handler->write_callback;
    return att_server_client_write_callback;
}

static btstack_packet_handler_t att_server_packet_handler_for_handle(uint16_t handle){
    att_service_handler_t * handler = att_service_handler_for_handle(handle);
    if (handler) return handler->packet_handler;
    return att_client_packet_handler;
}

static void att_notify_write_callbacks(hci_con_handle_t con_handle, uint16_t transaction_mode){
    // notify all callbacks
    btstack_linked_list_iterator_t it;
    btstack_linked_list_iterator_init(&it, &service_handlers);
    while (btstack_linked_list_iterator_has_next(&it)){
        att_service_handler_t * handler = (att_service_handler_t*) btstack_linked_list_iterator_next(&it);
        if (!handler->write_callback) continue;
        (*handler->write_callback)(con_handle, 0, transaction_mode, 0, NULL, 0);
    }
    if (!att_server_client_write_callback) return;
    (*att_server_client_write_callback)(con_handle, 0, transaction_mode, 0, NULL, 0);
}

// returns first reported error or 0
static uint8_t att_validate_prepared_write(hci_con_handle_t con_handle){
    btstack_linked_list_iterator_t it;
    btstack_linked_list_iterator_init(&it, &service_handlers);
    while (btstack_linked_list_iterator_has_next(&it)){
        att_service_handler_t * handler = (att_service_handler_t*) btstack_linked_list_iterator_next(&it);
        if (!handler->write_callback) continue;
        uint8_t error_code = (*handler->write_callback)(con_handle, 0, ATT_TRANSACTION_MODE_VALIDATE, 0, NULL, 0);
        if (error_code) return error_code;
    }
    if (!att_server_client_write_callback) return 0;
    return (*att_server_client_write_callback)(con_handle, 0, ATT_TRANSACTION_MODE_VALIDATE, 0, NULL, 0);
}

static uint16_t att_server_read_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
    att_read_callback_t callback = att_server_read_callback_for_handle(attribute_handle);
    if (!callback) return 0;
    return (*callback)(con_handle, attribute_handle, offset, buffer, buffer_size);
}

static int att_server_write_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
   
    if(PATCH_FUN[ATT_SERVER_WRITE_CALLBACK_ID]){
         
         return ((int (*)(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size))PATCH_FUN[ATT_SERVER_WRITE_CALLBACK_ID])(con_handle,attribute_handle,transaction_mode,offset,buffer,buffer_size);
    }

	switch (transaction_mode){
        case ATT_TRANSACTION_MODE_VALIDATE:
            return att_validate_prepared_write(con_handle);
        case ATT_TRANSACTION_MODE_EXECUTE:
        case ATT_TRANSACTION_MODE_CANCEL:
            att_notify_write_callbacks(con_handle, transaction_mode);
            return 0;
        default:
            break;
    }

    // track CCC writes
    if (att_is_persistent_ccc(attribute_handle) && offset == 0 && buffer_size == 2){
        att_server_persistent_ccc_write(con_handle, attribute_handle, little_endian_read_16(buffer, 0));
    }

    att_write_callback_t callback = att_server_write_callback_for_handle(attribute_handle);
    if (!callback) return 0;
    return (*callback)(con_handle, attribute_handle, transaction_mode, offset, buffer, buffer_size);
}

/**
 * @brief register read/write callbacks for specific handle range
 * @param att_service_handler_t
 */
void att_server_register_service_handler(att_service_handler_t * handler){
    if (att_service_handler_for_handle(handler->start_handle) ||
        att_service_handler_for_handle(handler->end_handle)){
        //*log_error("handler for range 0x%04x-0x%04x already registered", handler->start_handle, handler->end_handle);
        return;
    }
    btstack_linked_list_add(&service_handlers, (btstack_linked_item_t*) handler);
}

void att_server_init(uint8_t const * db, att_read_callback_t read_callback, att_write_callback_t write_callback){

   if(PATCH_FUN[ATT_SERVER_INIT_ID]){
         ((void (*)(uint8_t const * db, att_read_callback_t read_callback, att_write_callback_t write_callback))PATCH_FUN[ATT_SERVER_INIT_ID])(db,   read_callback,   write_callback);
         return ;
    }

    // store callbacks
    att_server_client_read_callback  = read_callback;
    att_server_client_write_callback = write_callback;

    // register for HCI Events
    hci_event_callback_registration1.callback = &att_event_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration1);

    // register for SM events
    sm_event_callback_registration.callback = &att_event_packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    // and L2CAP ATT Server PDUs
    att_dispatch_register_server(att_packet_handler);

    att_set_db(db);
    att_set_read_callback(att_server_read_callback);
    att_set_write_callback(att_server_write_callback);

}

void att_server_register_packet_handler(btstack_packet_handler_t handler){
    att_client_packet_handler = handler;    
}

#ifndef REDUCE_ROM2///
// to be deprecated
int  att_server_can_send_packet_now(hci_con_handle_t con_handle){
    return att_dispatch_server_can_send_now(con_handle);
}
#endif

int att_server_register_can_send_now_callback(btstack_context_callback_registration_t * callback_registration, hci_con_handle_t con_handle){
    return att_server_request_to_send_notification(callback_registration, con_handle);
}
#ifndef REDUCE_ROM2///
void att_server_request_can_send_now_event(hci_con_handle_t con_handle){
    att_client_waiting_for_can_send_registration.callback = &att_emit_can_send_now_event;
    att_server_request_to_send_notification(&att_client_waiting_for_can_send_registration, con_handle);
}
// end of deprecated
#endif

int att_server_request_to_send_notification(btstack_context_callback_registration_t * callback_registration, hci_con_handle_t con_handle){
    att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server) return ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER;
    btstack_linked_list_add_tail(&att_server->notification_requests, (btstack_linked_item_t*) callback_registration);
    att_dispatch_server_request_can_send_now_event(con_handle);
    return ERROR_CODE_SUCCESS;
}
#ifndef REDUCE_ROM2///
int att_server_request_to_send_indication(btstack_context_callback_registration_t * callback_registration, hci_con_handle_t con_handle){
    att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server) return ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER;
    btstack_linked_list_add_tail(&att_server->indication_requests, (btstack_linked_item_t*) callback_registration);
    att_dispatch_server_request_can_send_now_event(con_handle);
    return ERROR_CODE_SUCCESS;
}
#endif

int att_server_notify(hci_con_handle_t con_handle, uint16_t attribute_handle, const uint8_t *value, uint16_t value_len){
    if(PATCH_FUN[ATT_SERVER_NOTIFY_ID]){
         
         return ((int (*)(hci_con_handle_t con_handle, uint16_t attribute_handle, const uint8_t *value, uint16_t value_len))PATCH_FUN[ATT_SERVER_NOTIFY_ID])(con_handle,attribute_handle, value, value_len);
    }

	
	att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server) return ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER;

    if (!att_dispatch_server_can_send_now(con_handle)) return BTSTACK_ACL_BUFFERS_FULL;

    l2cap_reserve_packet_buffer();
    uint8_t * packet_buffer = l2cap_get_outgoing_buffer();
    uint16_t size = att_prepare_handle_value_notification(&att_server->connection, attribute_handle, value, value_len, packet_buffer);
	return l2cap_send_prepared_connectionless(att_server->connection.con_handle, L2CAP_CID_ATTRIBUTE_PROTOCOL, size);
}

int att_server_indicate(hci_con_handle_t con_handle, uint16_t attribute_handle, const uint8_t *value, uint16_t value_len){
    
	if(PATCH_FUN[ATT_SERVER_INDICATE_ID]){
         
         return ((int (*)(hci_con_handle_t con_handle, uint16_t attribute_handle, const uint8_t *value, uint16_t value_len))PATCH_FUN[ATT_SERVER_INDICATE_ID])(con_handle, attribute_handle, value, value_len) ;
    }
#ifndef REDUCE_ROM2///

	att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server) return ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER;

    if (att_server->value_indication_handle) return ATT_HANDLE_VALUE_INDICATION_IN_PROGRESS;
    if (!att_dispatch_server_can_send_now(con_handle)) return BTSTACK_ACL_BUFFERS_FULL;

    // track indication
    att_server->value_indication_handle = attribute_handle;
    btstack_run_loop_set_timer_handler(&att_server->value_indication_timer, att_handle_value_indication_timeout);
    btstack_run_loop_set_timer(&att_server->value_indication_timer, ATT_TRANSACTION_TIMEOUT_MS);
    btstack_run_loop_add_timer(&att_server->value_indication_timer);

    l2cap_reserve_packet_buffer();
    uint8_t * packet_buffer = l2cap_get_outgoing_buffer();
    uint16_t size = att_prepare_handle_value_indication(&att_server->connection, attribute_handle, value, value_len, packet_buffer);
	l2cap_send_prepared_connectionless(att_server->connection.con_handle, L2CAP_CID_ATTRIBUTE_PROTOCOL, size);
    #endif
		return 0;
}
#ifndef REDUCE_ROM2///
uint16_t att_server_get_mtu(hci_con_handle_t con_handle){
    att_server_t * att_server = att_server_for_handle(con_handle);
    if (!att_server) return 0;
    return att_server->connection.mtu;
}
#endif