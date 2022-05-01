/*************************************************************************
 * MODULE NAME:    tra_queue.c
 * PROJECT CODE:   Bluestream
 * DESCRIPTION:    Queue functions for all Queues in the Bluetooth 
 *                 host controller stack
 * AUTHOR:         HC Team
 * DATE:           28/03/2000
 *
 * SOURCE CONTROL: $Id: tra_queue.c,v 1.129 2014/03/11 03:14:17 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *      November 2000 - Added fixed size chunk allocation scheme for L2CAP data
 *
 * ISSUES:
 * NOTES TO USERS: 
 *      March 2002      P7C major restructure of queues
 *
 *      Not             Segmentation of variable length queues removed
 *************************************************************************/
#include "sys_config.h"

#include <assert.h>
#include "tra_queue.h"

#include "hw_macro_defs.h"
#include "hw_memcpy.h"
#include "sys_irq.h"
#include "sys_atomic.h"
#include "le_connection.h"
#include "le_security.h"

#include "lc_interface.h"
#include "lslc_access.h"
#include "dl_dev.h"

#include "bt_test.h"
#include "bt_timer.h"
#include "patch_function_id.h"

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
#include "bt_mini_sched.h"
#endif

#include "hc_event_gen.h"
#include "hc_const.h"
#include "sys_debug_config.h"
#include "patch_function_id.h"
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)

#if     (__DEBUG_PCM_VOICE__ == 1)                          //- add (#if #endif).
#define TRA_QUEUE_SYN_AGGREGATION_ON_RECEIVE		0
#else
#define TRA_QUEUE_SYN_AGGREGATION_ON_RECEIVE		1
#endif

#define TRA_QUEUE_SYN_MAX_LATENCY 20 /* nominal 6.25ms */

#if     (__DEBUG_PCM_VOICE__ == 1)                          //-  add (#if #endif).
#define TRA_QUEUE_SCO_CROSS_BUFFER_FRAGMENTATION	0
#else
#define TRA_QUEUE_SCO_CROSS_BUFFER_FRAGMENTATION	1
#endif

#endif

/*
 * Some compile checks against options and queue sizes
 */
#if (PRH_BS_CFG_SYS_LMP_FIVE_SLOT_PKT_SUPPORTED==1)
#if (PRH_BS_CFG_SELECT_LEVEL!=1)
    #if (PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH < MAX_DH5_USER_PDU) || \
            (PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH < MAX_DH5_USER_PDU)
    #error L2CAP packets must support at least DH5 (339 bytes) since 5-slot supported
    #endif
#endif
#elif PRH_BS_CFG_SYS_LMP_THREE_SLOT_PKT_SUPPORTED==1
    #if (PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH < MAX_DH3_USER_PDU) || \
            (PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH < MAX_DH3_USER_PDU)
    #error L2CAP packets must support at least DH3 (183 bytes) since 3-slot supported
    #endif
#else /*1 slot packets only */
    #if (PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH < MAX_DH1_USER_PDU) || \
            (PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH < MAX_DH1_USER_PDU)
    #error L2CAP packets must support at least DH1 (27 bytes) since 1-slot supported
    #endif
#endif

#if (MAX_ACTIVE_DEVICE_LINKS > 255)
#error device_index in each t_q_descr is u_int8, please revisit
#endif

/*
 * Static space for queues (word aligned heap and an array of headers)
 */
typedef union
{
    u_int32 _dummy;
    u_int8 hci_event_heap_array_[PRH_BS_CFG_SYS_SIZEOF_HCI_EVENT_HEAP];
} t_event_heap;
extern t_event_heap event_heap_;

extern u_int8* hci_event_heap ;
extern t_q_descr hci_event_headers[PRH_BS_CFG_SYS_MAX_HCI_EVENTS];

typedef union
{
    u_int32 _dummy;
    u_int8 hci_command_heap_array_[PRH_BS_CFG_SYS_SIZEOF_HCI_COMMAND_HEAP];
} t_command_heap;
extern t_command_heap command_heap_;

extern u_int8* hci_command_heap ;
extern t_q_descr hci_command_headers[PRH_BS_CFG_SYS_MAX_HCI_COMMANDS];


typedef union
{
    u_int32 _dummy;
    u_int8 lmp_in_heap_array_[PRH_BS_CFG_SYS_SIZEOF_LMP_IN_HEAP];
} t_lmp_in_heap;
extern t_lmp_in_heap lmp_in_heap_;

extern u_int8* lmp_in_heap  ;
extern t_q_descr lmp_in_headers[PRH_BS_CFG_SYS_MAX_LMP_IN_MESSAGES];

extern u_int8 lmp_out_heaps[MAX_ACTIVE_DEVICE_LINKS][PRH_BS_CFG_SYS_SIZEOF_LMP_OUT_HEAP];
extern t_q_descr lmp_out_headers[MAX_ACTIVE_DEVICE_LINKS][PRH_BS_CFG_SYS_MAX_LMP_OUT_MESSAGES];    


#if (PRH_BS_CFG_SYS_TRACE_VIA_HCI_SUPPORTED==1)
/*
 * Setup a distinct queue for logging
 */
extern u_int8 hci_debug_heap[PRH_BS_CFG_SYS_SIZEOF_HCI_DEBUG_HEAP];
extern t_q_descr hci_debug_headers[PRH_BS_CFG_SYS_MAX_HCI_DEBUG_EVENTS];
#endif

/* 
 *  Declare a container for all the queues 
 */
extern t_queue BTQueues[TRA_QUEUE_TOTAL_NUM_QUEUES];

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
extern t_q_descr eSCO_Tx_Descriptor[PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI];
extern u_int8 eSCO_Tx_Buffers[PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI][PRH_BS_CFG_SYS_SIZEOF_ESCO_OUT_HEAP];
#endif

#if (defined(SYS_DEBUG_INJECT_SCO_BELOW_HCI) || defined(SYS_DEBUG_INJECT_ESCO_BELOW_HCI))
//Input data is sign-mag as a 1k sinwave: 0x0000,0x5A82,0x7FFF,0x5A82,0x0000,0xA57E,0x8001,0xA57E.
#if 0 // 16 samples sine wave 500hz
#define SYS_DEBUG_SCO_TEST_VECTOR_REPEAT_PERIOD 32 /* pattern repeats every 32 bytes */
#if 1 // 25% amplitude
u_int8 temp_test_buffer[192] = 
{          0x00,0x00,  0x3F,0x0C,  0xA0,0x16,  0x90,0x1D,  0x00,0x20,  0x90,0x1D,  0xA0,0x16,  0x3F,0x0C,
           0x00,0x00,  0xC1,0xF3,  0x60,0xE9,  0x70,0xE2,  0x00,0xE0,  0x70,0xE2,  0x60,0xE9,  0xC1,0xF3,
		   0x00,0x00,  0x3F,0x0C,  0xA0,0x16,  0x90,0x1D,  0x00,0x20,  0x90,0x1D,  0xA0,0x16,  0x3F,0x0C,
           0x00,0x00,  0xC1,0xF3,  0x60,0xE9,  0x70,0xE2,  0x00,0xE0,  0x70,0xE2,  0x60,0xE9,  0xC1,0xF3,
		   0x00,0x00,  0x3F,0x0C,  0xA0,0x16,  0x90,0x1D,  0x00,0x20,  0x90,0x1D,  0xA0,0x16,  0x3F,0x0C,
           0x00,0x00,  0xC1,0xF3,  0x60,0xE9,  0x70,0xE2,  0x00,0xE0,  0x70,0xE2,  0x60,0xE9,  0xC1,0xF3,
		   0x00,0x00,  0x3F,0x0C,  0xA0,0x16,  0x90,0x1D,  0x00,0x20,  0x90,0x1D,  0xA0,0x16,  0x3F,0x0C,
           0x00,0x00,  0xC1,0xF3,  0x60,0xE9,  0x70,0xE2,  0x00,0xE0,  0x70,0xE2,  0x60,0xE9,  0xC1,0xF3,
		   0x00,0x00,  0x3F,0x0C,  0xA0,0x16,  0x90,0x1D,  0x00,0x20,  0x90,0x1D,  0xA0,0x16,  0x3F,0x0C,
           0x00,0x00,  0xC1,0xF3,  0x60,0xE9,  0x70,0xE2,  0x00,0xE0,  0x70,0xE2,  0x60,0xE9,  0xC1,0xF3,
		   0x00,0x00,  0x3F,0x0C,  0xA0,0x16,  0x90,0x1D,  0x00,0x20,  0x90,0x1D,  0xA0,0x16,  0x3F,0x0C,
           0x00,0x00,  0xC1,0xF3,  0x60,0xE9,  0x70,0xE2,  0x00,0xE0,  0x70,0xE2,  0x60,0xE9,  0xC1,0xF3 };
