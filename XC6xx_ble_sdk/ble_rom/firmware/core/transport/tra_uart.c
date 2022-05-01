/*****************************************************************************
 * MODULE NAME:  tra_uart
 * PROJECT CODE: BlueStream
 * DESCRIPTION:  uart physical layer transport module.
 * AUTHOR:       John Sheehy
 * DATE:         22/11/2000
 * 
 * SOURCE CONTROL: $Id: tra_uart.c,v 1.41 2014/02/21 15:38:12 tomk Exp $
 *
 * LICENCE:    Copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *
 * This contains the tx routines and rx state machine for uart transport
 * Because of the interrupt driven nature of the UARTs, this is done character 
 * by character with the end of transmission interrupt being used to signal
 * the transmission of the next one. 
 * Due to different platforms having different UARTs, this layer is an 
 * abstraction of the PDU transmission and reception/decoding to/from the host.
 * This allows character reception from various different UART drivers to be 
 * handled in the same way without duplication of code. 
 * Regarding block transmission, this layer takes advantage of FIFOs on the 
 * individual UARTs, e.g. on the ARM AEB-1 board, there is only a limited 
 * 1 byte FIFO.
 *
 * This module can be replaced by a USB equivalent.
 ****************************************************************************/
#include "sys_config.h"

#if (BUILD_TYPE!=GENERIC_BUILD)

#include "tra_queue.h"    
#include "tra_hcit.h"
#include "tra_uart.h"
#include "hc_event_gen.h"

#if defined(TRA_HCIT_UART_DIRECT_RX_READ)
#include "hcit_hal_serial.h"
#endif

#ifndef __USE_INLINES__
#include "tra_uart_impl.h"
#endif

struct tra_hcit_uart_info sys_hcit_uart_info;

/*
 * Transmit state machine variables
 */

/*
 * Transmit variables associated with the actual data being transmitted
 *     tra_hcit_uart_tx_pdu_buf         Pointer to the buffer being transmitted
 *     tra_hcit_uart_tx_pdu_length      Remaining Length of PDU being transmitted
 *     tra_hcit_uart_tx_original_length Original PDU length for Queue acknowledge
 */
static volatile u_int8  *tra_hcit_uart_tx_pdu_buf;
static volatile u_int32 tra_hcit_uart_tx_pdu_length;  
static u_int32 tra_hcit_uart_tx_original_length;    

/* 
 * Variables used while transmitting characters (used both for header and PDU) 
 *      tra_hcit_uart_cur_tx_pointer    Pointer to the buffer being transmitted
 *      tra_hcit_uart_cur_tx_length     Remaining Length of buffer (header/PDU)
 */
static volatile u_int8  *tra_hcit_uart_cur_tx_pointer; 
static volatile u_int32 tra_hcit_uart_cur_tx_length;         

/*
 * The state variable for the transmit state machine and the
 * associated transmit states used whilst encoding a transmit PDU to the host
 */
enum
{
    TRA_HCIT_UART_STATE_TX_HEADER,
    TRA_HCIT_UART_STATE_TX_BODY
};
static u_int8 tra_hcit_uart_tx_state;


/*
 * Receive specific state machine variables 
 * 
 * 4 byte array to store the HCI command/ACL/SCO header,
 * This header is stripped off in the tra_hcit.c module
 * rather than the main hci control logic
 */

static u_int32 tra_hcit_uart_rx_head_buf_array[1];
static u_int8* tra_hcit_uart_rx_head_buf = (u_int8*)tra_hcit_uart_rx_head_buf_array;

/* 
 * Receive variables associated with the actual data being received
 * tra_hcit_uart_rx_pdu_buf             Pointer to write received characters to
 * tra_hcit_uart_rx_pdu_length          Current Length of received PDU
 * tra_hcit_uart_rx_head_length         Header received length
 * tra_hcit_uart_rx_type                PDU Type: HCI command|ACL Data|SCO Data
 */
static volatile u_int8* tra_hcit_uart_rx_pdu_buf;               
static volatile u_int16 tra_hcit_uart_rx_pdu_length;
static u_int8 tra_hcit_uart_rx_head_length;
static u_int8 tra_hcit_uart_rx_type;         
static u_int8 tra_hcit_uart_hardware_error_event_flag;
/*
 * The state variable for the receive state machine and the
 * associated states used whilst decoding a received PDU from the host
 */
