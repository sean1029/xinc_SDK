
#ifndef __HCI_TRANSPORT_RAM_VAL_H
#define __HCI_TRANSPORT_RAM_VAL_H

#include <stdint.h>
#include "sys_hal_types.h"
#include "sys_types.h"
#include "btstack_defines.h"
#include "btstack_run_loop.h"
#include "hci.h"

typedef enum {
    RX_IDLE = 1,
    RX_DONE
} RX_STATE;

typedef enum {
    TX_IDLE = 1,
    TX_DONE
} TX_STATE;

// write state
extern TX_STATE tx_state; 
//// packet reader state machine
extern RX_STATE rx_state;
extern uint16_t rx_size;

extern void dummy_handler(uint8_t packet_type, uint8_t *packet, uint16_t size);


extern uint8_t packet_sent_event[2];
extern void (*packet_handler)(uint8_t packet_type, uint8_t *packet, uint16_t size);
extern unsigned char *TRAhcit_Generic_Get_Rx_Buf(uint8_t ptype, int size, uint8_t *hci_header);
extern void TRAhcit_Generic_Commit_Rx_Buf(unsigned char buffer_type); 	
extern t_error TRAhcit_Register_Generic_Dispatcher(uint32_t (*callback)(uint8_t *data, uint32_t pdu_len, uint8_t *header, uint8_t head_len));

// incoming packet buffer
extern uint8_t hci_packet_with_pre_buffer[HCI_INCOMING_PRE_BUFFER_SIZE + HCI_INCOMING_PACKET_BUFFER_SIZE + 1]; // packet type + max(acl header + acl payload, event header + event data)
extern uint8_t * hci_packet ;//= &hci_packet_with_pre_buffer[HCI_INCOMING_PRE_BUFFER_SIZE];

// data source for integration with BTstack Runloop
extern btstack_data_source_t transport_data_source;

extern struct tra_hcit_info sys_hcit_generic_info;


#endif // __HCI_TRANSPORT_RAM_VAL_H