#else
u_int8 temp_test_buffer[192] = 
{          0x00,0x00,  0xFB,0x30,  0x82,0x5A,  0x41,0x76,  0xFF,0x7F,  0x41,0x76,  0x82,0x5A,  0xFB,0x30,
           0x00,0x00,  0x05,0xCF,  0x7E,0xA5,  0xBF,0x89,  0x01,0x80,  0xBF,0x89,  0x7E,0xA5,  0x05,0xCF,
		   0x00,0x00,  0xFB,0x30,  0x82,0x5A,  0x41,0x76,  0xFF,0x7F,  0x41,0x76,  0x82,0x5A,  0xFB,0x30,
           0x00,0x00,  0x05,0xCF,  0x7E,0xA5,  0xBF,0x89,  0x01,0x80,  0xBF,0x89,  0x7E,0xA5,  0x05,0xCF,
		   0x00,0x00,  0xFB,0x30,  0x82,0x5A,  0x41,0x76,  0xFF,0x7F,  0x41,0x76,  0x82,0x5A,  0xFB,0x30,
           0x00,0x00,  0x05,0xCF,  0x7E,0xA5,  0xBF,0x89,  0x01,0x80,  0xBF,0x89,  0x7E,0xA5,  0x05,0xCF,
		   0x00,0x00,  0xFB,0x30,  0x82,0x5A,  0x41,0x76,  0xFF,0x7F,  0x41,0x76,  0x82,0x5A,  0xFB,0x30,
           0x00,0x00,  0x05,0xCF,  0x7E,0xA5,  0xBF,0x89,  0x01,0x80,  0xBF,0x89,  0x7E,0xA5,  0x05,0xCF,
		   0x00,0x00,  0xFB,0x30,  0x82,0x5A,  0x41,0x76,  0xFF,0x7F,  0x41,0x76,  0x82,0x5A,  0xFB,0x30,
           0x00,0x00,  0x05,0xCF,  0x7E,0xA5,  0xBF,0x89,  0x01,0x80,  0xBF,0x89,  0x7E,0xA5,  0x05,0xCF,
		   0x00,0x00,  0xFB,0x30,  0x82,0x5A,  0x41,0x76,  0xFF,0x7F,  0x41,0x76,  0x82,0x5A,  0xFB,0x30,
           0x00,0x00,  0x05,0xCF,  0x7E,0xA5,  0xBF,0x89,  0x01,0x80,  0xBF,0x89,  0x7E,0xA5,  0x05,0xCF };
#endif
#else // 8 samples sine wave 1khz
#define SYS_DEBUG_SCO_TEST_VECTOR_REPEAT_PERIOD 16 /* pattern repeats every 16 bytes */
#if 1 // 25% amplitude
u_int8 temp_test_buffer[192] = 
{		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
           0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
           0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9,
		   0x00,0x00,  0xA0,0x16,  0x00,0x20,  0xA0,0x16,  0x00,0x00,  0x60,0xE9,  0x00,0xE0,  0x60,0xE9 };
#else // 100% amplitude
u_int8 temp_test_buffer[192] = 
{          0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
           0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
           0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5,
		   0x00,0x00,  0x82,0x5A,  0xFF,0x7F,  0x82,0x5A,  0x00,0x00,  0x7E,0xA5,  0x01,0x80,  0x7E,0xA5 };
#endif
#endif
static t_q_descr test_qd;
static u_int8    temp_test_buffer_ptr=0;
static u_int8    dummy_voice_txed=0;
#endif
/*****************************************************************************
 * mBTq_Get_Q_Ref
 *
 * Return a reference to the Q for the given
 *
 * q_type
 * device_index
 * 
 * q_type               device_index        q_index offset

 * HCI_COMMAND_Q        0                   q_type
 * HCI_EVENT_Q          0                   q_type
 * LMP_IN_Q             ANY                 q_type
 * LMP_OUT_Q            ANY                 q_type + device_index
 * L2CAP_IN_Q           ANY                 q_type + device_index
 * L2CAP_OUT_Q          ANY                 q_type + device_index
 ****************************************************************************/
#define mBTq_Get_Q_Ref(q_type, device_index) \
    (BTQueues + (q_type) + (((q_type)==LMP_IN_Q) ? 0 : (device_index)))

/*****************************************************************************
 * _BTq_Initialise_Variable_Chunk_Queue
 *
 * Initialises the specified variable chunk sized queue 
 * i.e. the header array and data heap pointers
 ****************************************************************************/
static void _BTq_Initialise_Variable_Chunk_Queue(
    t_queue *q, t_q_descr *headers, 
    u_int8 num_headers, u_int8 *data_ptr,  u_int16 data_length)
{
    
	if(PATCH_FUN[_BTQ_INITIALISE_VARIABLE_CHUNK_QUEUE_ID]){
         ((void (*)(t_queue *q, t_q_descr *headers, u_int8 num_headers, u_int8 *data_ptr,  u_int16 data_length))PATCH_FUN[_BTQ_INITIALISE_VARIABLE_CHUNK_QUEUE_ID])(q,headers,num_headers,data_ptr,data_length);
         return ;
    }

	u_int16 i;

    /* 
     * Zero the header array equiv memset(&headers[i], 0, sizeof(t_q_descr) );
     */
    for(i=0;i<num_headers;i++)
    {
        u_int16 j = sizeof(t_q_descr);
        u_int8 *ptr=(u_int8*) &headers[i];

        while (j-- != 0)
        {
            *ptr++ = 0;
        }
    }

    q->headers = headers;

    q->num_headers = num_headers;
    q->desc_head = q->desc_tail = 0;

    q->head_offset = q->tail_offset = 0;
    q->end_offset = q->data_size = data_length;
    q->q_data = data_ptr;

    q->num_processed_chunks = 0;
    q->original_length = 0;
    q->rollback_length = 0; 
    q->num_entries = 0;
    q->pending_commits = 0;

	q->total_data_enqueued = 0;
}


/*****************************************************************************
 * _BTq_Initialise_Data_Queue
 *
 * Initialises the specified queue i.e. the header array and data heap pointers
 ****************************************************************************/
static void _BTq_Initialise_Data_Queue(t_queue *q)
{
    q->headers = 0x0;
    q->headers_tail = 0x0;

    q->num_processed_chunks = 0;
    q->original_length = 0;
    q->rollback_length = 0; 
    q->num_entries = 0;
    q->pending_commits = 0;

	q->total_data_enqueued = 0;
}
/*
 * The data queues now adopt a single generic data queue strategy
 * to support both ACL and SCO.
 * The queue definitions are held in RAM allowing them to
 * to be completely reconfigured after initialisation.
 * Each data queue group is completely defined by t_data_queue.
 *
 * Notes:
 * 1.  L2CAP (ACL buffers) and SCO buffers are aligned to a multiple of 
 *     PRH_BS_CFG_SYS_ACL_BUFFER_ALIGNMENT.
 *     For SCO buffers its not required.
 * 2.  IN:  Radio Baseband to Host       OUT: Host to Radio Baseband
 * 3.  ACL/SCO data is handled differently because of flow control
 * 4.  All data queues are now stored in a single RAM area which can
 *     later be completely restructured to any required size.
 * 5.  Each queue has
 *     a/  A stack of free header addresses
 *     b/  An array of headers
 *     c/  A data heap of fixed buffer sizes
 */

/*
 * Queue descriptions for L2CAP queues,
 * Deliberately placed before BTq_data_memory for test validation
 */
extern t_data_queue BTq_l2cap_in;
extern t_data_queue BTq_l2cap_out;

extern struct s_all_queues
{
    /*
     * ACL Queues
     */
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)//no used
    t_q_descr **l2cap_in_free_header_list;
    t_q_descr *l2cap_in_headers;
    u_int8 *l2cap_in_heaps;

    t_q_descr **l2cap_out_free_header_list;
    t_q_descr *l2cap_out_headers;
    u_int8 *l2cap_out_heaps;
#else
    t_q_descr *l2cap_in_free_header_list[PRH_BS_CFG_SYS_NUM_IN_ACL_PACKETS];
    t_q_descr l2cap_in_headers[PRH_BS_CFG_SYS_NUM_IN_ACL_PACKETS];
    u_int8 l2cap_in_heaps[PRH_BS_CFG_SYS_NUM_IN_ACL_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH)];

    t_q_descr *l2cap_out_free_header_list[PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS];
    t_q_descr l2cap_out_headers[PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS];
    u_int8 l2cap_out_heaps[PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH)];
#endif

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    /*
     * SCO Queues
     */
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)
    t_q_descr **sco_in_free_header_list;
    t_q_descr *sco_in_headers;
    u_int8 *sco_in_heaps;
    
    t_q_descr **sco_out_free_header_list;
    t_q_descr *sco_out_headers;
    u_int8 *sco_out_heaps;
#else
    t_q_descr *sco_in_free_header_list[PRH_BS_CFG_SYS_NUM_IN_SCO_PACKETS];
    t_q_descr sco_in_headers[PRH_BS_CFG_SYS_NUM_IN_SCO_PACKETS];
    u_int8 sco_in_heaps[PRH_BS_CFG_SYS_NUM_IN_SCO_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH)];

    t_q_descr *sco_out_free_header_list[PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS];
    t_q_descr sco_out_headers[PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS];
    u_int8 sco_out_heaps[PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH)];
#endif

#endif
} BTq_data_memory;


#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
/*
 * Queue descriptions for SCO queues,
 * Deliberately placed after BTq_data_memory for test validation
 */
extern t_data_queue BTq_sco_in;
extern t_data_queue BTq_sco_out;
extern boolean BTq_sco_in_data_aggregation;
#endif

#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1) //no used
/*****************************************************************************
 * _BTq_Allocate_Data_Buffers
 *
 * Allocates memory from heap for data buffer dimensions specified.
 ****************************************************************************/