enum
{
    TRA_HCIT_UART_STATE_RX_TYPE,
    TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE1,
    TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE2,
    TRA_HCIT_UART_STATE_RX_COMMAND_LENGTH,
    TRA_HCIT_UART_STATE_RX_ACL_HEAD1,
    TRA_HCIT_UART_STATE_RX_ACL_HEAD2,
    TRA_HCIT_UART_STATE_RX_ACL_LENGTH1,
    TRA_HCIT_UART_STATE_RX_ACL_LENGTH2,
    TRA_HCIT_UART_STATE_RX_SCO_HEAD1,
    TRA_HCIT_UART_STATE_RX_SCO_HEAD2,
    TRA_HCIT_UART_STATE_RX_SCO_LENGTH,
    TRA_HCIT_UART_STATE_RX_DATA_START,
    TRA_HCIT_UART_STATE_RX_DATA_CONT,
    TRA_HCIT_UART_STATE_RX_DATA_COMMIT,
    TRA_HCIT_UART_STATE_RX_EVENT_OPCODE,
    TRA_HCIT_UART_STATE_RX_EVENT_LENGTH,
    TRA_HCIT_UART_STATE_RX_DATA_WHEN_NO_BUFFER
};
static u_int8 tra_hcit_uart_rx_state;

static boolean tra_hcit_uart_rx_activity = FALSE;

/*****************************************************************************
 * Function: TRAhcit_UART_Initialise
 * Parameters: 
 *
 * Returns: 
 * 
 * This function initialises the UART sublayer of the HCI Transport subsystem.
 * Pointers are assigned etc..
 ****************************************************************************/
void TRAhcit_UART_Initialise(void)
{
    /*
     * Initialise layer variables
     */
    tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_TYPE;
    
    tra_hcit_uart_rx_activity = FALSE;
    
    if(TRAhcit_UART_Get_Default_Baud_Rate()==0)
        TRAhcit_UART_Set_Default_Baud_Rate(TRA_HCIT_UART_DEFAULT_BAUD_RATE);

    sys_hcit_uart_info.initialise(TRAhcit_UART_Get_Default_Baud_Rate());

    /*
     * Clear the busy flag
     */
    sys_hcit_uart_info.common.tx_busy = 0;
}

/*****************************************************************************
 * Function: TRAhcit_UART_Shutdown
 *
 * It is guaranteed that the function pointer sys_hcit_uart_info.shutdown
 * is valid, as the parent generic HCI Transport layer
 * will not allow this function to be called if
 * the transport layer was not active (and hence this pointer valid)
 *
 ****************************************************************************/
void TRAhcit_UART_Shutdown(void)
{
    sys_hcit_uart_info.shutdown();
}



/*****************************************************************************
 * Function: TRAhcit_UART_Tx_Block
 * Parameters: 
 *      data  :         pointer to the data 
 *      length:         length of data
 *      queue :         queue ID to commit to (e.g. HCI_COMMAND_QUEUE )
 *      device index :  device index to commit to.
 *
 * This function transmits a queued PDU over the hci uart interface.
 * It initially sets up pointers from supplied info
 * The transmission continues character
 * by character (or in blocks determined by the Tx FIFO length, if 
 * available), through the end of transmission interrupt.
 ****************************************************************************/
void TRAhcit_UART_Transmit(u_int8 *data, u_int32 pdu_len, u_int8 *header, 
                           u_int8 head_len, u_int8 tra_source, 
                           t_deviceIndex device_index)
{
    /*
     * Set this first to stop any overlap attempts 
     */

    sys_hcit_uart_info.common.tx_busy = 1;

    sys_hcit_uart_info.common.tx_device_index = device_index;
    sys_hcit_uart_info.common.tx_tra_source = tra_source;

    /*
     * Store the pointer to the header
     */

    tra_hcit_uart_cur_tx_pointer = header;

    /*
     * Number of header bytes to be transmitted
     */
    
    tra_hcit_uart_cur_tx_length = head_len;

    /*
     * Store the pdu buffer and length
     */

    tra_hcit_uart_tx_pdu_buf = data;
    tra_hcit_uart_tx_pdu_length = pdu_len;

    /* 
     * Original length of PDU (minus header) for ACKing the PDU from the queue
     */
    tra_hcit_uart_tx_original_length = pdu_len;    

    tra_hcit_uart_tx_state = TRA_HCIT_UART_STATE_TX_HEADER;

    /* 
     * Now that the proper information is set up, send packet 
     */
    TRAhcit_UART_Tx_PDU();
}


