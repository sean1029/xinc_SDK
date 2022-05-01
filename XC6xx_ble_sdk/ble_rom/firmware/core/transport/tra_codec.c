/*****************************************************************************
 * MODULE NAME:    tra_codec.c
 * PROJECT CODE:   Bluetooth
 * DESCRIPTION:    Codec FIFO Transport Interface
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  27 Feb 2013
 *
 * SOURCE CONTROL: $Id: tra_codec.c,v 1.5 2013/10/25 16:49:05 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2013 Ceva Inc
 *     All rights reserved.
 *
 *****************************************************************************/
#include "sys_config.h"
#include "tra_codec.h"
#include "tra_queue.h"
#include "lmp_types.h"
#include "lmp_sco_container.h"
#include "lc_interface.h"
#include "hw_lc.h"
#include "hw_delay.h"
#include "hw_memcpy.h"
#include "hw_codec.h"

#include "sys_debug_config.h"           


#if (PRH_BS_CFG_SYS_ESCO_VIA_VCI_SUPPORTED==1)

static u_int8 _TRAcodec_rx_data_buffer[PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH + 4];
static u_int8 _TRAcodec_rx_data_index = 0;

static u_int8 _TRAcodec_tx_data_buffer[PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH + 4];
static u_int8 _TRAcodec_tx_data_index = 0;

static u_int8 _TRAcodec_rx_data_count = 0;
static u_int8 _TRAcodec_tx_data_count = 0;

/*****************************************************************************
 * TRAcodec_Initialise
 *
 ****************************************************************************/
void TRAcodec_Initialise(void)
{
    _TRAcodec_rx_data_index = 0;
    _TRAcodec_tx_data_index = 0;

	_TRAcodec_rx_data_count = 0;
	_TRAcodec_tx_data_count = 0;
}

/*****************************************************************************
 * TRAcodec_Get_FIFO_Sample_Size
 *
 ****************************************************************************/
u_int8 TRAcodec_Get_FIFO_Sample_Size(void)
{
	/* FIFO sample size may be 8-bit or 16-bit - check hal HWcodec configuration */
	u_int8 input_sample_size = LMscoctr_Get_Input_Sample_Size(HWcodec_Get_Voice_Setting());
    return ((input_sample_size > 8)?(2):(1)); // return size in bytes
}

/*******************************************************************************
 *
 * FUNCTION: TRAcodec_Enable
 *
 ******************************************************************************/
void TRAcodec_Enable(void)
{
	/*
	 * If support SCO and eSCO via VCI need to enable the VCI FIFO functionality
	 * else can use the standard VCI interface for SCO connections only.
	 */
	u_int32 _dummy_read;

	HW_set_vci_rgf_fifo_reset(1);
	HW_set_vci_rgf_mode_enable(1);
	HWdelay_Wait_For_us(1);
	HW_set_vci_rgf_fifo_reset(0);

	HW_set_vci_rgf_fifo_16bit_mode(1);

	HW_set_vci_tx_fifo_threshold(10); /* HW_get_vci_tx_fill_status()==1 on overflow */
	HW_set_vci_rx_fifo_threshold(10); /* HW_get_vci_rx_fill_status()==1 on underflow */

	/* The first read access from the VCI FIFO is garbage - as the reading of the
	 * register triggers a new read from the FIFO, i.e. data is from previous read.
	 */
	_dummy_read = HW_read_vci_tx_fifo_data();
}

/*******************************************************************************
 *
 * FUNCTION: TRAcodec_Disable
 *
 ******************************************************************************/
void TRAcodec_Disable(void)
{
	HW_set_vci_rgf_mode_enable(0);
}

/*****************************************************************************
 * TRAcodec_Dispatch_Pending_SCO
 *
 ****************************************************************************/