extern t_SYS_Config g_sys_config;
extern void (*HC_MemFree)(void*);
int _BTq_Allocate_Data_Buffers( void* (*mem_alloc)(u_int32 size),
                                u_int16 num_in_acl_packets,
                                u_int16 num_out_acl_packets,
                                u_int16 num_in_sco_packets,
                                u_int16 num_out_sco_packets)
{
    g_sys_config.hc_buffer_size.numScoDataPackets = num_out_sco_packets;
    g_sys_config.hc_buffer_size.numAclDataPackets = num_out_acl_packets;
    g_sys_config.hc_buffer_size_in.numScoDataPackets = num_in_sco_packets;
    g_sys_config.hc_buffer_size_in.numAclDataPackets = num_in_acl_packets;
    
    if(num_in_acl_packets)
    {   
        BTq_data_memory.l2cap_in_free_header_list = (t_q_descr **) mem_alloc(
            num_in_acl_packets * sizeof(t_q_descr*));
        
        BTq_data_memory.l2cap_in_headers = (t_q_descr *) mem_alloc(
            num_in_acl_packets * sizeof(t_q_descr));
        
        BTq_data_memory.l2cap_in_heaps = (u_int8 *) mem_alloc(
            num_in_acl_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH));

        if( (BTq_data_memory.l2cap_in_free_header_list == 0)
         || (BTq_data_memory.l2cap_in_headers == 0)
         || (BTq_data_memory.l2cap_in_heaps == 0))
		{
			_BTq_Deallocate_Data_Buffers((void(*)(void*))HC_MemFree);
            return 1;
		}

		/*
		 * The memory should only be initialized if the allocation was sucessful.
		 */
		memset(BTq_data_memory.l2cap_in_free_header_list, 0, 
			num_in_acl_packets * sizeof(t_q_descr*)); 
		memset(BTq_data_memory.l2cap_in_headers, 0,
			num_in_acl_packets * sizeof(t_q_descr));
		memset(BTq_data_memory.l2cap_in_heaps, 0,
			num_in_acl_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH));
    }
    else
    {
        BTq_data_memory.l2cap_in_free_header_list = NULL;
        BTq_data_memory.l2cap_in_headers = NULL;
        BTq_data_memory.l2cap_in_heaps = NULL;
    }
    
    if(num_out_acl_packets)
    {
        BTq_data_memory.l2cap_out_free_header_list = (t_q_descr **) mem_alloc(
            num_out_acl_packets * sizeof(t_q_descr*));
        
		BTq_data_memory.l2cap_out_headers = (t_q_descr *) mem_alloc(
            num_out_acl_packets * sizeof(t_q_descr));

        BTq_data_memory.l2cap_out_heaps = (u_int8 *) mem_alloc(
            num_out_acl_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH));
            
        if( (BTq_data_memory.l2cap_out_free_header_list == 0)
         || (BTq_data_memory.l2cap_out_headers == 0)
         || (BTq_data_memory.l2cap_out_heaps == 0))
		{
			_BTq_Deallocate_Data_Buffers((void(*)(void*))HC_MemFree);
            return 1;
		}

		/*
		 * The memory should only be initialized if the allocation was sucessful.
		 */
		memset(BTq_data_memory.l2cap_out_free_header_list, 0, 
			num_out_acl_packets * sizeof(t_q_descr*)); 
		memset(BTq_data_memory.l2cap_out_headers, 0, 
			num_out_acl_packets * sizeof(t_q_descr)); 
		memset(BTq_data_memory.l2cap_out_heaps, 0,
			num_out_acl_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH));
    }
    else
    {
        BTq_data_memory.l2cap_out_free_header_list = NULL;
        BTq_data_memory.l2cap_out_headers = NULL;
        BTq_data_memory.l2cap_out_heaps = NULL;
    }
        
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)

    if(num_in_sco_packets)
    {
        BTq_data_memory.sco_in_free_header_list = (t_q_descr **) mem_alloc(
            num_in_sco_packets * sizeof(t_q_descr*));

        BTq_data_memory.sco_in_headers = (t_q_descr *) mem_alloc(
            num_in_sco_packets * sizeof(t_q_descr));

        BTq_data_memory.sco_in_heaps = (u_int8 *) mem_alloc(
            num_in_sco_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH));

        if( (BTq_data_memory.sco_in_free_header_list == 0)
         || (BTq_data_memory.sco_in_headers == 0)
         || (BTq_data_memory.sco_in_heaps == 0))
		{
			_BTq_Deallocate_Data_Buffers((void(*)(void*))HC_MemFree);
            return 1;
		}

		/*
		 * The memory should only be initialized if the allocation was sucessful.
		 */
		memset(BTq_data_memory.sco_in_free_header_list, 0, 
			num_in_sco_packets * sizeof(t_q_descr*)); 
        memset(BTq_data_memory.sco_in_headers, 0, 
			num_in_sco_packets * sizeof(t_q_descr)); 
	    memset(BTq_data_memory.sco_in_heaps, 0, 
			num_in_sco_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH));

    }
    else
    {
        BTq_data_memory.sco_in_free_header_list = NULL;
        BTq_data_memory.sco_in_headers = NULL;
        BTq_data_memory.sco_in_heaps = NULL;
    }

    if(num_out_sco_packets)
    {
        BTq_data_memory.sco_out_free_header_list = (t_q_descr **) mem_alloc(
            num_out_sco_packets * sizeof(t_q_descr*));

        BTq_data_memory.sco_out_headers = (t_q_descr *) mem_alloc(
            num_out_sco_packets * sizeof(t_q_descr));

        BTq_data_memory.sco_out_heaps = (u_int8 *) mem_alloc(
            num_out_sco_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH));

        if( (BTq_data_memory.sco_out_free_header_list == 0)
         || (BTq_data_memory.sco_out_headers == 0)
         || (BTq_data_memory.sco_out_heaps == 0))
		{
			_BTq_Deallocate_Data_Buffers((void(*)(void*))HC_MemFree);
            return 1;
		}
		
		/*
		 * The memory should only be initialized if the allocation was sucessful.
		 */
        memset(BTq_data_memory.sco_out_free_header_list, 0, 
			num_out_sco_packets * sizeof(t_q_descr*)); 
		memset(BTq_data_memory.sco_out_headers, 0, 
			num_out_sco_packets * sizeof(t_q_descr)); 
		memset(BTq_data_memory.sco_out_heaps, 0, 
			num_out_sco_packets * sizeof(u_int8) *
            mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH));

    }
    else
    {
        BTq_data_memory.sco_out_free_header_list = NULL;
        BTq_data_memory.sco_out_headers = NULL;
        BTq_data_memory.sco_out_heaps = NULL;
    }
    
#endif

    return 0;
}

/*****************************************************************************
 * _BTq_Free_Data_Buffers
 *
 * Deallocate memory from heap for data buffer dimensions specified.
 ****************************************************************************/
void _BTq_Deallocate_Data_Buffers(void (*mem_dealloc)(void*))
{
    if(BTq_data_memory.l2cap_in_free_header_list)
    {
        mem_dealloc(BTq_data_memory.l2cap_in_free_header_list);
        BTq_data_memory.l2cap_in_free_header_list = NULL;
    }
    
    if(BTq_data_memory.l2cap_in_headers)
    {
        mem_dealloc(BTq_data_memory.l2cap_in_headers);
        BTq_data_memory.l2cap_in_headers = NULL;
    }
    
    if(BTq_data_memory.l2cap_in_heaps)
    {
        mem_dealloc(BTq_data_memory.l2cap_in_heaps);
        BTq_data_memory.l2cap_in_heaps = NULL;
    }
    
    if(BTq_data_memory.l2cap_out_free_header_list)
    {
        mem_dealloc(BTq_data_memory.l2cap_out_free_header_list);
        BTq_data_memory.l2cap_out_free_header_list = NULL;
    }
    
    if(BTq_data_memory.l2cap_out_headers)
    {
        mem_dealloc(BTq_data_memory.l2cap_out_headers);
        BTq_data_memory.l2cap_out_headers = NULL;
    }
    
    if(BTq_data_memory.l2cap_out_heaps)
    {
        mem_dealloc(BTq_data_memory.l2cap_out_heaps);
        BTq_data_memory.l2cap_out_heaps = NULL;
    }

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    if(BTq_data_memory.sco_in_free_header_list)
    {
        mem_dealloc(BTq_data_memory.sco_in_free_header_list);
        BTq_data_memory.sco_in_free_header_list = NULL;
    }
    
    if(BTq_data_memory.sco_in_headers)
    {
        mem_dealloc(BTq_data_memory.sco_in_headers);
        BTq_data_memory.sco_in_headers = NULL;
    }
    
    if(BTq_data_memory.sco_in_heaps)
    {
        mem_dealloc(BTq_data_memory.sco_in_heaps);
        BTq_data_memory.sco_in_heaps = NULL;
    }
    
    if(BTq_data_memory.sco_out_free_header_list)
    {
        mem_dealloc(BTq_data_memory.sco_out_free_header_list);
        BTq_data_memory.sco_out_free_header_list = NULL;
    }
    
    if(BTq_data_memory.sco_out_headers)
    {
        mem_dealloc(BTq_data_memory.sco_out_headers);
        BTq_data_memory.sco_out_headers = NULL;
    }
    
    if(BTq_data_memory.sco_out_heaps)
    {
        mem_dealloc(BTq_data_memory.sco_out_heaps);
        BTq_data_memory.sco_out_heaps = NULL;
    }
#endif
}

#endif



/*****************************************************************************
 * _BTq_Get_Data_Queue_Ref
 *
 * This function returns the data queue descriptor for the identified queue
 * If not a data queue it will return 0
 ****************************************************************************/