/*****************************************************************************
 * Function: TRAhcit_UART_Tx_PDU(void)
 * parameters: void.
 * 
 * This function sends the PDU checking after each byte if it was the last
 ****************************************************************************/
void TRAhcit_UART_Tx_PDU(void)
{
    /*
     * Call the UARTs transmit function, with header bytes and length
     * make it call back to TRAhcit_UART_Tx_Head when finished. The transmit
     * function will update the pointers by the amount it transmitted 
     * (always 1 for non-FIFO enabled UARTs)
     */

#if TRA_HCIT_UART_POLLED_TX_SUPPORTED == 1

    if(!sys_hcit_uart_info.poll_tx)
#endif
    {
        /*
         * If we enter here via a UART callback when CTS == 1
         * has been received via MSR, we must check if this is busy
         * so that we can validly finish the transmission (if we were
         * interrupted by DCTS in the last byte.
         */
        
        if(sys_hcit_uart_info.common.tx_busy)
        {
            if(!tra_hcit_uart_cur_tx_length)
            {
                /*
                 * If the header is finished being sent, move onto the PDU
                 */
            
                if(tra_hcit_uart_tx_state == TRA_HCIT_UART_STATE_TX_HEADER)
                { 
                    tra_hcit_uart_cur_tx_pointer = tra_hcit_uart_tx_pdu_buf;
                    tra_hcit_uart_cur_tx_length = tra_hcit_uart_tx_pdu_length;
                    tra_hcit_uart_tx_state = TRA_HCIT_UART_STATE_TX_BODY;
                }
                else 
                { 
                    /* 
                     * Finished sending the PDU, return to start 
                     * Immediate return for faster interrupt 
                     */
                    TRAhcit_Generic_Acknowledge_Complete_Tx(
                        sys_hcit_uart_info.common.tx_tra_source, 
                        tra_hcit_uart_tx_original_length); 
                    sys_hcit_uart_info.common.tx_busy = 0;
                    return;  
                } 
            }
            sys_hcit_uart_info.transmit(&tra_hcit_uart_cur_tx_pointer, 
                &tra_hcit_uart_cur_tx_length, TRAhcit_UART_Tx_PDU);
        }
    }

#if (TRA_HCIT_UART_POLLED_TX_SUPPORTED == 1)
    else
    {
        /*
         * Otherwise, we must poll spinning
         * NOTE: The goto is used here for more compact code
         */

    do_remainder:

        while(tra_hcit_uart_cur_tx_length)
        {
            sys_hcit_uart_info.poll_tx(&tra_hcit_uart_cur_tx_pointer, 
                &tra_hcit_uart_cur_tx_length);
        }
        if(tra_hcit_uart_tx_state == TRA_HCIT_UART_STATE_TX_HEADER)
        {
            tra_hcit_uart_cur_tx_pointer = tra_hcit_uart_tx_pdu_buf;
            tra_hcit_uart_cur_tx_length = tra_hcit_uart_tx_pdu_length;
            tra_hcit_uart_tx_state = TRA_HCIT_UART_STATE_TX_BODY;
            goto do_remainder; 
        } 
    
        TRAhcit_Generic_Acknowledge_Complete_Tx(
            sys_hcit_uart_info.common.tx_tra_source, 
            tra_hcit_uart_tx_original_length); 
        sys_hcit_uart_info.common.tx_busy = 0;
    }
#endif
}

/****************************************************************************
 * Function: TRAhcit_UART_Rx_Char
 * Paramters: ch : character received from the UART driver
 *
 * This function is called by the individual UARTs receive interrupt handler
 * When a full HCI packet has been decoded, this will call the HCI generic 
 * commit function to validate the entry just placed on the queue. 
 ****************************************************************************/
