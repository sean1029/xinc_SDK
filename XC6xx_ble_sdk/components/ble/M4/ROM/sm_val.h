

#ifndef __SM_VAL_H
#define __SM_VAL_H

 #if defined __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "btstack_util.h"
#include "btstack_defines.h"
#include "hci.h"
#include "btstack_tlv.h"
#include "btstack_crypto.h"

#define BTSTACK_TAG32(A,B,C,D) ((A << 24) | (B << 16) | (C << 8) | D)

//
// GLOBAL DATA
//

extern uint8_t test_use_fixed_local_csrk;

#ifdef ENABLE_TESTING_SUPPORT
extern uint8_t test_pairing_failure;
#endif

// configuration
extern uint8_t sm_accepted_stk_generation_methods;
extern uint8_t sm_max_encryption_key_size;
extern uint8_t sm_min_encryption_key_size;
extern uint8_t sm_auth_req;// = 1;
extern uint8_t sm_io_capabilities;// = IO_CAPABILITY_NO_INPUT_NO_OUTPUT;
extern uint8_t sm_slave_request_security;
extern uint32_t sm_fixed_passkey_in_display_role;
extern uint8_t sm_reconstruct_ltk_without_le_device_db_entry;

#ifdef ENABLE_LE_SECURE_CONNECTIONS
extern uint8_t sm_sc_oob_random[16];
extern sm_sc_oob_state_t sm_sc_oob_state;
#endif


extern uint8_t               sm_persistent_keys_random_active;
extern /*const*/ btstack_tlv_t * sm_tlv_impl;
extern void *                sm_tlv_context;

// Security Manager Master Keys, please use sm_set_er(er) and sm_set_ir(ir) with your own 128 bit random values
extern sm_key_t sm_persistent_er;
extern sm_key_t sm_persistent_ir;

typedef enum {
    DKG_W4_WORKING,
    DKG_CALC_IRK,
    DKG_CALC_DHK,
    DKG_READY
} derived_key_generation_t;

// derived from sm_persistent_ir
extern sm_key_t sm_persistent_dhk;
extern sm_key_t sm_persistent_irk;
extern derived_key_generation_t dkg_state;

// derived from sm_persistent_er
// ..

// random address update
//extern random_address_update_t rau_state;
extern bd_addr_t sm_random_address;

#ifdef USE_CMAC_ENGINE
// CMAC Calculation: General
extern btstack_crypto_aes128_cmac_t sm_cmac_request;
extern void (*sm_cmac_done_callback)(uint8_t hash[8]);
extern uint8_t sm_cmac_active;
extern uint8_t sm_cmac_hash[16];
#endif

// CMAC for ATT Signed Writes
#ifdef ENABLE_LE_SIGNED_WRITE
extern  uint16_t        sm_cmac_signed_write_message_len;
extern  uint8_t         sm_cmac_signed_write_header[3];
extern /* const*/ uint8_t * sm_cmac_signed_write_message;
extern  uint8_t         sm_cmac_signed_write_sign_counter[4];
#endif

// CMAC for Secure Connection functions
#ifdef ENABLE_LE_SECURE_CONNECTIONS
extern sm_connection_t * sm_cmac_connection;
extern uint8_t           sm_cmac_sc_buffer[80];
#endif


typedef enum {
    JUST_WORKS,
    PK_RESP_INPUT,       // Initiator displays PK, responder inputs PK
    PK_INIT_INPUT,       // Responder displays PK, initiator inputs PK
    PK_BOTH_INPUT,       // Only input on both, both input PK
    NUMERIC_COMPARISON,  // Only numerical compparison (yes/no) on on both sides
    OOB                  // OOB available on one (SC) or both sides (legacy)
} stk_generation_method_t;

// resolvable private address lookup / CSRK calculation
extern int       sm_address_resolution_test;
extern int       sm_address_resolution_ah_calculation_active;
extern uint8_t   sm_address_resolution_addr_type;
extern bd_addr_t sm_address_resolution_address;
extern void *    sm_address_resolution_context;
//extern address_resolution_mode_t sm_address_resolution_mode;
extern btstack_linked_list_t sm_address_resolution_general_queue;

// aes128 crypto engine.
//extern sm_aes128_state_t  sm_aes128_state;

// crypto 
extern btstack_crypto_random_t   sm_crypto_random_request;
extern btstack_crypto_aes128_t   sm_crypto_aes128_request;
#ifdef ENABLE_LE_SECURE_CONNECTIONS
extern  btstack_crypto_ecc_p256_t sm_crypto_ecc_p256_request;
extern  btstack_crypto_random_t   sm_crypto_random_oob_request;
#endif