t_data_queue *_BTq_Get_Data_Queue_Ref(u_int8 q_type)
{
	if(PATCH_FUN[_BTQ_GET_DATA_QUEUE_REF_ID]){
         
         return ((t_data_queue *(*)(u_int8 q_type))PATCH_FUN[_BTQ_GET_DATA_QUEUE_REF_ID])(q_type);
    }

    t_data_queue *p_data_queue;

    if (q_type == L2CAP_IN_Q)
    {
        p_data_queue = &BTq_l2cap_in;
    }
    else if (q_type == L2CAP_OUT_Q)
    {
        p_data_queue = &BTq_l2cap_out;
    }
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)    
    else if (q_type == SCO_IN_Q)
    {        p_data_queue = &BTq_sco_in;
    }
    else if (q_type == SCO_OUT_Q)
    {
        p_data_queue = &BTq_sco_out;
    }
#endif
    else
    {
        p_data_queue = (t_data_queue *)0; 
    }
    return p_data_queue;    
}

/*****************************************************************************
 * BTq_Initialise_Data_Queue_Type
 *
 * This function initialises the shared queue variables used to manage 
 * a single type of queue 
 *
 * q_type                   L2CAP_IN_Q L2CAP_OUT_Q | SCO_IN_Q | SCO_OUT_Q | 
 * num_of_headers           Number of queue entries supported
 * queue_headers            The queue headers
 * free_header_list         A set of pointers to maintain the free headers
 * max_data_packet_length   Size reported to Host if applicable
 * queue_heap               Queue data area must be large enough to
 *                          support aligned version of queue
 ****************************************************************************/
void BTq_Initialise_Data_Queue_Type(u_int8 q_type,
    u_int16   num_of_headers,
    t_q_descr *queue_headers,
    t_q_descr **free_header_list,
    u_int16   max_data_packet_length,
    u_int8    *queue_heap
)
{
    if(PATCH_FUN[BTQ_INITIALISE_DATA_QUEUE_TYPE_ID]){
         ((void (*)(u_int8 q_type,u_int16 num_of_headers,t_q_descr *queue_headers,t_q_descr **free_header_list,u_int16   max_data_packet_length,u_int8*queue_heap))PATCH_FUN[BTQ_INITIALISE_DATA_QUEUE_TYPE_ID])(q_type,num_of_headers,queue_headers,free_header_list,max_data_packet_length,queue_heap);
         return ;
    }

	
	u_int16 aligned_data_packet_length;
    u_int16 i;

    t_data_queue *p_data_queue = _BTq_Get_Data_Queue_Ref(q_type);

    p_data_queue->next_free_pointer = 0;
    p_data_queue->num_of_consumed_chunks = 0;
    p_data_queue->num_of_headers = num_of_headers;
    p_data_queue->max_data_packet_length = max_data_packet_length;
    p_data_queue->free_header_list = free_header_list;
    p_data_queue->heap = queue_heap;

    /*
     * Set up the global free lists and allocate data buffers to headers
     */
    aligned_data_packet_length = 
        mBTq_Get_Aligned_Data_Packet_Length(max_data_packet_length);

    for(i=0; i<num_of_headers; i++)
    {
        p_data_queue->free_header_list[i]
            = &queue_headers[i];
        p_data_queue->free_header_list[i]->data 
            = p_data_queue->heap + (i*aligned_data_packet_length);
    }
}

/*****************************************************************************
 * _BTq_Initialise_All_Data_Queues
 *
 * This function initialises the shared queue variables used to manage 
 * the global heap of header descriptors and data chunks
 ****************************************************************************/
static void _BTq_Initialise_All_Data_Queues(void)
{
	
	if(PATCH_FUN[_BTQ_INITIALISE_ALL_DATA_QUEUES_ID]){
         ((void (*)(void))PATCH_FUN[_BTQ_INITIALISE_ALL_DATA_QUEUES_ID])();
         return ;
    }

    u_int16 i;

    BTq_Initialise_Data_Queue_Type(L2CAP_IN_Q,      /*q_type*/
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)
        g_sys_config.hc_buffer_size_in.numAclDataPackets,
#else
        PRH_BS_CFG_SYS_NUM_IN_ACL_PACKETS,          /*num_of_headers*/
#endif
        BTq_data_memory.l2cap_in_headers,           /*queue_headers*/
        BTq_data_memory.l2cap_in_free_header_list,  /*free_header_list*/
        PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH,        /*max_data_packet_length*/
        BTq_data_memory.l2cap_in_heaps              /*queue_heap*/ );

    BTq_Initialise_Data_Queue_Type(L2CAP_OUT_Q,     /*q_type*/
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)
        g_sys_config.hc_buffer_size.numAclDataPackets,
#else
        PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS,         /*num_of_headers*/
#endif
        BTq_data_memory.l2cap_out_headers,          /*queue_headers*/
        BTq_data_memory.l2cap_out_free_header_list, /*free_header_list*/
        PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH,       /*max_data_packet_length*/
        BTq_data_memory.l2cap_out_heaps             /*queue_heap*/ );

    /* 
     * Initialise the input and output L2CAP queues 
     *
     * In the case of ACL queues, which now use a global chunk allocation scheme,
     * most of the function arguments are not used inside the BTq_Init function 
     */

    /* 
     * L2CAP input queues
     */
    for(i=0; i<MAX_ACTIVE_DEVICE_LINKS; i++)
    {
        _BTq_Initialise_Data_Queue(BTQueues+L2CAP_IN_Q+i);
    }

    /*
     * L2CAP output queues, the extra is used for the Piconet broadcast queues.
     */
    for(i=0; i<MAX_ACTIVE_DEVICE_LINKS+1; i++)
    {
        _BTq_Initialise_Data_Queue(BTQueues+L2CAP_OUT_Q+i);
    }

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)    

    /* 
     * Initialise the input and output SCO queues and associated groups
     */
    BTq_Initialise_Data_Queue_Type(SCO_IN_Q,        /*q_type*/
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)
        g_sys_config.hc_buffer_size_in.numScoDataPackets,
#else
        PRH_BS_CFG_SYS_NUM_IN_SCO_PACKETS,          /*num_of_headers*/
#endif
        BTq_data_memory.sco_in_headers,             /*queue_headers*/
        BTq_data_memory.sco_in_free_header_list,    /*free_header_list*/
        PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH,        /*max_data_packet_length*/
        BTq_data_memory.sco_in_heaps                /*queue_heap*/ );


    BTq_Initialise_Data_Queue_Type(SCO_OUT_Q,       /*q_type*/
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)
        g_sys_config.hc_buffer_size.numScoDataPackets,
#else
        PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS,         /*num_of_headers*/
#endif
        BTq_data_memory.sco_out_headers,            /*queue_headers*/
        BTq_data_memory.sco_out_free_header_list,   /*free_header_list*/
        PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH,       /*max_data_packet_length*/
        BTq_data_memory.sco_out_heaps               /*queue_heap*/ );

#if (BUILD_TYPE!=UNIT_TEST_BUILD)
	BTq_sco_in_data_aggregation = TRUE;
#else
	BTq_sco_in_data_aggregation = FALSE;
#endif

    for(i=0; i<PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI; i++)
    {
        _BTq_Initialise_Data_Queue(BTQueues+SCO_IN_Q+i);
        _BTq_Initialise_Data_Queue(BTQueues+SCO_OUT_Q+i);
    }
#if (defined(SYS_DEBUG_INJECT_SCO_BELOW_HCI) || defined(SYS_DEBUG_INJECT_ESCO_BELOW_HCI))
	test_qd.data = temp_test_buffer;
	temp_test_buffer_ptr = 0;
	dummy_voice_txed = 0;
#endif
#endif
}
/*****************************************************************************
 * BTq_Get_L2CAP_In_Heap_Ref
 *
 * Return location of the L2CAP In Data Heap Area
 *
 * Currently unused.
 ****************************************************************************/
/*u_int8* BTq_Get_L2CAP_In_Heap_Ref(void) */
/*{ */
/*    return (u_int8*) BTq_l2cap_in.heap; */
/*} */
/*****************************************************************************
 * BTq_Is_Queue_Empty
 *
 * This function returns 1 if the queue specified is empty
 ****************************************************************************/
boolean BTq_Is_Queue_Empty(u_int8 q_type, t_deviceIndex device_index)
{
    return !(BTQueues[q_type+device_index].num_entries);
}

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER) //not used
/*****************************************************************************
 * BTq_Are_Queues_Empty
 *
 * This function returns 1 if the queue set specified is empty
 ****************************************************************************/
boolean BTq_Are_Queues_Empty(u_int8 q_type)
{
    t_queue *q  = BTQueues + q_type;
    t_queue *q_end;
        
    switch(q_type)
    {
    case HCI_COMMAND_Q:
    case HCI_EVENT_Q:
#if (PRH_BS_CFG_SYS_TRACE_VIA_HCI_SUPPORTED==1)
    case TCI_DEBUG_Q:
#endif
    case LMP_IN_Q:
        return (q->num_entries == 0);
    case LMP_OUT_Q:
    case L2CAP_IN_Q:
        q_end = q + MAX_ACTIVE_DEVICE_LINKS;
        break;
    case L2CAP_OUT_Q:
        q_end = q + MAX_ACTIVE_DEVICE_LINKS + 1 /*extra queue for piconet broadcast*/;
        break;
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    case SCO_IN_Q:
    case SCO_OUT_Q:
        q_end = q + PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI;
        break;
#endif
    }

    do
    {
        if(q->num_entries) return FALSE;
        q++;
    }
    while(q != q_end);
    
    return TRUE;
}
#endif

/*****************************************************************************
 * _BTq_Allocate_Header
 *
 * Allocate a queue descriptor for the required queue
 *
 ****************************************************************************/