t_error TRAcodec_Dispatch_Pending_SCO(void)
{
	u_int8 sco_index, index, rx_fifo_fill_level, tx_fifo_fill_level;

	u_int8 vci_sample_size = TRAcodec_Get_FIFO_Sample_Size();

	t_q_descr *qd;
    u_int32 sco_data;

	for (sco_index=0; sco_index<PRH_BS_CFG_SYS_MAX_SYN_LINKS; sco_index++)
	{
		t_sco_info *p_sco_link = LMscoctr_Find_SCO_By_SCO_Index(sco_index);

		if (p_sco_link && (p_sco_link->state != SCO_IDLE))
		{
			/* Dispatch Incoming SCO/eSCO Data */

			rx_fifo_fill_level = HW_get_vci_rx_fifo_fill_level();

			while (rx_fifo_fill_level < (TRA_CODEC_VCI_RX_FIFO_SIZE - (4/vci_sample_size)))
    		{
				if (_TRAcodec_rx_data_count < 4) /* data written 4 bytes at a time */
				{
            		qd = BTq_Dequeue_Next_Data_By_Length(SCO_IN_Q, sco_index, 0);

	            	if (qd)
		        	{
			    		BTq_Reduce_Queue_Number_Of_Processed_Chunks(SCO_IN_Q, sco_index, 1);

						for (index = 0; index < _TRAcodec_rx_data_count; index++)
            			{
            				_TRAcodec_rx_data_buffer[index] = _TRAcodec_rx_data_buffer[_TRAcodec_rx_data_index++];
            			}

						hw_memcpy8( (void*)&_TRAcodec_rx_data_buffer[_TRAcodec_rx_data_count], (void*)qd->data, qd->length);

						_TRAcodec_rx_data_count += qd->length;
						_TRAcodec_rx_data_index = 0;

						BTq_Ack_Last_Dequeued(SCO_IN_Q, sco_index, qd->length);
					}
            		else
            		{
            			break; /* no incoming SCO/eSCO data to process */
            		}
				}
				else
				{
				#if (0 == 1)
                    static volatile u_int32  *log_tx  =  (volatile unsigned *)(0x20000000);;
                #endif
            		index = _TRAcodec_rx_data_index;

					sco_data = (((u_int32)_TRAcodec_rx_data_buffer[index+3])<<24)
							 + (((u_int32)_TRAcodec_rx_data_buffer[index+2])<<16)
							 + (((u_int32)_TRAcodec_rx_data_buffer[index+1])<<8)
							 + (_TRAcodec_rx_data_buffer[index]);

                 #if (0 == 1)
                    (*log_tx++) = sco_data;                    
                 #endif

            	 #if (1 == __DEBUG_SCO_MIPS)
            	 {
            	    static  unsigned    int test_cnt = 0;
                    //PrintK(DEFAULT_LEVEL," [bt] vci_rx_sco_data=0x%x\r\n" , sco_data);              
                    if(test_cnt++&0x01) { *GPIO_PORT_DR0 = (1<<19); }  //- gpio[3]
                    else { *GPIO_PORT_DR0 = (1<<19)| (1<<3); }
                 }
                 #endif                  
					HW_write_vci_rx_fifo_data(sco_data);

					_TRAcodec_rx_data_index += 4;
					_TRAcodec_rx_data_count -= 4;

					rx_fifo_fill_level += 4/vci_sample_size; // = HW_get_vci_rx_fifo_fill_level();
				}
	    	}

			/* Dispatch Outgoing SCO/eSCO Data */

			tx_fifo_fill_level = HW_get_vci_tx_fifo_fill_level();

			while (tx_fifo_fill_level >= (4/vci_sample_size))
			{
				if (_TRAcodec_tx_data_count < PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH)
				{
            		sco_data = HW_read_vci_tx_fifo_data();
            	#if (0==1)
                    PrintK(DEFAULT_LEVEL," [bt] vci_tx_sco_data=0x%x\r\n" , sco_data);
                #endif    
					_TRAcodec_tx_data_buffer[_TRAcodec_tx_data_index++] = sco_data & 0xFF;
					_TRAcodec_tx_data_buffer[_TRAcodec_tx_data_index++] = (sco_data>>8) & 0xFF;
					_TRAcodec_tx_data_buffer[_TRAcodec_tx_data_index++] = (sco_data>>16) & 0xFF;
					_TRAcodec_tx_data_buffer[_TRAcodec_tx_data_index++] = (sco_data>>24) & 0xFF;

					_TRAcodec_tx_data_count += 4;

					tx_fifo_fill_level -= (4/vci_sample_size); // = HW_get_vci_tx_fifo_fill_level();
				}
				else
				{
					qd = BTq_Enqueue(SCO_OUT_Q, sco_index, PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH);

					if (qd)
					{
						hw_memcpy8( qd->data, (void *)&_TRAcodec_tx_data_buffer[0], PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH);
						BTq_Commit(SCO_OUT_Q, sco_index);

						BTq_Reduce_Queue_Number_Of_Processed_Chunks(SCO_OUT_Q, sco_index, 1);

						_TRAcodec_tx_data_count -= PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH;

            			for (index = 0; index < _TRAcodec_tx_data_count; index++)
            			{
            				_TRAcodec_tx_data_buffer[index] = _TRAcodec_tx_data_buffer[index+PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH];
            			}

						_TRAcodec_tx_data_index = _TRAcodec_tx_data_count;
					}
					else
					{
            			break; /* no outgoing SCO/eSCO data to process */
					}
				}
			}

			break; // no need to check other sco_index - HW supports just one SCO/eSCO via VCI
		}
	}

    return (t_error)0;
}

#if  (1 == __DEBUG_SCO_MIPS)
extern  void    sco_mips_toggle(void)
{
    static  unsigned    int idle_loop =0;
    if(idle_loop++&0x01)   {*GPIO_PORT_DR0 = (1<<20);}    // gpio[4].
    else    {*GPIO_PORT_DR0 = (1<<20)|(1<<4);}    
}
#endif

#endif