// temp storage for random data
extern uint8_t sm_random_data[8];
extern uint8_t sm_aes128_key[16];
extern uint8_t sm_aes128_plaintext[16];
extern uint8_t sm_aes128_ciphertext[16];

// to receive hci events
extern   btstack_packet_callback_registration_t hci_event_callback_registration4;

/* to dispatch sm event */
extern  btstack_linked_list_t sm_event_handlers;

// LE Secure Connections
#ifdef ENABLE_LE_SECURE_CONNECTIONS
extern  ec_key_generation_state_t ec_key_generation_state;
extern  uint8_t ec_q[64];
#endif

//
// Volume 3, Part H, Chapter 24
// "Security shall be initiated by the Security Manager in the device in the master role.
// The device in the slave role shall be the responding device."
// -> master := initiator, slave := responder
//

// data needed for security setup
typedef struct sm_setup_context {

    btstack_timer_source_t sm_timeout;

    // used in all phases
    uint8_t   sm_pairing_failed_reason;

    // user response, (Phase 1 and/or 2)
    uint8_t   sm_user_response;
    uint8_t   sm_keypress_notification; // bitmap: passkey started, digit entered, digit erased, passkey cleared, passkey complete, 3 bit count

    // defines which keys will be send after connection is encrypted - calculated during Phase 1, used Phase 3
    int       sm_key_distribution_send_set;
    int       sm_key_distribution_received_set;

    // Phase 2 (Pairing over SMP)
    stk_generation_method_t sm_stk_generation_method;
    sm_key_t  sm_tk;
    uint8_t   sm_have_oob_data;
    uint8_t   sm_use_secure_connections;

    sm_key_t  sm_c1_t3_value;   // c1 calculation
    sm_pairing_packet_t sm_m_preq; // pairing request - needed only for c1
    sm_pairing_packet_t sm_s_pres; // pairing response - needed only for c1
    sm_key_t  sm_local_random;
    sm_key_t  sm_local_confirm;
    sm_key_t  sm_peer_random;
    sm_key_t  sm_peer_confirm;
    uint8_t   sm_m_addr_type;   // address and type can be removed
    uint8_t   sm_s_addr_type;   //  ''
    bd_addr_t sm_m_address;     //  ''
    bd_addr_t sm_s_address;     //  ''
    sm_key_t  sm_ltk;

    uint8_t   sm_state_vars;
#ifdef ENABLE_LE_SECURE_CONNECTIONS
    uint8_t   sm_peer_q[64];    // also stores random for EC key generation during init
    sm_key_t  sm_peer_nonce;    // might be combined with sm_peer_random
    sm_key_t  sm_local_nonce;   // might be combined with sm_local_random
    sm_key_t  sm_dhkey;
    sm_key_t  sm_peer_dhkey_check;
    sm_key_t  sm_local_dhkey_check;
    sm_key_t  sm_ra;
    sm_key_t  sm_rb;
    sm_key_t  sm_t;             // used for f5 and h6
    sm_key_t  sm_mackey;
    uint8_t   sm_passkey_bit;   // also stores number of generated random bytes for EC key generation
#endif

    // Phase 3

    // key distribution, we generate
    uint16_t  sm_local_y;
    uint16_t  sm_local_div;
    uint16_t  sm_local_ediv;
    uint8_t   sm_local_rand[8];
    sm_key_t  sm_local_ltk;
    sm_key_t  sm_local_csrk;
    sm_key_t  sm_local_irk;
    // sm_local_address/addr_type not needed

    // key distribution, received from peer
    uint16_t  sm_peer_y;
    uint16_t  sm_peer_div;
    uint16_t  sm_peer_ediv;
    uint8_t   sm_peer_rand[8];
    sm_key_t  sm_peer_ltk;
    sm_key_t  sm_peer_irk;
    sm_key_t  sm_peer_csrk;
    uint8_t   sm_peer_addr_type;
    bd_addr_t sm_peer_address;

} sm_setup_context_t;

//
extern sm_setup_context_t the_setup;
extern sm_setup_context_t * setup;// = &the_setup;

// active connection - the one for which the_setup is used for
extern uint16_t sm_active_connection_handle;// = HCI_CON_HANDLE_INVALID;

// @returns 1 if oob data is available
// stores oob data in provided 16 byte buffer if not null
extern int (*sm_get_oob_data)(uint8_t addres_type, bd_addr_t addr, uint8_t * oob_data);// = NULL;
extern int (*sm_get_sc_oob_data)(uint8_t addres_type, bd_addr_t addr, uint8_t * oob_sc_peer_confirm, uint8_t * oob_sc_peer_random);

//gap_random_address_type_t gap_random_adress_type;
extern btstack_timer_source_t gap_random_address_update_timer;
extern uint32_t gap_random_adress_update_period;


#if defined __cplusplus
}
#endif

#endif // __SM_VAL_H