t_q_descr * _BTq_Allocate_Header(
    t_data_queue *p_data_queue, t_length length)
{
    t_q_descr *q_descr;

    if( p_data_queue->next_free_pointer < p_data_queue->num_of_headers &&
        length <= p_data_queue->max_data_packet_length)
    {
        q_descr = p_data_queue->free_header_list[p_data_queue->next_free_pointer++];
    }
    else
    {
        q_descr = (t_q_descr *) NULL;
    }
    return q_descr;
}

/*****************************************************************************
 * _BTq_Deallocate_Header
 *
 * Deallocate a queue descriptor for the required queue
 ****************************************************************************/
void _BTq_Deallocate_Header(t_data_queue *p_data_queue, t_q_descr *q_descr)
{
    q_descr->data -= q_descr->data_start_offset;
    q_descr->data_start_offset = 0;
    p_data_queue->free_header_list[--p_data_queue->next_free_pointer] = q_descr;
}

/*****************************************************************************
 * BTq_Get_Queue_Number_Of_Processed_Chunks
 *
 * Get the number of processed chunks on queue.
 ****************************************************************************/
u_int16 BTq_Get_Queue_Number_Of_Processed_Chunks(
        u_int8 q_type, t_deviceIndex device_index)
{
    return BTQueues[q_type+device_index].num_processed_chunks;
}

/*****************************************************************************
 * BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks
 *
 * Get the number of processed chunks on queue.
 * Reset the number of processed chunks on queue to 0
 * Decrement the associated total of processed chunks.
 *
 * q_type           L2CAP_IN_Q L2CAP_OUT_Q SCO_IN_Q SCO_OUT_Q
 * device_index     Device or Sco index for queue
 *
 ****************************************************************************/
u_int16 BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(
        u_int8 q_type, t_deviceIndex device_index)
{
     u_int16 number_of_chunks;
     number_of_chunks = BTQueues[q_type+device_index].num_processed_chunks;
    
     /*
      * Reset Queue Number of Processed Chunks to 0
      */
     BTq_Reduce_Queue_Number_Of_Processed_Chunks(
        q_type, device_index, number_of_chunks);

    return number_of_chunks;
}

/*****************************************************************************
 * BTq_Get_Total_Number_Of_Processed_Chunks
 *
 * Get the number of processed chunks
 * For outgoing BB L2CAP queues, processed means transmitted/flushed.
 * For incoming BB L2CAP queues, processed means received into queues
 *
 * q_type           L2CAP_IN_Q L2CAP_OUT_Q SCO_IN_Q SCO_OUT_Q
 *
 ****************************************************************************/
u_int16 BTq_Get_Total_Number_Of_Processed_Chunks(u_int8 q_type)
{
    t_data_queue *p_data_queue = _BTq_Get_Data_Queue_Ref(q_type);

    return p_data_queue->num_of_consumed_chunks;
}

/*****************************************************************************
 * BTq_Reduce_Queue_Number_Of_Processed_Chunks
 *
 * Reduce the number of queue processed chunks and the associated total
 * of processed chunks for the given queue type by reduction
 *
 * q_type           L2CAP_IN_Q L2CAP_OUT_Q SCO_IN_Q SCO_OUT_Q
 * device_index     Device or Sco index for queue
 * chunk_reductions The number of processed chunks to reduce
 *
 ****************************************************************************/
void BTq_Reduce_Queue_Number_Of_Processed_Chunks(
        u_int8 q_type, t_deviceIndex device_index, u_int16 chunk_reduction)
{
   
     
    if(PATCH_FUN[BTQ_REDUCE_QUEUE_NUMBER_OF_PROCESSED_CHUNKS_ID]){
         ((void (*)(u_int8 q_type, t_deviceIndex device_index, u_int16 chunk_reduction))PATCH_FUN[BTQ_REDUCE_QUEUE_NUMBER_OF_PROCESSED_CHUNKS_ID])(q_type, device_index, chunk_reduction);
         return ;
    }


	t_data_queue *p_data_queue = _BTq_Get_Data_Queue_Ref(q_type);

    u_int16 *p_total_number_of_chunks;
    u_int16 *p_q_num_processed_chunks;
    /* 
     * CPU flags - stores current interrupt settings to enforce exclusivity 
     */
    u_int32 cpu_flags;  

    p_q_num_processed_chunks = &BTQueues[q_type+device_index].num_processed_chunks;
    p_total_number_of_chunks = &p_data_queue->num_of_consumed_chunks;

    SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags); 

	/*******************************************************************************************
	 * It is important to prevent the number of "processed_chunks" and "number_of_chunks" from
	 * wrapping. Currently this occurs in ARQ_BV32 with eSCO and Erroneous Data turned on.
	 * 
	 * However, we have to preclude wrapping AT all cost.
	 *******************************************************************************************/

	if (*p_q_num_processed_chunks)
		*p_q_num_processed_chunks -= chunk_reduction;

	if (*p_total_number_of_chunks)
	    *p_total_number_of_chunks -= chunk_reduction;

    SYSirq_Interrupts_Restore_Flags(cpu_flags);  
}


/*****************************************************************************
 * BTq_Initialise
 *
 * Initialise all the queues in the system.
 ****************************************************************************/
void BTq_Initialise(void)
{
	if(PATCH_FUN[BTQ_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[BTQ_INITIALISE_ID])();
         return ;
    }

    int q_offset;

    /* 
     * Initialise the input and output L2CAP queues and SCO queues if supported
     */
    _BTq_Initialise_All_Data_Queues();
    
    /* 
     * Initialise HCI Event Queue 
     */
    _BTq_Initialise_Variable_Chunk_Queue(BTQueues + HCI_EVENT_Q,
        hci_event_headers, PRH_BS_CFG_SYS_MAX_HCI_EVENTS,
        hci_event_heap, PRH_BS_CFG_SYS_SIZEOF_HCI_EVENT_HEAP);        
    /* 
     * Initialise HCI Command Queue 
     */
    _BTq_Initialise_Variable_Chunk_Queue(BTQueues + HCI_COMMAND_Q,
        hci_command_headers, PRH_BS_CFG_SYS_MAX_HCI_COMMANDS,
        hci_command_heap, PRH_BS_CFG_SYS_SIZEOF_HCI_COMMAND_HEAP);   
    
    /* 
     * Initialise the shared LMP incoming Queue 
     */
    _BTq_Initialise_Variable_Chunk_Queue(BTQueues + LMP_IN_Q, 
        lmp_in_headers, PRH_BS_CFG_SYS_MAX_LMP_IN_MESSAGES,
        lmp_in_heap, PRH_BS_CFG_SYS_SIZEOF_LMP_IN_HEAP);        
    /* 
     * Initialise the output LMP queues 
     */
    for(q_offset=0; q_offset < MAX_ACTIVE_DEVICE_LINKS; q_offset++)
    {
        _BTq_Initialise_Variable_Chunk_Queue(BTQueues + LMP_OUT_Q + q_offset, 
            lmp_out_headers[q_offset], PRH_BS_CFG_SYS_MAX_LMP_OUT_MESSAGES, 
            lmp_out_heaps[q_offset], PRH_BS_CFG_SYS_SIZEOF_LMP_OUT_HEAP);        
    }
    

#if (PRH_BS_CFG_SYS_TRACE_VIA_HCI_SUPPORTED==1)
    /* 
     * Initialise TCI Debug Queue 
     */
    _BTq_Initialise_Variable_Chunk_Queue(BTQueues + TCI_DEBUG_Q,
        hci_debug_headers, PRH_BS_CFG_SYS_MAX_HCI_DEBUG_EVENTS,
        hci_debug_heap, PRH_BS_CFG_SYS_SIZEOF_HCI_DEBUG_HEAP);        
#endif
}

/*****************************************************************************
 * BTq_Reset
 *
 * Reset the queues associated with device index
 * Warning: This function should be executed atomically with 
 * respect to any other contexts that might try to enqueue or dequeue 
 * to/from this particular queue.
 *
 * Reset the LMP output Queue
 * Reset the L2CAP input and output queues after returning any entries
 *       and resetting the number of processed chunks
 * Note that LMP input queue is not reset, since shared queue.
 ****************************************************************************/
void BTq_Reset(t_deviceIndex device_index)
{   
	
	if(PATCH_FUN[BTQ_RESET_ID]){
         ((void (*)(t_deviceIndex device_index))PATCH_FUN[BTQ_RESET_ID])(device_index);
         return ;
    }

    t_queue *q;
    t_q_descr *cursor;
 
    _BTq_Initialise_Variable_Chunk_Queue(BTQueues + LMP_OUT_Q + device_index, 
        lmp_out_headers[device_index], PRH_BS_CFG_SYS_MAX_LMP_OUT_MESSAGES, 
        lmp_out_heaps[device_index], PRH_BS_CFG_SYS_SIZEOF_LMP_OUT_HEAP);

    /*
     * Reset the Baseband incoming L2CAP Queue
     * In the case of L2cap, its allocations being dynamic in nature, all of 
     * the allocated chunks and headers must be returned to the respective  
     * global list before the initialisation function can be called again
     */
    BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(L2CAP_IN_Q, device_index);

    /* 
     * Need to deallocate anything allocated - order is not important 
     */
    q = mBTq_Get_Q_Ref(L2CAP_IN_Q, device_index); 
    cursor = q->headers;
    while(cursor)
    {
        t_q_descr *cursor_next;
        cursor_next = cursor->next;
        _BTq_Deallocate_Header(&BTq_l2cap_in, cursor);
        cursor = cursor_next;
    }
    _BTq_Initialise_Data_Queue(q);

    /* 
     * Reset the Baseband outgoing L2CAP Queue
     */
    BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(L2CAP_OUT_Q, device_index);

    /* 
     * Need to deallocate anything allocated - order is not important 
     */
    q = mBTq_Get_Q_Ref(L2CAP_OUT_Q, device_index); 
    cursor = q->headers;
    while(cursor)
    {
        t_q_descr *cursor_next;
        cursor_next = cursor->next;
        _BTq_Deallocate_Header(&BTq_l2cap_out, cursor);
        cursor = cursor_next;
    }
    _BTq_Initialise_Data_Queue(q);
}
/*************************************************************************** 
 * BTq_Enqueue
 *
 * Returns an empty queue descriptor if one is available.
 * NOTE: some of the fields may have residual garbage.
 * It is guaranteed that the length field will be correct
 * and the callback field set to 0x0
 ****************************************************************************/