void TRAhcit_UART_Rx_Char(u_int8 ch)
{
    /*
     * Receive state machine. Stores 3/4 header bytes seperately until
     * the length has been received and decoded. At that point a buffer is
     * obtained from the generic transport mechanism and the PDU is enqueued.
     * This function uses early returns at the end of each state block. This is
     * for speed in exiting, as this function currently runs at the highest 
     * interrupt priority, and should take as little time as possible
     */

    /*
     * NOTE: 
     * This module could employ an optional watchdog timer of one second 
     * to reset the state to TRA_HCIT_UART_STATE_RX_PTYPE in case of a loss of 
     * synchronisation.
     */

    /*
     * The goto labels handle commands like reset which need the state machine
     * to be rerun as there will not be another character to provoke it into 
     * doing so. Basically, items with zero length. Multiple goto labels used
     * to avoid the top level switch.
     */

    switch(tra_hcit_uart_rx_state)    
    {
        case TRA_HCIT_UART_STATE_RX_DATA_CONT:
_TRA_HCIT_UART_STATE_RX_DATA_CONT:

#if (PRH_BS_DEV_BIG_ENDIAN_AND_DIRECT_BASEBAND_MOVE==1)
            /*
             * Copy in the received character into the newly allocated buffer
             * Swap the byte address within word32 (data is then ready for direct
             * copy to ACL buffer in baseband)
             */
             if (tra_hcit_uart_rx_type == TRA_HCIT_ACLDATA)
             {
                (*((u_int8*) (((u_int32) (tra_hcit_uart_rx_pdu_buf))^0x3))) = ch; 
             }
             else
#endif
            /*
             * Copy in the received character into the newly allocated buffer
             */
            *tra_hcit_uart_rx_pdu_buf = ch;               

            /*
             * Increase pointer to next byte, and denote one less byte to go
             */
            tra_hcit_uart_rx_pdu_buf++;  
            tra_hcit_uart_rx_pdu_length--;     
    
            if(!tra_hcit_uart_rx_pdu_length)
            {    
                /*
                 * Time to commit, set new state and continue 
                 */  
                tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_DATA_COMMIT;
                goto _TRA_HCIT_UART_STATE_RX_DATA_COMMIT;
            }
#if defined(TRA_HCIT_UART_DIRECT_RX_READ)
            else if(TRAhcit_UART_Is_Rx_Byte_Available())
            {
                ch = TRAhcit_UART_Get_Rx_Byte();
                goto _TRA_HCIT_UART_STATE_RX_DATA_CONT;
            }
#endif
            return;  

        case TRA_HCIT_UART_STATE_RX_TYPE:
/* _TRA_HCIT_UART_STATE_RX_TYPE: */

        	tra_hcit_uart_rx_activity = TRUE;
        	
            /* 
             * New HCI Transport packet, must first determine type 
             * Note: tra_hcit_uart_rx_type is assigned immediate (faster than = ch) 
             */
            switch(ch)
            {
                case TRA_HCIT_COMMAND:
                    tra_hcit_uart_rx_type = TRA_HCIT_COMMAND;    
                    tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE1;
		            tra_hcit_uart_hardware_error_event_flag=0;
                    return;
                case TRA_HCIT_ACLDATA:
                    tra_hcit_uart_rx_type = TRA_HCIT_ACLDATA;
                    tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_ACL_HEAD1;
		            tra_hcit_uart_hardware_error_event_flag=0;
                    return;
                case TRA_HCIT_SCODATA:
                    tra_hcit_uart_rx_type = TRA_HCIT_SCODATA;
                    tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_SCO_HEAD1;
		            tra_hcit_uart_hardware_error_event_flag=0;
                    return;
                default:
                    tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_TYPE;
                    /* 
                     * Error Handle hook 
                     */
		            if (tra_hcit_uart_hardware_error_event_flag==0)
		            {
			            HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_HCIT_RX_PDU_MALFORMED);
			            tra_hcit_uart_hardware_error_event_flag=1;
		            }
	        }
            break;        

        case TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE1:
/* _TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE1: */

            tra_hcit_uart_rx_head_buf[0] = ch;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE2;
            return;
            
        case TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE2:
/* _TRA_HCIT_UART_STATE_RX_COMMAND_OPCODE2: */

            tra_hcit_uart_rx_head_buf[1] = ch;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_COMMAND_LENGTH;
            return;
            
        case TRA_HCIT_UART_STATE_RX_COMMAND_LENGTH:
/* _TRA_HCIT_UART_STATE_RX_COMMAND_LENGTH: */

            /* 
             * Final state in decoding header of received HCI command 
             * Note: Keep track of the head length (in case PDU is zero length)   
             */
            tra_hcit_uart_rx_head_buf[2] = ch;
            tra_hcit_uart_rx_pdu_length = ch;
            tra_hcit_uart_rx_head_length = 3;         
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_DATA_START;    
            
            /*
             * Handle zero length commands, in particular the HCI Reset command
             * There will be no more characters for this command 
             * - so re-run state machine straight away
             */
            if (tra_hcit_uart_rx_pdu_length==0)            
            {
                goto _TRA_HCIT_UART_STATE_RX_DATA_START;    
            }
            return;
            
        case TRA_HCIT_UART_STATE_RX_ACL_HEAD1:
/* _TRA_HCIT_UART_STATE_RX_ACL_HEAD1: */

            tra_hcit_uart_rx_head_buf[0] = ch;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_ACL_HEAD2;
            return;
            
        case TRA_HCIT_UART_STATE_RX_ACL_HEAD2:
/* _TRA_HCIT_UART_STATE_RX_ACL_HEAD2: */

            tra_hcit_uart_rx_head_buf[1] = ch;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_ACL_LENGTH1;
            return;

        case TRA_HCIT_UART_STATE_RX_ACL_LENGTH1:
/* _TRA_HCIT_UART_STATE_RX_ACL_LENGTH1: */

            tra_hcit_uart_rx_head_buf[2] = ch;
            tra_hcit_uart_rx_pdu_length = ch;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_ACL_LENGTH2;
            return;

        case TRA_HCIT_UART_STATE_RX_ACL_LENGTH2:
/* _TRA_HCIT_UART_STATE_RX_ACL_LENGTH2: */

            /* 
             * Final state in decoding header of received ACL 
             */
            tra_hcit_uart_rx_head_buf[3] = ch;
            /* 
             * Now, decode length fully 
             */
            tra_hcit_uart_rx_pdu_length |= (ch << 8);
            /* 
             * This is set to zero here as header space is not actually 
             * allocated on the queue as with commands. 
             */    
            tra_hcit_uart_rx_head_length = 0;           
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_DATA_START;

            /* 
             * If the ACL data length is 0 Then
             *    Immediately re-run state machine to enqueue 0 length data pdu
             * Endif
             * Note: Unlike commands, the header length is not added on here, 
             *       as it's stored seperately.  
             */                        
            if(tra_hcit_uart_rx_pdu_length==0) 
                goto _TRA_HCIT_UART_STATE_RX_DATA_START;    
            return;

        case TRA_HCIT_UART_STATE_RX_SCO_HEAD1:
/* _TRA_HCIT_UART_STATE_RX_SCO_HEAD1: */

            tra_hcit_uart_rx_head_buf[0] = ch;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_SCO_HEAD2;
            return;
            
        case TRA_HCIT_UART_STATE_RX_SCO_HEAD2:
/* _TRA_HCIT_UART_STATE_RX_SCO_HEAD2: */

            tra_hcit_uart_rx_head_buf[1] = ch;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_SCO_LENGTH;
            return;
            
        case TRA_HCIT_UART_STATE_RX_SCO_LENGTH:
/* _TRA_HCIT_UART_STATE_RX_SCO_LENGTH: */

            /* 
             * Final state in decoding header of received SCO 
             * As with ACL data, SCO headers are stored seperately from the data, 
             * so this is zero for enqueuing 
             */
            tra_hcit_uart_rx_head_buf[2] = ch;
            tra_hcit_uart_rx_pdu_length = ch;
            tra_hcit_uart_rx_head_length = 0;  
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_DATA_START;

            /* 
             * If the SCO data length is 0 Then
             *    Immediately re-run state machine to enqueue 0 length data pdu
             * Endif
             */                        
            if(tra_hcit_uart_rx_pdu_length==0)
            {
                goto _TRA_HCIT_UART_STATE_RX_DATA_START;   
            }
            return;
        
        case TRA_HCIT_UART_STATE_RX_DATA_START:
_TRA_HCIT_UART_STATE_RX_DATA_START:

            /* 
             * This function should return a buffer for writing the remainder of 
             * the PDU into.  
             * HCI commands will have their headers copied in automatically, 
             * ACL data will have their headers stripped i.e. the information 
             * will be stored in members of the queue descriptor.
             */
            tra_hcit_uart_rx_pdu_buf = 
                (volatile u_int8*) TRAhcit_Generic_Get_Rx_Buf(tra_hcit_uart_rx_type,
                tra_hcit_uart_rx_pdu_length + tra_hcit_uart_rx_head_length, 
                tra_hcit_uart_rx_head_buf);

            if(!tra_hcit_uart_rx_pdu_buf)
            {
                /*
                 * No buffer available, hence skip rest of packet
                 * Host will be notified via HCI_DATA_BUFFER_OVERFLOW_EVENT in 
                 * TRAhcit_Generic_Get_Rx_Buf()
                 */
                tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_DATA_WHEN_NO_BUFFER;
                goto _TRA_HCIT_UART_STATE_RX_DATA_WHEN_NO_BUFFER;
            }

            if(!tra_hcit_uart_rx_pdu_length)    /* Zero length command or data */
            {
                tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_DATA_COMMIT;
                goto _TRA_HCIT_UART_STATE_RX_DATA_COMMIT;
            }

            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_DATA_CONT;
            /* 
             * The goto avoids duplication of the code in DATA_CONT - space saving
             */    
            goto _TRA_HCIT_UART_STATE_RX_DATA_CONT;        

            
        /* 
         * When we get here, just keep receiving until the remaining length runs out 
         */
        
        case TRA_HCIT_UART_STATE_RX_DATA_WHEN_NO_BUFFER:
_TRA_HCIT_UART_STATE_RX_DATA_WHEN_NO_BUFFER:

            /*
             * This state handles case when no buffer was available.
             * All rx bytes of packet are skipped.
             * Host will be notified via HCI_DATA_BUFFER_OVERFLOW_EVENT
             */   
            if(tra_hcit_uart_rx_pdu_length == 0 || 
                --tra_hcit_uart_rx_pdu_length == 0)
            {    
                tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_TYPE;
            }    
            return;

        case TRA_HCIT_UART_STATE_RX_DATA_COMMIT:
_TRA_HCIT_UART_STATE_RX_DATA_COMMIT:

            /* 
             * Finished reading the pdu, therefore commit entry 
             */
                
            TRAhcit_Generic_Commit_Rx_Buf(tra_hcit_uart_rx_type);
    
            *(u_int32*)tra_hcit_uart_rx_head_buf = 0; 
            tra_hcit_uart_rx_activity = FALSE;
            tra_hcit_uart_rx_state = TRA_HCIT_UART_STATE_RX_TYPE;
            
            return;
                    
        default:    /* Error Handle hook */
            return;    
    }
    return;    
}        

/****************************************************************************
 * Function: 
 * Paramters:
 *
 ****************************************************************************/
boolean TRAhcit_UART_Transport_Activity(void)
{
	return (tra_hcit_uart_rx_activity || sys_hcit_uart_info.common.tx_busy);
}

#endif


/*****************************************************************************
 * Functions: TRAhcit_UART_Get_Default_Baud_Rate
 *            TRAhcit_UART_Set_Default_Baud_Rate
 * Parameters: 
 *
 * Returns: 
 * 
 * Used to modify the default HCIT baud rate over the TCI, and used by flash
 * update functions to store non-volatile configurations afterwards.
 *
 ****************************************************************************/
static u_int32 tra_hcit_uart_baud_rate = 0;

u_int32 TRAhcit_UART_Get_Default_Baud_Rate(void)
{
    return tra_hcit_uart_baud_rate;
}

void TRAhcit_UART_Set_Default_Baud_Rate(u_int32 baud_rate)
{
    tra_hcit_uart_baud_rate = baud_rate;
}

