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

//#define __BTSTACK_FILE__ "ancs_client_demo.c"

// *****************************************************************************
//
// ANCS Client Demo
//
// TODO: query full text upon notification using control point
// TODO: present notifications in human readable form
//
// *****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "btstack_config.h"

#include "btstack_run_loop.h"

#include "btstack_debug.h"
#include "btstack_event.h"
#include "btstack_memory.h"
#include "gap.h"
#include "hci.h"
#include "hci_dump.h"
#include "l2cap.h"

#include "ancs_client.h"
#include "att_db.h"
#include "att_server.h"
#include "gatt_client.h"
#include "le_device_db.h"
#include "sm.h"

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

static void app_packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case SM_EVENT_JUST_WORKS_REQUEST:
                    sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
                    printf("Just Works Confirmed.\n");
                    break;
                case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
                    printf("Passkey display: %"PRIu32"\n", sm_event_passkey_display_number_get_passkey(packet));
                    break;
            }
            break;
    }
}

static void ancs_callback(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    const char * attribute_name;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_ANCS_META) return;
    switch (hci_event_ancs_meta_get_subevent_code(packet)){
        case ANCS_SUBEVENT_CLIENT_CONNECTED:
            printf("ANCS Client: Connected\n");
            break;
        case ANCS_SUBEVENT_CLIENT_DISCONNECTED:
            printf("ANCS Client: Disconnected\n");
            break;
        case ANCS_SUBEVENT_CLIENT_NOTIFICATION:
            attribute_name = ancs_client_attribute_name_for_id(ancs_subevent_client_notification_get_attribute_id(packet));
            if (!attribute_name) break;
            printf("Notification: %s - %s\n", attribute_name, ancs_subevent_client_notification_get_text(packet));
            break;
        default:
            break;
    }
}

int btstack_ancs_main(int argc, const char * argv[]);
int btstack_ancs_main(int argc, const char * argv[]){
    (void)argc;
    (void)argv;

    printf("BTstack ANCS Client starting up...\n");


    // set up l2cap_le
    l2cap_init();
    
    // setup le device db
    le_device_db_init();

    // setup SM: Display only
//    sm_init();
//	sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
//    sm_set_authentication_requirements(SM_AUTHREQ_BONDING); 
	
//    sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY);
//    sm_set_authentication_requirements( SM_AUTHREQ_BONDING ); 

    // register for HCI events
    hci_event_callback_registration.callback = &app_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for SM events
//    sm_event_callback_registration.callback = &app_packet_handler;
//    sm_add_event_handler(&sm_event_callback_registration);

    // setup ATT server
//    att_server_init(profile_data, NULL, NULL);    

    // setup ANCS Client
    ancs_client_init();


    // register for ATT Serer events
    att_server_register_packet_handler(app_packet_handler);

    // setup GATT client
    gatt_client_init();

    // register for ancs events
    ancs_client_register_callback(&ancs_callback);

    // turn on!
    hci_power_control(HCI_POWER_ON);
    
    return 0;
}