t_q_descr *BTq_Enqueue(u_int8 q_type, t_deviceIndex device_index, t_length length)
{
	if(PATCH_FUN[BTQ_ENQUEUE_ID]){
         
         return ((t_q_descr *(*)(u_int8 q_type, t_deviceIndex device_index, t_length length))PATCH_FUN[BTQ_ENQUEUE_ID])(q_type, device_index, length);
    }

	
    t_data_queue  *p_data_queue = _BTq_Get_Data_Queue_Ref(q_type);
    t_queue       *q;
    t_q_descr     *qd = 0x0;
    u_int32       cpu_flags;

    /* 
     * ACL/L2cap or SCO queuing is a fixed size size chunk system 
     */
    if(p_data_queue)
    {
        q = BTQueues + q_type + device_index;

        /*
         * This block must be atomic to prevent the ACK function corrupting
         * the linked list pointers through pre-emption
         */
        SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);    
            
		/*
		 *	Bug2896 - If there is already a pending commit on this queue, return zero
		 *  and send a buffer over flow event to the host.
		 */
		if (q->pending_commits != 0)
		{
			t_lm_event_info event_info;
            SYSirq_Interrupts_Restore_Flags(cpu_flags);    

			if (q_type > L2CAP_OUT_Q)
				event_info.link_type = SCO_LINK;
			else 
				event_info.link_type = ACL_LINK;

			HCeg_Generate_Event(HCI_DATA_BUFFER_OVERFLOW_EVENT, &event_info);
			return (t_q_descr *) NULL;
		}
       
#if (TRA_QUEUE_SYN_AGGREGATION_ON_RECEIVE==1)
		if (BTq_sco_in_data_aggregation)
		{
			if (q_type == SCO_IN_Q && q->num_entries && q->headers_tail)
			{ /* if elements already on queue */
				qd = q->headers_tail;
				if(length <= (p_data_queue->max_data_packet_length - qd->data_start_offset - qd->length))
				{ /* if last enqueued buffer still has sufficient room */
					qd->data += qd->length;
					qd->data_start_offset += qd->length;
					qd->length = length;
					q->pending_commits++;
					q->num_entries--;
					SYSirq_Interrupts_Restore_Flags(cpu_flags);
					return qd;
				}
			}
		}
#endif

        /* 
         * Firstly, get a free header - this is the most efficient way to do 
         * this without sparse lookup tables 
         */
        qd = _BTq_Allocate_Header(p_data_queue, length);

        if(qd)
        {
            /*
             * Insert the descriptor into the single linked list of descriptors.
             */
            if(!q->headers)                        /* Then it's the first entry */
            {    
                q->headers = qd;
            }
            else
            {
                q->headers_tail->next = qd;
            }    

            q->headers_tail = qd;
            q->pending_commits++;    
            SYSirq_Interrupts_Restore_Flags(cpu_flags);    
        
            qd->next = 0x0;    
            
#if (TRA_QUEUE_SYN_AGGREGATION_ON_RECEIVE==1)
            if ((q_type == SCO_IN_Q) && (BTq_sco_in_data_aggregation))
            {
            	qd->insert_time = LC_Get_Native_Clock(); /* enqueue time of first segment */
            }
#endif

            /* 
             * Normal setting (exception and overriden if rx broadcast packet)
             */
            qd->device_index = device_index;    
            qd->broadcast_flags = 0x0; 
            qd->callback = 0x0;
            qd->length = length;
			qd->message_type = 0x00;
			qd->data_start_offset = 0;
			qd->encrypt_status = LE_DATA_NOT_ENCRYPTED;
        }
        else
        {
            SYSirq_Interrupts_Restore_Flags(cpu_flags);    
        }
        return qd;
    }


    /* 
     * Other queues use a variable length chunk system 
     */
    else    
    {
        /*
         * To enqueue, check if what the current tail points to is free
         * if so, try to get some memory from the chunk
         */

        q = mBTq_Get_Q_Ref(q_type, device_index);		
		/*
		 *	Bug2896 - If there is already a pending commit on this queue, return zero
		 *  and send a buffer over flow event to the host.
		 */
		if (q->pending_commits != 0)
		{
			t_lm_event_info event_info;

			event_info.link_type = ACL_LINK;

			if (q_type != HCI_EVENT_Q)
				HCeg_Generate_Event(HCI_DATA_BUFFER_OVERFLOW_EVENT, &event_info);

			return (t_q_descr *) NULL;
		}

        qd = &q->headers[q->desc_tail];

        if(!qd->data)                      
        {
            /* 
             * If qd->data pointer is NULL then it's Free
             * For efficiency q_data member of t_q_descr should come first.
             * If the _pointer_ is 0x0 then it's free (NOT what pointer points to!)
             *
             * The callback is set to 0x0 by default so a user function can check 
             * the value of the function ptr to determine if associated callback
             */ 
			qd->encrypt_status = LE_DATA_NOT_ENCRYPTED;
            qd->length = length;
            qd->callback = 0x0;            
            qd->device_index = device_index;
            qd->broadcast_flags = 0x0; 
            length = (length+3)&~0x3;        /* Round up to multiple of 4 */


            if(!q->num_entries)
            {
                if(q->data_size >= (s_int16)length)
                {
                    SYSatomic_Increment_u_int8(&q->pending_commits);    
                    qd->data = q->q_data;
                    q->tail_offset += length;
                    goto out;    
                }
            }
        
            if((q->head_offset - q->tail_offset) >= (s_int16)length)
            {
                SYSatomic_Increment_u_int8(&q->pending_commits);    
                qd->data = q->q_data + q->tail_offset;    
                q->tail_offset += length;
                goto out;
            }
            if(q->tail_offset > q->head_offset)
            {
                if((q->data_size - q->tail_offset) >= (s_int16)length)
                {
                    SYSatomic_Increment_u_int8(&q->pending_commits);    
                    qd->data = q->q_data + q->tail_offset;    
                    q->tail_offset += length;
                    goto out;    
                }
                if(q->head_offset >= (s_int16)length)
                {
                    SYSatomic_Increment_u_int8(&q->pending_commits);    
                    qd->data = q->q_data;
                    q->end_offset = q->tail_offset;
                    q->tail_offset = length;        /* Wrap the tail */
                    goto out;    
                }
            }
        }
    }
    return (t_q_descr*)0x0;    

out:

    if (++q->desc_tail == q->num_headers)
    {
        q->desc_tail = 0;
    }
    return qd;                          /* Return the descriptor I just filled */
}    

/*****************************************************************************
 * BTq_Commit
 *
 * This function commits an entry onto a queue by atomically increasing its
 * entry count.
 * Only after this function is executed will the entry become visible on the 
 * queue. This is to prevent another source attempting to remove the entry 
 * before its contents have been fully written.
 ****************************************************************************/
void BTq_Commit(u_int8 q_type, t_deviceIndex device_index)
{
	
	 
    if(PATCH_FUN[BTQ_COMMIT_ID]){
         ((void (*)(u_int8 q_type, t_deviceIndex device_index))PATCH_FUN[BTQ_COMMIT_ID])(q_type,   device_index);
         return ;
    }

    t_queue *q;
    u_int32 cpu_flags;

    q = mBTq_Get_Q_Ref(q_type, device_index);

    SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
    q->pending_commits--;
    q->num_entries++;
#ifndef BLUETOOTH_MODE_LE_ONLY
	if (q_type == SCO_OUT_Q)
		q->total_data_enqueued += q->headers_tail->length;
#endif

    SYSirq_Interrupts_Restore_Flags(cpu_flags);

#ifndef BLUETOOTH_MODE_LE_ONLY
#if (TRA_QUEUE_SYN_AGGREGATION_ON_RECEIVE==1)
    if ((q_type == SCO_IN_Q) && (BTq_sco_in_data_aggregation))
    { // aggregate a packet_status_flag representation
    	t_q_descr     *qd = q->headers_tail;
    	//if (qd->broadcast_flags) // previous bit-field values - so aggregate // TK 07.01.2013 fix for BQB
    	{
    		qd->broadcast_flags |= 1<<(qd->message_type); // apply as a bit-field

    		if (qd->broadcast_flags & (1<<SYNC_DATA_LOST))
    			//qd->message_type = (qd->broadcast_flags > SYNC_DATA_LOST)?SYNC_DATA_PARTIALLY_LOST:SYNC_DATA_LOST;
				qd->message_type = (qd->broadcast_flags > (1<<SYNC_DATA_LOST))?SYNC_DATA_PARTIALLY_LOST:SYNC_DATA_LOST; // TK 07.01.2013 fix for BQB
    		else if (qd->broadcast_flags & (1<<SYNC_DATA_WITH_ERRORS))
    			qd->message_type = SYNC_DATA_WITH_ERRORS;
    	}
    }
#endif
#endif

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
    switch(q_type)
    {
    case LMP_IN_Q:
        BTms_OS_Post(BTMS_OS_EVENT_LMP_IN_QUEUE);
        break;
    case L2CAP_IN_Q:
        BTms_OS_Post(BTMS_OS_EVENT_L2CAP_IN_QUEUE);
        break;
    case L2CAP_OUT_Q:
    #if 0
        if(BTtst_Read_Loopback_Mode()==LOCAL_LOOPBACK)
            BTms_OS_Post(BTMS_OS_EVENT_L2CAP_OUT_QUEUE);
    #endif        
        break;
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    case SCO_IN_Q:
	        BTms_OS_Post(BTMS_OS_EVENT_SCO_IN_QUEUE);
        break;    
    case SCO_OUT_Q:
        if(BTtst_Read_Loopback_Mode()==LOCAL_LOOPBACK)
            BTms_OS_Post(BTMS_OS_EVENT_SCO_OUT_QUEUE);
        break;
#endif
    case HCI_COMMAND_Q:
        BTms_OS_Post(BTMS_OS_EVENT_HCI_COMMAND_QUEUE);
        break;
    }
#endif
}

/*****************************************************************************
 * BTq_Dequeue_Next
 *
 * Dequeue an entry from the selected queue
 *
 * This function is used to dequeue a full entry from the queue without
 * any segmentation.
 *
 * Note:  
 *    B1893: No longer used for remote loopback, hence remove check for 
 *    BTq_Change_Endianness_If_Required() 
 ****************************************************************************/
t_q_descr *BTq_Dequeue_Next(u_int8 q_type, t_deviceIndex device_index)
{
	
	if(PATCH_FUN[BTQ_DEQUEUE_NEXT_ID]){
         
         return ((t_q_descr *(*)(u_int8 q_type, t_deviceIndex device_index))PATCH_FUN[BTQ_DEQUEUE_NEXT_ID])(q_type,  device_index);
    }

    t_data_queue *p_data_queue = _BTq_Get_Data_Queue_Ref(q_type);
    t_queue *q;

    q = mBTq_Get_Q_Ref(q_type, device_index);

    /*
     * If entries on queue Then
     *    Return pointer to first entry
     * Endif
     */
    if(q->num_entries)
    {
        q->original_length = 0;
        if(p_data_queue)
        {
#if (PRH_BS_DEV_BIG_ENDIAN_AND_DIRECT_BASEBAND_MOVE==1)
            /*
             * If Direct Baseband Copy (Big Endian) Then 
             *    If Dequeueing Input ACL Data and not in Remote Loopback Then
             *        Change Data to Little Endian
             *        [The data was directly copied from baseband (in big_endian)]
             */
            if (q_type == L2CAP_IN_Q && q->headers /*&& 
                BTtst_Read_Loopback_Mode() != REMOTE_LOOPBACK */)
            {
                BTq_Change_Endianness_If_Required(q->headers->data, q->headers->length);
            }
#endif
            return q->headers;    
        }
        else 
        {
            u_int32 cpu_flags;
            /* 
             * Do the get read ptr here - we need to protect this section as BTq_Enqueue 
             * could pre-empt it for this index, and fail, due to the wrap not occurring, 
             * or pre-empting between altering head_offset and end_offset
             */
            SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
            if(q->head_offset == q->end_offset)
            {
                q->head_offset = 0;
                q->end_offset = q->data_size;
            }
            SYSirq_Interrupts_Restore_Flags(cpu_flags);

            return &q->headers[q->desc_head];    
        }
    }
    return (t_q_descr *)0x0;
}

/*****************************************************************************
 * BTq_LE_Dequeue_Next
 *
 * Special function for baseband dequeue: Additional argument is a bitmap of
 * packet types.
 * This function will select the most appropriate depending on the types
 * specified and the state of the queue in question.
 *
 * q_type           LMP_OUT_Q   L2CAP_OUT_Q    SCO_OUT_Q
 * device_index     All baseband queues have an associated device_index
 * in_packet_types  Packet types to be used for tx
 ****************************************************************************/
t_q_descr *BTq_LE_Dequeue_Next(u_int8 q_type, t_deviceIndex device_index,
                                   u_int16 length)
{
	
	if(PATCH_FUN[BTQ_LE_DEQUEUE_NEXT_ID]){
         
         return ((t_q_descr *(*)(u_int8 q_type, t_deviceIndex device_index,u_int16 length))PATCH_FUN[BTQ_LE_DEQUEUE_NEXT_ID])(q_type,   device_index,length);
    }

    t_queue *q;
    t_q_descr *qd;

   // u_int16 packet_type_length;
    u_int32 cpu_flags;

    q = BTQueues + q_type + device_index;

    if(q->num_entries)            /* First check what should be done.. */
    {
        if(q_type == LMP_OUT_Q)
        {
            qd = &q->headers[q->desc_head];

            /*
             * If the head has reached the end, it is not read from it now,
             * Must first wrap and then read from it
             */
            SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
            if(q->head_offset == q->end_offset)
            {
                q->head_offset = 0;
                q->end_offset = q->data_size;
            }
            SYSirq_Interrupts_Restore_Flags(cpu_flags);
#if 1 // Not Needed for LE
            q->original_length = 0;
#endif
            return qd;
        }

        /*
         * Every thing here relates to L2CAP/ACL or SCO data only
         */

        /*
         * Take the descriptor from head of linked list
         */
        qd = q->headers;

        /*
         * Verify if the context is a re-transmission and undo this if
         * we re-enter due to a retransmission
         */
#if 1 // segmentation packet for BLE.
/*        if(q->rollback_length)
        {
            qd->length += q->rollback_length;
        }
*/
        /*
         * Determine if segmentation required
         * (If what's on the current queue descriptor is longer than the packet type)
         */
        if((qd->length > length))
        {
            u_int16 neat_packet_type_length;
            /*
             * Align to 32 bit word for fast copy
             */
            neat_packet_type_length = length & (~0x3);

            /*
             * Save original pre-segmented length
             * Change qd->length to the rounded down length
             * Increment the number of entries, as we've created a new one
             */
            q->original_length = qd->length;
            /*q->rollback_length = q->original_length - neat_packet_type_length;*/
            qd->length = neat_packet_type_length;
            return qd;
        }
#endif

        /*
         * At this point , then the qd->length must be equal to the
         * packet_type_length, so just return this descriptor
         */
#if 1 // Not Needed for LE
        q->original_length = 0;
#endif
        return qd;
    }

    return (t_q_descr *)0x0;
}

/*****************************************************************************
 * BTq_Dequeue_Next_Data_By_Length
 *
 * Special function for HC to HOST Data dequeuing. 
 * Additional argument a maximum length to segment to, or if supported,
 * to reassemble to.
 *
 * q_type               Normally L2CAP_IN_Q
 * device_index         Associated device index for the q_type
 * max_packet_length    If not 0 Then Segment/[Reassemble] to presented length
 *
 * This function will never roll-back to start of packet
 ****************************************************************************/
t_q_descr *BTq_Dequeue_Next_Data_By_Length(u_int8 q_type, t_deviceIndex device_index, u_int16 max_packet_length)
{
	if(PATCH_FUN[BTQ_DEQUEUE_NEXT_DATA_BY_LENGTH_ID]){
         
         return ((t_q_descr *(*)(u_int8 q_type, t_deviceIndex device_index, u_int16 max_packet_length))PATCH_FUN[BTQ_DEQUEUE_NEXT_DATA_BY_LENGTH_ID])(q_type,device_index,max_packet_length);
    }
    t_queue   *q;
    t_q_descr *qd;

    q = BTQueues + q_type + device_index;

    /*
     * If there are entries on the queue Then
     *    Extract an appropriate queue descriptor
     * Else
     *    Return an empty descriptor
     * Endif
     */
    if(q->num_entries)
    {    
        qd = q->headers;        

#if (PRH_BS_DEV_BIG_ENDIAN_AND_DIRECT_BASEBAND_MOVE==1)
        /*
         * If Direct Baseband Copy (Big Endian) and Not Remote Loopback Then 
         *    If First Segment of ACL packet Then
         *         Change Data to Little Endian 
         *
         * Note must be done before qd->length is adjusted during segmentation
         * SCO endianess is resolved directly on baseband copies
         */
        if (q_type == L2CAP_IN_Q && qd->data_start_offset == 0 &&
                BTtst_Read_Loopback_Mode() != REMOTE_LOOPBACK)
        {
            BTq_Change_Endianness_If_Required(qd->data, qd->length);
        }
#endif

#if (TRA_QUEUE_SYN_AGGREGATION_ON_RECEIVE==1)
        if ((q_type == SCO_IN_Q) && (BTq_sco_in_data_aggregation))
        {
        	if ((q->num_entries == 1) && !BTtimer_Is_Expired(qd->insert_time + TRA_QUEUE_SYN_MAX_LATENCY)
        		&& (qd->data_start_offset + ((qd->length)<<1) <= BTq_sco_in.max_data_packet_length))
        	{ /* still space & time for further aggregation on only enqueued buffer */
                return 0;
        	}
        	else if(qd->data_start_offset)
        	{ /* if contains multiple segments adjust data pointer to start */
				qd->broadcast_flags = 0; // TK 07/01/2013 fix for BQB - reserved field - most be set to 0.
        		qd->data -= qd->data_start_offset;
        		qd->length += qd->data_start_offset;
        		qd->data_start_offset = 0;
        	}
        }
#endif

        /* 
         * If current entry descriptor length is longer than requested 
         *    and the max_packet_length is non 0  Then
         *     Segment the descriptor
         * Endif
         */    
        if((qd->length > max_packet_length) && (max_packet_length > 0))     
        {
            /*
             * Save original pre-segmented length
             * Change qd->length to the rounded down length
             * Increment the number of entries, as we've created a new one
             */
            q->original_length = qd->length;             
            q->rollback_length = q->original_length - max_packet_length;  
            qd->length = max_packet_length;        
        }
        else
        {
            /* 
             * At this point , then the qd->length must be equal to the 
             * packet_type_length, so just return this descriptor 
             */    
            q->original_length = 0;         
        }
    }
    else
    {
        /*
         * Queue is empty, nothing to transmit.
         */
        qd = (t_q_descr *)0;
    }

    return qd;
}

/*****************************************************************************
 * BTq_Ack_Last_Dequeued
 *
 * Acknowledge dequeue entry, removing entry from queue.
 *
 * q_type               All queue types
 * device_index         Associated device index for the q_type
 * length               Length of entry being acknowledged (historical)
 *
 ****************************************************************************/
void BTq_Ack_Last_Dequeued(u_int8 q_type, t_deviceIndex device_index, t_length length)
{
	  if(PATCH_FUN[BTQ_ACK_LAST_DEQUEUED_ID]){
         ((void (*)(u_int8 q_type, t_deviceIndex device_index, t_length length))PATCH_FUN[BTQ_ACK_LAST_DEQUEUED_ID])(q_type,   device_index,   length);
         return ;
    }
    t_data_queue  *p_data_queue = _BTq_Get_Data_Queue_Ref(q_type);
    t_queue   *q;
    t_q_descr *qd=0;
    u_int32 cpu_flags;

    q = mBTq_Get_Q_Ref(q_type, device_index);

#ifdef SYS_DEBUG_INJECT_SCO_BELOW_HCI
	if ((q_type == SCO_OUT_Q) && (dummy_voice_txed==1))
	{
		dummy_voice_txed=0;
		return;
	}
#endif

    if((q->num_entries == 0) && (HCI_COMMAND_Q !=q_type))
    {
        HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_CORRUPTION_OF_QUEUES);
        return;
    }
    
    q->rollback_length = 0;
#ifndef BLUETOOTH_MODE_LE_ONLY
	if (q_type == SCO_OUT_Q)
	{
		SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
		q->total_data_enqueued -= length;
		SYSirq_Interrupts_Restore_Flags(cpu_flags);
	}
#endif       
    if(length < q->original_length)                    /* Then it was segmented */            
    {
        if (p_data_queue)
        {
            qd = q->headers;

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
            if (q_type == L2CAP_IN_Q || q_type == SCO_IN_Q)
#else
            if (q_type == L2CAP_IN_Q)  
#endif
            {
                /*
                 * The following code block tracks how many "chunks" have been
                 * processed/sent by the HC to Host but not acknowledged 
                 * (i.e. delivered to the host).
                 * Segmentation is as a result of Host Buffer Sizes being
                 * smaller than received packet
                 */
                 q->num_processed_chunks++; 
                 p_data_queue->num_of_consumed_chunks++;
            }
        }

        /*
         * Set message type to reflect the sub-segment
         * Set new length to remainder
         * Set data pointer to start of remainder of data
         */
        qd->message_type = LCH_continue;            
        qd->length = q->original_length - length;    
        qd->data += length;                            
        /*
         * Record how far the data pointer is from start of segment
         */
        qd->data_start_offset += length;
    }
    else                                    
    {
        /*
         * Hasn't been segmented, has it been aggregated ?
         */
        if (p_data_queue)
        {
            SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
    
            qd = q->headers;
            q->headers = qd->next;

            if (q->headers==0)
            {
                q->headers_tail = 0x0;
            }

            _BTq_Deallocate_Header(p_data_queue, qd);
            /*
             * The following code block tracks how many "chunks" have been
             * consumed (i.e. delivered successfully by the link controller.
             * Zero length packets must also be tracked.
             */
            p_data_queue->num_of_consumed_chunks++;                  
            q->num_processed_chunks++; 

            q->num_entries--;
            SYSirq_Interrupts_Restore_Flags(cpu_flags);
        }    
        else /* Variable queue */
        {
           SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);    

            /*
             * Lengths always aligned to 4 byte chunks
             */
            length = (length+3)&~0x3;        
            qd = &q->headers[q->desc_head]; 
            q->head_offset += length;
            if (++q->desc_head == q->num_headers)
            {
                q->desc_head = 0;
            }

            /* 
             * Zero the data pointer to indicate free block 
             */
            qd->data = 0x0;

            /* 
             * Decrement number of entries and reset head/tail to zero if no entries 
             */
            q->num_entries--;
            if(!q->num_entries && !q->pending_commits)
            {
                q->head_offset = q->tail_offset = 0;
                q->end_offset = q->data_size;
            }
            SYSirq_Interrupts_Restore_Flags(cpu_flags);
        }

        /* 
         * If there is a callback set, call it since packet now complete. 
         */    
        if(qd->callback)
        {
            qd->callback(qd->device_index);
        }
    }
}


#if (PRH_BS_DEV_BIG_ENDIAN_AND_DIRECT_BASEBAND_MOVE==1)
#include "hw_macro_defs.h"              /* Default SWAP_ENDIAN32 macro */
/*****************************************************************************
 * BTq_Change_Endianness_If_Required
 *
 * Convert data in-situ from little to big endian or vice versa
 *
 * Expected usage is for big endian systems where the data is converted
 * before enqueue or after dequeue (and not in baseband).
 ****************************************************************************/
void BTq_Change_Endianness_If_Required(u_int8 *p_data, u_int16 length)
{
    size_t  num_word32 = ((length + 3) >> 2);
    u_int32 *p_word32 = (u_int32*) p_data;
    u_int32 word32;

    while (num_word32-- > 0)
    {
        word32 = *p_word32;
        *p_word32++ = SWAP_ENDIAN32(word32);
    }                   
}
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)  //not used
/*****************************************************************************
 *
 * BTq_BB_Dequeue_Next_Synchronous
 *
 * Special function for baseband dequeue of eSCO data.
 *
 ****************************************************************************/
#if (1==PRH_BS_DBG_SOURCE_SINK_SYN_TEST_DATA)
#include <string.h>
u_int8 test_buffer[4][362] = 
{    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00 },
    { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
      0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 }   };

u_int32 test_buffer_counter[4] = { 0, 0, 0, 0 };
#endif

u_int16 BTq_BB_Dequeue_Next_Synchronous(u_int8 q_type, u_int8 sco_index, 
                                   u_int16 packet_length, u_int8 *buffer)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
#if (1==PRH_BS_DBG_SOURCE_SINK_SYN_TEST_DATA)
    _Insert_Uint32(&(test_buffer[sco_index][0]), test_buffer_counter[sco_index]);
    memcpy(buffer, test_buffer[sco_index], packet_length);
    test_buffer_counter[sco_index]++;
    return packet_length;
#else
    t_q_descr *p_qD;
	boolean finished = FALSE;
	u_int16 dequeue_length = packet_length;
	
	if (BTq_Get_Total_Bytes_Enqueued(SCO_OUT_Q, sco_index) >= dequeue_length)
	{
		
		while (!finished)
		{
			p_qD = BTq_Dequeue_Next_Data_By_Length(SCO_OUT_Q, sco_index, dequeue_length);
			
			if(p_qD && p_qD->data)
			{
			/*
			* Most of the hw_memcpy functions are for use with the
			* Tabasco hw. These functions "know" the endianness
			* of Tabasco and of the source and "swap" endianness
			* if they believe source and dest are different
			* endianness. The hw_memcpy8 function won't do
			* any swapping (in this case (copying from queues
			* to a buffer, there is no swap).
				*/
				buffer = hw_memcpy8(buffer, (void *)p_qD->data, p_qD->length);
				//BTq_Ack_Last_Dequeued(SCO_OUT_Q, sco_index, p_qD->length);  //-  delete here! Important!!
				
				if (p_qD->length < dequeue_length)
				{
				/*
				* Need to dequeue more data
				*/
					dequeue_length -= p_qD->length;
					buffer += p_qD->length;
				}
				else
				{
					finished = TRUE;
				}

				BTq_Ack_Last_Dequeued(SCO_OUT_Q, sco_index, p_qD->length);  //-  Add here! Important!!
			}
            // the else seems redundant ! consider removing
			else
			{
				finished = TRUE;
				packet_length=0;
			}
		}
    	return packet_length;
	}
	else
	{
#ifdef  SYS_DEBUG_INJECT_ESCO_BELOW_HCI
		// GF BQB 30 March -- Check for Tester Bugs, ARQ_BV27 etc..

		if ((BTtst_Get_DUT_Mode() < DUT_ENABLED) && (packet_length < 160))
		{
			buffer = hw_memcpy8(buffer, &temp_test_buffer[temp_test_buffer_ptr], packet_length);
			temp_test_buffer_ptr = (temp_test_buffer_ptr + packet_length) % SYS_DEBUG_SCO_TEST_VECTOR_REPEAT_PERIOD;
			return packet_length;
		}
		else
			return 0;
#endif

		return 0;
	}
#endif
#endif
}
#endif
/*****************************************************************************
 * BTq_Get_Queue_Number_Of_Entries
 *
 * Get the number of entries on queue.
 ****************************************************************************/
u_int16 BTq_Get_Queue_Number_Of_Entries(
        u_int8 q_type, t_deviceIndex device_index)
{
    return BTQueues[q_type+device_index].num_entries;
}

