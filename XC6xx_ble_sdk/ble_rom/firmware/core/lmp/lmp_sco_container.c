/***********************************************************************
 *
 * MODULE NAME:    lmp_sco_container.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Maintains a container of SCOs. 
 *             
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  11 February 2000
 *
 * SOURCE CONTROL: $Id: lmp_sco_container.c,v 1.104 2014/03/11 03:14:01 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    11.Feb.2000 -   GF       - initial version
 *    
 ***********************************************************************/

#include "sys_config.h"
#include <assert.h>                     /* Must be after sys_config.h           */
#include <string.h>                     /* For memset                           */

#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1))

#include "lmp_config.h"
#include "lmp_acl_container.h"
#include "lmp_ch.h"
#include "lmp_encode_pdu.h"
#include "lmp_features.h"
#include "lmp_link_policy.h"
#include "lmp_sco_container.h"
#include "lmp_utils.h"
#include "lslc_access.h"

#include "lc_interface.h"
#include "bt_timer.h"
#include "hc_const.h"
#include "hw_codec.h"
#include "hw_memcpy.h"
#include "hci_params.h"

t_esco_descr LMscoctr_eSCO_Scratchpad;
//u_int8 LMscoctr_max_latency;
u_int16 LMscoctr_last_tx_lm_pkt_index;

const u_int8 eSCO_packet_mapping[2][7] = {{ LM_SCO_PACKET_EV3, LM_SCO_PACKET_EV4, LM_SCO_PACKET_EV5, LM_SCO_PACKET_2EV3,LM_SCO_PACKET_3EV3, LM_SCO_PACKET_2EV5, LM_SCO_PACKET_3EV5}, { EV3, EV4, EV5, EDR_2EV3, EDR_3EV3, EDR_2EV5,EDR_3EV5 }};
const u_int16 eSCO_HCI_packet_mapping[7] = { HCI_SYN_EV3, HCI_SYN_EV4,HCI_SYN_EV5, HCI_SYN_2EV3, HCI_SYN_3EV3,HCI_SYN_2EV5, HCI_SYN_3EV5};

/*
 * Helper functions
 */
u_int8 LMscoctr_Get_Max_Pkt_Len(t_packet pkt_type);
static u_int8 _LMscoctr_Derive_D_SCO(u_int8 lm_sco_packet, u_int8 timing_ctrl);
#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
static u_int8 _LMscoctr_Derive_D_SCO_Repeater_Mode(t_sco_info *p_sco_link_new,u_int8 timing_ctrl);
#endif


/* 
 * Array of SCO structures, 
 *  one per simultaneous SCO supported
 *  one for negotiation of SCO e.g. request to Master/Slave, Change Packet Type
 */
t_sco_info sco_link_container[PRH_BS_CFG_SYS_MAX_SYN_LINKS+1]; 
/*
 * For collisions it is impossible to detect from LMP_not_accepted
 * message whether the response is for a new connection
 * request or for an existing SCO.  Hence need to record.
 */
u_int8 _LMscoctr_lm_sco_handle_for_last_slave_LMP_sco_link_req;

/***********************************************************************
 * MACRO :- mLMscoctr_Get_Negotiate_Sco_Link_Ref
 *
 * DESCRIPTION :- 
 * Returns a reference to the sco link to be used for negotiation
 ***********************************************************************/
#define  mLMscoctr_Get_Negotiate_Sco_Link_Ref() \
    (sco_link_container + PRH_BS_CFG_SYS_MAX_SYN_LINKS)

u_int8 _LMscoctr_number_scos_used;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
extern t_q_descr eSCO_Tx_Descriptor[PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI];
extern u_int8 eSCO_Tx_Buffers[PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI][PRH_BS_CFG_SYS_SIZEOF_ESCO_OUT_HEAP];
u_int8 _LMeScoctr_number_escos_used;
t_sco_info*  _p_LMscoctr_Topical_eSco_Link;
t_esco_transaction_type _LMscoctr_Topical_eSco_Link_Transaction_Type;
t_lm_sco_states LMscoctr_saved_state;
int _LMscoctr_num_d_escos, _LMscoctr_active_d_esco_index, _LMscoctr_num_rs_t_escos;
u_int8 _LMscoctr_d_esco_list[MAX_NUM_D_ESCOS_IN_LIST];
u_int8 _LMscoctr_rs_t_esco_list[MAX_NUM_RS_T_ESCOS_IN_LIST];

/*
 * Ensure that the lookahead bits of MAX_D_ESCO_LOOKAHEAD_FRAMES
 * can all be stored.
 */
u_int8 _LMeScoctr_d_esco_calculator_bitmap[(MAX_D_ESCO_LOOKAHEAD_FRAMES+7)/8];
void _LMscoctr_SetDEscoCalculatorBit(int bit_pos);
int  _LMscoctr_TestDEscoCalculatorBit(int bit_pos);
void _LMscoctr_ClearDEscoCalculatorBit(int bit_pos);

t_esco_retrans_mode _LMscoctr_esco_retrans_mode;

#endif



/***********************************************************************
 * FUNCTION :- LMscoctr_Initialise
 *
 * DESCRIPTION :- 
 * Initialises the SCO link container.
 ***********************************************************************/
void LMscoctr_Initialise(void)
{
    u_int i;
    t_sco_info *p_sco_link;

    /*
     * Initialise eSCO retransmission mode to regular.
     */
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    _LMscoctr_esco_retrans_mode = REGULAR_RETRANS;
#endif

    /*
     * Initialise all the sco_containers, including ones for negotiation.
     */
    for (i=0; i < sizeof(sco_link_container)/sizeof(sco_link_container[0]); i++)
    {
        p_sco_link = &sco_link_container[i];
        p_sco_link->state = SCO_UNUSED;

        /*
         * Store index in container for fast mapping
         */
        p_sco_link->sco_index = i;
    } 

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    /*
     * Initialise the eSCO buffers for all sco_containers, except
     * the container used for negotiation.
     *
     * Note: Does the sco_container used for the negotiation require
     * an eSCO buffer ?
     */
    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        p_sco_link = &sco_link_container[i];
        p_sco_link->tx_descriptor = &eSCO_Tx_Descriptor[i];
        p_sco_link->tx_descriptor->data = eSCO_Tx_Buffers[i];
    } 

    _LMeScoctr_number_escos_used = 0;
#endif

    _LMscoctr_number_scos_used = 0;

	
    /*
     * Disable codec until sco connections are activated.
     */
     LC_Disable_Codec();
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Alloc
 *
 * DESCRIPTION :- 
 * Allocates a SCO link structure. 
 * Returns a pointer to the sco link structure if there was one free.
 * Otherwise return 0.
 *
 * device_index     the associated link device index
 * sco_cfg          the hardware configuration to be used
 * link_type        SCO link or eSCO link.
 ***********************************************************************/
t_sco_info* LMscoctr_Alloc(t_deviceIndex device_index, u_int16 voice_setting, t_linkType link_type)
{
    u_int i = 0;
    t_sco_info *p_sco_link = 0;    /* Default, all links allocated */
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    t_devicelink* p_device_link = DL_Get_Device_Ref(device_index);
#endif
    
    /*
     * Enable codec if SCO connections active via codec.
     */
    if(((voice_setting & PRH_BS_HCI_VOICE_SETTING_FOR_SCO_VIA_HCI)==0)
      && (_LMscoctr_number_scos_used==0)
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
      && (_LMeScoctr_number_escos_used==0)
#endif
      )
    {
        LC_Enable_Codec();
    }

    while((sco_link_container[i].state != SCO_UNUSED) && 
          (i < PRH_BS_CFG_SYS_MAX_SYN_LINKS))
    {
        i++;
    }

    if (i < PRH_BS_CFG_SYS_MAX_SYN_LINKS)
    {
	   /*
	    * Mark SCO as allocated
		*/
        p_sco_link = &sco_link_container[i];
        p_sco_link->state = SCO_IDLE;
        p_sco_link->link_type = link_type;
        p_sco_link->voice_setting = voice_setting;
		
        /*
		* Set the SCO Connection Handle/SCO LMP handle in the Link Structure
		*/
        p_sco_link->connection_handle = PRH_SCO_CONNECTION_HANDLE_OFFSET + i;
        p_sco_link->lm_sco_handle = i+1; /* Note SCO handle 0 is invalid */
		
        {
			t_lmp_link* p_link = LMaclctr_Find_Device_Index(device_index);
			assert(p_link);
			
			if((p_link->role == SLAVE) && (link_type == ESCO_LINK))
			{
				p_sco_link->lm_sco_handle = 0;
			}
			
			p_sco_link->device_index = device_index;
			
			/*
			* Record the current active hardware SCO configuration.
			*/
			p_sco_link->sco_cfg_hardware = LC_Get_Hardware_SCO_CFG(voice_setting);
			
			p_sco_link->d_sco = 0;
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)

			p_sco_link->esco_end_retransmission_window_timer = 0;
			p_sco_link->esco_lt_addr = 0;
			p_sco_link->tx_packet_type = (t_packet)0;
			p_sco_link->rx_packet_type = (t_packet)0;
			p_sco_link->negotiation_state = 0;
			p_sco_link->rx_packet_length = 0;
			p_sco_link->tx_packet_length = 0;
			p_sco_link->w_esco = 0;
			p_sco_link->renegotiated_params_available=0;
			
			if(link_type == ESCO_LINK)
			{
				if(p_link->role == MASTER)
				{
					p_sco_link->esco_lt_addr = DL_Alloc_eSCO_LT_Addr();
					if(p_sco_link->esco_lt_addr == NO_AMADDR_AVAILABLE)
					{
						p_sco_link->state = SCO_IDLE;
						return 0;
					}
				}
				/*
				* Allocate an eSCO LT for this device_link
				*/
				/*
				*
				* CM: 22 SEP 03
				* 
				* This is required as the the allocation of the LT container
				* for the device_link (ie the lt_index) is done at a point
				* where the lt_addr is not known. Once the lt_addr is known
				* the LM must ensure that the association between the 
				* lt_index and lt_addr is made.
				* 
				* Would like to get rid of the requirement for this element !
				*/
				p_sco_link->esco_lt_index = DL_Allocate_eSCO_Link(p_device_link);
				_LMeScoctr_number_escos_used++;
				
#if 1
				// Init Codec and Transcoder Settings
				//
				p_sco_link->Start_Tx_Hpf_Filt =0;
				p_sco_link->Start_Tx_Pf1_Filt_A = 0;
				p_sco_link->Start_Tx_Pf1_Filt_B = 0;
				p_sco_link->Start_Tx_Pf2_Filt = 0;
				p_sco_link->Start_Tx_Pf3_Filt = 0;
				p_sco_link->Start_Tx_Cvsd_Filt_A = 0;
				p_sco_link->Start_Tx_Cvsd_Filt_B = 0;
				
				p_sco_link->Start_Rx_Hpf_Filt = 0;
				p_sco_link->Start_Rx_Pf1_Filt_A = 0;
				p_sco_link->Start_Rx_Pf1_Filt_B = 0;
				p_sco_link->Start_Rx_Pf2_Filt = 0;
				p_sco_link->Start_Rx_Pf3_Filt = 0;
				p_sco_link->Start_Rx_Cvsd_Filt_A = 0;
				p_sco_link->Start_Rx_Cvsd_Filt_B = 0;
				
				p_sco_link->End_Tx_Hpf_Filt = 0;
				p_sco_link->End_Tx_Pf1_Filt_A = 0;
				p_sco_link->End_Tx_Pf1_Filt_B = 0;
				p_sco_link->End_Tx_Pf2_Filt = 0;
				p_sco_link->End_Tx_Pf3_Filt = 0;
				p_sco_link->End_Tx_Cvsd_Filt_A = 0;
				p_sco_link->End_Tx_Cvsd_Filt_B = 0;
				
				p_sco_link->End_Rx_Hpf_Filt = 0;
				p_sco_link->End_Rx_Pf1_Filt_A = 0;
				p_sco_link->End_Rx_Pf1_Filt_B = 0;
				p_sco_link->End_Rx_Pf2_Filt = 0;
				p_sco_link->End_Rx_Pf3_Filt = 0;
				p_sco_link->End_Rx_Cvsd_Filt_A = 0;
				p_sco_link->End_Rx_Cvsd_Filt_B = 0;
#endif		
			}
#endif
        }

        p_sco_link->lm_air_mode = 0;
        p_sco_link->lm_sco_packet = 0;
        p_sco_link->next_activity_time = 0;
        p_sco_link->packet_types = 0;
        p_sco_link->t_sco = 0;
        p_sco_link->timing_ctrl = 0;

        if(link_type==SCO_LINK)
        {
            _LMscoctr_number_scos_used++;

            if (SLAVE == DL_Get_Role_Peer(DL_Get_Device_Ref(device_index)))
                g_LM_config_info.num_master_sco_links++;
            else
                g_LM_config_info.num_slave_sco_links++;
        }
    }
 
    return p_sco_link;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Free
 *
 * DESCRIPTION :- 
 * Frees a previously allocated SCO in the sco_container. 
 ***********************************************************************/
void  LMscoctr_Free(t_sco_info* p_sco_link)
{
    t_lmp_link* p_link;

    p_link = LMaclctr_Find_Device_Index(p_sco_link->device_index);
    if (p_sco_link)
    {
        p_sco_link->state = SCO_UNUSED;
        if(p_sco_link->link_type==SCO_LINK)
        {
            _LMscoctr_number_scos_used--;

            if (SLAVE == DL_Get_Role_Peer(DL_Get_Device_Ref(p_sco_link->device_index)))
                g_LM_config_info.num_master_sco_links--;
            else
                g_LM_config_info.num_slave_sco_links--;
        }

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        if(p_sco_link->link_type == ESCO_LINK)
        {
            if (p_link->role == MASTER)
            {
                DL_Free_eSCO_LT_Addr(p_sco_link->esco_lt_addr);
            }
            DL_Free_eSCO_Link(DL_Get_Device_Ref(p_sco_link->device_index), p_sco_link->esco_lt_index);
            _LMeScoctr_number_escos_used--;
        }        
#endif
    }

#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
    /* 
     * Reallow scans if either the following are now satisifed:
     * NO SCO connections or 1 HV3 master SCO.
     */
    if ( (LMscoctr_Get_Number_SCO_Connections() == 0) ||
         ( (LMscoctr_Get_Number_SCO_Connections() == 1) && 
           (LMscoctr_Get_Active_LM_SCO_Packet_Type() == LM_SCO_PACKET_HV3) &&
           (!LMconfig_LM_Connected_As_Slave()) ) )
    {
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_SCO_PREVENTING_SCANS);    
    }
#endif

    /*
     * Disable codec if no SCO connections active.
     */
    if((_LMscoctr_number_scos_used==0)
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        && (_LMeScoctr_number_escos_used==0)
#endif
        )
    {
        LC_Disable_Codec();
    }

    /*
     * However, there is an undetected possibility of a scenario where the
     * negotiating SCO is not freed due to a cryptic collision.
     * As this is a blocker for any future SCOs, prevent it!
     */
    if (_LMscoctr_number_scos_used == 0)
    {
        t_sco_info *p_sco_renegotiate_link = mLMscoctr_Get_Negotiate_Sco_Link_Ref();
        /*
         * Ensure in test that this is never noted.
         */
        p_sco_renegotiate_link->state = SCO_UNUSED;
    }
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Free_SCOs_On_Link
 *
 * DESCRIPTION :- 
 * Frees all SCOs associated with an ACL link identified by 
 * "device_index"
 ***********************************************************************/
void LMscoctr_Free_SCOs_On_Link(t_deviceIndex device_index)
{
    t_sco_info* p_sco_link;
    u_int i;

    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        p_sco_link = &sco_link_container[i];        
        if ((p_sco_link->state != SCO_UNUSED) && 
            (p_sco_link->device_index == device_index))
        {
            LMscoctr_Free(p_sco_link);  
        }
    }
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Get_Number_SCO_Connections
 *
 * DESCRIPTION :- 
 * Returns number of SCOs active 
 ***********************************************************************/
u_int8 LMscoctr_Get_Number_SCO_Connections(void)
{
    return _LMscoctr_number_scos_used;
}

u_int8 LMscoctr_Get_Number_SYN_Connections(void)
{
    u_int8 num_syns = _LMscoctr_number_scos_used
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    + _LMeScoctr_number_escos_used
#endif
        ;

    return num_syns;
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Get_Number_SCOs_Active_On_Link
 *
 * DESCRIPTION :- 
 * Returns number of SCOs active on an ACL link identified by device_index
 ***********************************************************************/
u_int8 LMscoctr_Get_Number_SCOs_Active_On_Link(t_deviceIndex device_index)
{
    u_int8 scos_active_on_link = 0;
    u_int i;

    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        if ((sco_link_container[i].state != SCO_UNUSED) && 
            (sco_link_container[i].device_index == device_index))
        {
            ++scos_active_on_link;
        }
    }
    return scos_active_on_link;
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Get_Number_SCOs_Active
 *
 * DESCRIPTION :- 
 * Returns the total number of SCOs active in the device 
 ***********************************************************************/
u_int8 LMscoctr_Get_Number_SCOs_Active(void)
{
    u_int8 scos_active = 0;
    u_int i;

    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        if (sco_link_container[i].state == SCO_ACTIVE)
        {
            ++scos_active;
        }
    }
    return scos_active;
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Derive_SCO_Parameters
 *
 * DESCRIPTION :- 
 * Establishes the SCO parameters for a new SCO link to support 
 * a given packet_type. The LM packet type is derived, the T-sco & D-sco 
 * determined, and the timing control allocated. In addition the air_mode
 * is determined.
 * 
 * p_sco_link       sco container to be setup
 * packet_types     packet_types to be allocated
 * role             device role MASTER or SLAVE
 * flag             initiating a SCO or responding to a peer
 *                  SCO?
 *
 *
 * Notes:
 * 1. If packet types are not valid, HV1's are selected.
 * 2. This function should not be called to re-negotiate a SCO
 *    since lm_sco_packet will reflect the existing packet!
 * 3. Since Master only, use Native Clock
 * 4. If responding to a peer SCO attempt, use the air-mode
 *    suggested by the peer, else use the default air-mode.
 *
 ***********************************************************************/
void LMscoctr_Derive_SCO_Parameters(
    t_sco_info *p_sco_link, t_packetTypes packet_types, t_role role,
    u_int8 use_peer_air_mode_flag)
{
    u_int8 lm_sco_packet;

    lm_sco_packet = LMscoctr_Derive_LM_SCO_Packet_Type(p_sco_link, packet_types);
    p_sco_link->lm_sco_packet = lm_sco_packet; 
    p_sco_link->t_sco = (lm_sco_packet + 1) << 1;

    if(use_peer_air_mode_flag == 0)
    {
        p_sco_link->lm_air_mode = LMscoctr_Map_AirMode_HCI2LMP((u_int8)LMscoctr_Get_Air_Coding(p_sco_link->voice_setting));
    }

    p_sco_link->packet_types = packet_types;

    if (role==MASTER)
    {
        u_int8 timing_ctrl = (u_int8)((LC_Get_Native_Clock() & 0x08000000) >> 27);
#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
        if (LC_Get_SCO_Repeater_Bit())
        {
           p_sco_link->d_sco = _LMscoctr_Derive_D_SCO_Repeater_Mode(p_sco_link, timing_ctrl);
        }
        else
#endif
        {
            p_sco_link->d_sco = _LMscoctr_Derive_D_SCO(lm_sco_packet, timing_ctrl);
        }
        p_sco_link->timing_ctrl = timing_ctrl;
        /*
         * p_sco_link_dest->lm_sco_handle assigned at SCO allocation
         */
         
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /* 
         * Disallow scans if both the following are not satisifed:
         * NO SCO connections or 1 HV3 master SCO.
         */
        if ((LMscoctr_Get_Number_SCO_Connections() > 1) || (lm_sco_packet != LM_SCO_PACKET_HV3))
        {
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SCO_PREVENTING_SCANS);    
        }
#endif
         
    }
    else
    {
        /*
         * For a slave lm_sco_handle, d_sco and timing_ctrl are invalid
         */
        p_sco_link->lm_sco_handle = 0; 
        p_sco_link->d_sco = 0;
        p_sco_link->timing_ctrl = 0;
        /*
         * For collisions it is impossible to detect from LMP_not_accepted
         * message whether the response is for a new connection
         * request or for an existing SCO.  Hence need to record.
         */
        _LMscoctr_lm_sco_handle_for_last_slave_LMP_sco_link_req = 0;
        
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /* 
         * Disallow scans if SCO connection as slave.
         */
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SCO_PREVENTING_SCANS);    
#endif

    }
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Assign_SCO_Parameters
 *
 * DESCRIPTION :- 
 * Assigns the SCO parameters for the SCO link from existing sco
 * definition.  Typically used in case of re-negotiation 
 ***********************************************************************/
void LMscoctr_Assign_SCO_Parameters(
    t_sco_info *p_sco_link_dest, const t_sco_info *p_sco_link_source)
{
    p_sco_link_dest->d_sco = p_sco_link_source->d_sco;
    p_sco_link_dest->t_sco = p_sco_link_source->t_sco;

    p_sco_link_dest->packet_types = p_sco_link_source->packet_types;

    p_sco_link_dest->lm_sco_packet = p_sco_link_source->lm_sco_packet;
    p_sco_link_dest->lm_air_mode = p_sco_link_source->lm_air_mode;

    p_sco_link_dest->timing_ctrl = p_sco_link_source->timing_ctrl;
    p_sco_link_dest->lm_sco_handle = p_sco_link_source->lm_sco_handle;
}


/********************************************************************
 * FUNCTION :- LMscoctr_Extract_SCO_Parameters_From_PDU
 * 
 * DESCRIPTION 
 * This function is responsible for Extacting the SCO parameters from 
 * an LMP_SCO_Link_Req PDU. 
 *
 * NOTE :- Additional parameter checking has yet to be performed here.
 * The master has to perform checks on the supplied paramters
 * Reasons for failure (included in the return LMP_NOT_ACCEPTED
 *    0x0a: Max number of SCO connections to a device.
 *    0x1b: SCO offset rejected.
 *    0x1c: SCO interval rejected
 *    0x1d: SCO air mode rejected
 *
 * The LMP_SCO_link_req parameters in p_payload are:
 *  SCO handle 
 *  timing control flags 
 *  D sco
 *  T sco
 *  SCO packet
 *  air mode
 *************************************************************************/
t_error LMscoctr_Extract_SCO_Parameters_From_PDU(
    t_sco_info *p_sco_link, t_p_pdu p_pdu, t_role role)
{
    t_error reject_reason = NO_ERROR;

    /*
     * Extract sco_handle if a slave else just ignore.
     */
    if (role == SLAVE)
    {
        p_sco_link->lm_sco_handle = *p_pdu;
    }
    p_pdu++;

    /* 
     * Need to assign parameters AFTER error checking has been performed 
     */
    p_sco_link->timing_ctrl = ((*(p_pdu++) & 0x2) >> 1);
    p_sco_link->d_sco = *p_pdu++;
    p_sco_link->t_sco = *p_pdu++;
    p_sco_link->lm_sco_packet = *p_pdu++;

    /*
     * Only packet types I know for now is what is presented in LMP_sco_link_req
     * For this device, once SCO is supported, all packets types are.
     *
     * p_sco_link->lm_sco_packet is guaranteed to be valid on entry to
     * this function.
     */
    p_sco_link->packet_types = (0x0020 << p_sco_link->lm_sco_packet);

    if ((role == SLAVE) && ((p_sco_link->t_sco >> 1) != (p_sco_link->lm_sco_packet+1)))
    {
        reject_reason = SCO_INTERVAL_REJECTED;
    }

    if (p_sco_link->d_sco > p_sco_link->t_sco)
    {
        reject_reason = SCO_OFFSET_REJECTED;
    }

    p_sco_link->lm_air_mode = *p_pdu;


    /*
     * Allow a device to dynamically turn off the SCO feature
     */
    if (!SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_SCO_FEATURE))
    {
        reject_reason = UNSUPPORTED_REMOTE_FEATURE;    
    }

    return reject_reason;   
}   


/***********************************************************************
 *
 * FUNCTION :- LMscoctr_Map_AirMode_LMP2HCI
 *
 * DESCRIPTION :- 
 * Maps the LMP Air Mode to the HCI air mode.
 *
 ***********************************************************************/
u_int8 LMscoctr_Map_AirMode_LMP2HCI(u_int8 lm_air_mode)
{
    const u_int8 lm_air_mode_to_hci_air_mode[4] = 
        {HCI_AIR_MODE_U_LAW, HCI_AIR_MODE_A_LAW, 
         HCI_AIR_MODE_CVSD, HCI_AIR_MODE_TRANSPARENT};
    return lm_air_mode_to_hci_air_mode[lm_air_mode&0x03];
}


/***********************************************************************
 *
 * FUNCTION :- LMscoctr_Map_AirMode_HCI2LMP
 *
 * DESCRIPTION :- 
 * Maps the HCI Air Mode to the LMP air mode.
 *
 ***********************************************************************/
u_int8 LMscoctr_Map_AirMode_HCI2LMP(u_int8 hci_air_mode)
{
    const u_int8 hci_air_mode_to_lm_air_mode[4] = 
        {LM_AIR_MODE_CVSD,  LM_AIR_MODE_U_LAW, 
         LM_AIR_MODE_A_LAW, LM_AIR_MODE_TRANSPARENT};
    return hci_air_mode_to_lm_air_mode[hci_air_mode&0x03];
}


/***********************************************************************
 *
 * FUNCTION :- LMscoctr_Is_LM_Air_Mode_Supported
 *
 * DESCRIPTION :- 
 *    Determines if the local host controller can
 *    support the LMP air mode.
 *
 ***********************************************************************/
boolean LMscoctr_Is_LM_Air_Mode_Supported(u_int8 lm_air_mode)
{
    u_int8 lm_air_mode_supported = 0;

    if(SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_CVSD_FEATURE))
        lm_air_mode_supported |= (1 << LM_AIR_MODE_CVSD);
    if(SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_U_LAW_FEATURE))
        lm_air_mode_supported |= (1 << LM_AIR_MODE_U_LAW);
    if(SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_A_LAW_FEATURE))
        lm_air_mode_supported |= (1 << LM_AIR_MODE_A_LAW);
    if(SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_TRANSPARENT_SCO_FEATURE))
        lm_air_mode_supported |= (1 << LM_AIR_MODE_TRANSPARENT);

    return( (lm_air_mode_supported & (1 << lm_air_mode)) >> (lm_air_mode) );
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Get_Active_LM_SCO_Packet_Type
 *
 * DESCRIPTION :- 
 * Returns current active LM SCO packet type  
 *    0: HV1, 1:HV2, 2:HV3 or LM_SCO_PACKET_INVALID
 ***********************************************************************/
u_int8 LMscoctr_Get_Active_LM_SCO_Packet_Type(void)
{
    u_int8 lm_sco_packet_active = LM_SCO_PACKET_INVALID;
    u_int i;

    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        if ((sco_link_container[i].state != SCO_UNUSED) &&
            (sco_link_container[i].state != SCO_IDLE) )
        {
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
			if ( sco_link_container[i].link_type == ESCO_LINK)
			{
                lm_sco_packet_active = LMscoctr_Get_eSCO_LMP_Type_From_BB_Packet(sco_link_container[i].tx_packet_type);
            }
			else
#endif
			lm_sco_packet_active = sco_link_container[i].lm_sco_packet;
            break;
        }
    }
    return lm_sco_packet_active;
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Get_Used_LM_SCO_Packet_Type
 *
 * DESCRIPTION :- 
 * Returns current used LM SCO packet type  
 *    0: HV1, 1:HV2, 2:HV3 or LM_SCO_PACKET_INVALID
 ***********************************************************************/
u_int8 LMscoctr_Get_Used_LM_SCO_Packet_Type(void)
{
    u_int8 lm_sco_packet_active = LM_SCO_PACKET_INVALID;
    u_int i;

    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        if (sco_link_container[i].state != SCO_UNUSED)
        {
            lm_sco_packet_active = sco_link_container[i].lm_sco_packet;
            break;
        }
    }
    return lm_sco_packet_active;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Is_All_Bandwidth_Used_By_SCO
 *
 * DESCRIPTION :- 
 * Returns true if all the slots used by SCO 
 * i.e. TRUE if 1 HV1, 2 HV2, 3 HV3
 ***********************************************************************/
u_int8 LMscoctr_Is_All_Bandwidth_Used_By_SCO(void)
{
    return (_LMscoctr_number_scos_used) &&
       (LMscoctr_Get_Active_LM_SCO_Packet_Type()+1 ==  
        _LMscoctr_number_scos_used);
}


/***********************************************************************
 *
 * FUNCTION :- LMscoctr_Is_All_Bandwidth_Used_By_SYN
 *
 * DESCRIPTION :- 
 * Returns true if all the slots used by SCO or eSCO
 *  slots.
 *
 ***********************************************************************/
u_int8 LMscoctr_Is_All_Bandwidth_Used_By_SYN(void)
{
    u_int8 sum_link_slot_occupancy = 0;
    u_int8 link_slot_occupancy = 0;
    u_int8 largest_t_esco = 0;
    int j;
    t_sco_info* p_esco_link;

    /*
     * Check for 100% bandwidth usage by sum of all (t*u) pairs.
     */
    for (j=0; j < PRH_BS_CFG_SYS_MAX_SYN_LINKS; j++)
    {
        p_esco_link = sco_link_container+j;
        if ((p_esco_link->state != SCO_UNUSED) &&
            (p_esco_link->state != SCO_IDLE))
        {
            /*
             * Find the largest t_esco.
             */
            if(largest_t_esco < p_esco_link->t_sco)
            {
                largest_t_esco = p_esco_link->t_sco;
            }
        }
    }

    for (j=0; j < PRH_BS_CFG_SYS_MAX_SYN_LINKS; j++)
    {
        p_esco_link = sco_link_container+j;
        if ((p_esco_link->state != SCO_UNUSED) &&
            (p_esco_link->state != SCO_IDLE))
        {
            /*
             * Find if all the bandwidth is
             * used by reserved slots.
             */
            if(p_esco_link->link_type == SCO_LINK)
            {
                link_slot_occupancy = 2;
            }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
            else
            {
                link_slot_occupancy = LMscoctr_GetSlotOccupancy(p_esco_link->tx_packet_type)+
                    LMscoctr_GetSlotOccupancy(p_esco_link->rx_packet_type);
                link_slot_occupancy += p_esco_link->w_esco;
            }
#endif

            link_slot_occupancy *= (largest_t_esco/p_esco_link->t_sco);
            sum_link_slot_occupancy += link_slot_occupancy;
        }
    }

    /*
     * If the bandwidth would be completely filled by
     * reserved slots if this new connection was added,
     * return -1.
     */
    if((sum_link_slot_occupancy) && (sum_link_slot_occupancy == largest_t_esco))
    {
        return 1;
    }

    return 0;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Is_Bandwidth_Available_For_SCO_Packet_Types
 *
 * DESCRIPTION :- 
 * Returns true if bandwidth available for any of the packets
 *
 * Bandwidth available if either
 * 1. No SCOs currently used && no eSCOs currently used.
 * 2. There is SYN bandwidth and the active packet type is one of selected 
 * packet_types     Bit field of packet types
 ***********************************************************************/
u_int8 LMscoctr_Is_Bandwidth_Available_For_SCO_Packet_Types
    (t_packetTypes packet_types)
{
    u_int8 check;

    u_int8 lm_sco_packet_active = LMscoctr_Get_Active_LM_SCO_Packet_Type();

    check = _LMscoctr_number_scos_used == 0 ||
            (!LMscoctr_Is_All_Bandwidth_Used_By_SCO() &&
              (packet_types & (HV1_BIT_MASK<<lm_sco_packet_active)) );

#if(PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    /*
     * if there is still a possiblity of bandwidth
     * being available at this point:
     *
     * then check if the Derive_D_eSCO function
     * thinks its possible to allocate the connection.
     */
    {
    u_int8 sco_packets[3][2] = {{ HV1, 2 }, { HV2, 4}, {HV3, 6} };
    int i;
    if((check) && (_LMeScoctr_number_escos_used > 0))
    {
        check = 0;
        for(i = 0; i < 3; i++)
        {
            if(packet_types & (1 << sco_packets[i][0]) )
            {
                if(-1 == LMscoctr_Derive_D_eSCO(0 /*esco_link*/, 
                    sco_packets[i][1], 0, 0, 
                    (t_packet)sco_packets[i][0], 
                    (t_packet)sco_packets[i][0]))
                {
                    check++;
                }
            }
            else
            {
                check++;
            }
        }

        /*
         * If check is 3, then have tried all possible
         * packet types (i.e. all packet types in packet_types
         * and none of them could fit into the existing
         * eSCO/SCO allocation.
         *
         * So, set check to 0 indicating false and return it.
         */
        if(check == 3)
        {
            check = 0;
        }
    }
    }
#endif

    return check;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Get_SCO_Index
 *
 * DESCRIPTION :- 
 * Returns the SCO index for the SCO link identified by p_sco_link
 ***********************************************************************/
u_int8 LMscoctr_Get_SCO_Index(const t_sco_info *p_sco_link)
{
    return (p_sco_link->sco_index);
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Find_SCO_By_Device_Index
 *
 * DESCRIPTION :- 
 * Returns the next SCO active on the ACL link identified by device_index
 ***********************************************************************/
t_sco_info *LMscoctr_Find_SCO_By_Device_Index(t_deviceIndex device_index)
{
    t_sco_info *p_sco_link = 0;
    u_int i;

    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        if ((sco_link_container[i].state != SCO_UNUSED) && 
            (sco_link_container[i].device_index == device_index))
        {
            p_sco_link = &sco_link_container[i];
        }
    }
    return p_sco_link;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Find_SCO_By_Connection_Handle
 *
 * DESCRIPTION :- 
 * Finds and returns a pointer to the SCO identified by the 
 * connection handle "handle".
 *
 * NOTE :- The connection_handle is used to uniquely identify the SCO
 *         across the HCI interface.
 ***********************************************************************/
t_sco_info *LMscoctr_Find_SCO_By_Connection_Handle(t_connectionHandle handle)
{
    t_sco_info *p_sco_link = 0;

    if ((handle >= PRH_SCO_CONNECTION_HANDLE_OFFSET) && 
        (handle <= PRH_SCO_CONNECTION_HANDLE_OFFSET + PRH_BS_CFG_SYS_MAX_SYN_LINKS - 1))
    {
        p_sco_link = &sco_link_container[handle-PRH_SCO_CONNECTION_HANDLE_OFFSET];
        if (p_sco_link->state == SCO_UNUSED)
        {
            p_sco_link = 0;
        }
    }

    return p_sco_link;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Find_SCO_By_SCO_Handle
 *
 * DESCRIPTION :- 
 * Finds and returns a pointer to the SCO identified by the 
 * SCO handle "sco_handle".
 *
 * NOTE :- The sco_handle is used to uniquely identify the SCO
 * accross peer Link Managers. It is unrelated to the connection
 * handle used to identify the SCO on the HCI interface.
 *
 * sco_handle 0 is invalid and returns NULL pointer.
 *
 ***********************************************************************/
t_sco_info* LMscoctr_Find_SCO_By_SCO_Handle(u_int8 sco_handle)
{
    t_sco_info *p_sco_link = NULL;
    u_int i;

    if (sco_handle != 0)
    {
        for (i=0; i<PRH_BS_CFG_SYS_MAX_SYN_LINKS && p_sco_link==0; i++)
        {
            /*
             * Select next sco container
             * If (Not USED OR handle match)
             *    Ignore selection
             * Endif
             */
            p_sco_link = &sco_link_container[i];
            if ( (p_sco_link->state == SCO_UNUSED) || 
                 (p_sco_link->lm_sco_handle != sco_handle) )
            {
                p_sco_link = 0;
            }
        }
    }
    return p_sco_link; 
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Find_SCO_By_SCO_Index
 *
 * DESCRIPTION :- 
 * Finds and returns a pointer to the SCO identified by the 
 * SCO index if the associated container is allocated. 
 * The SCO index is the index of the SCO in the 
 * sco container.
 * 
 ***********************************************************************/
t_sco_info *LMscoctr_Find_SCO_By_SCO_Index(u_int8 index)
{
    t_sco_info *p_sco_link = 0;

    if (index < PRH_BS_CFG_SYS_MAX_SYN_LINKS &&
       sco_link_container[index].state != SCO_UNUSED)
    {
        p_sco_link = &sco_link_container[index];
    }
    else
    {
        ; /* temporary for breaking */
        p_sco_link = 0;
    }

    return p_sco_link;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Derive_LM_SCO_Packet_Type
 *
 * DESCRIPTION :-
 * Determines the lm sco packet types (0-HV1, 1-HV2, 2-HV3) 
 * to be supported on a new SCO link.
 *
 * NOTE:  Only one SCO packet type can be supported by a device
 *        at any particular time due to the periodicity of packets
 *
 * The following are the combinations :  1 * HV1 or 2 * HV2 or 3 * HV3
 *
 * To determine the packet type the following steps are performed.
 * if (sco already active )
 *     Use the existing sco packet type (there is no choice!)
 * else ( if no scos active )
 *     Use the type with highest t_sco
 * else
 *     Set the type to HV1 (default).
 *
 * This function should not be used for SCO admission control.
 * Admission of SCO request should be checked before invoking.
 *
 ***********************************************************************/
u_int8 LMscoctr_Derive_LM_SCO_Packet_Type(
    const t_sco_info *p_sco_link, t_packetTypes packet_types)
{
    u_int8 lm_sco_packet_type = LM_SCO_PACKET_HV1;
    u_int8 lm_sco_packet_active;

    lm_sco_packet_active = LMscoctr_Get_Active_LM_SCO_Packet_Type();

    if (lm_sco_packet_active != LM_SCO_PACKET_INVALID)
    {
        lm_sco_packet_type = lm_sco_packet_active;
    }
    else
    {
        /* 
         * No SCOs ACTIVE - Select the packet type with lowest t_sco 
         */
        if(packet_types & HV1_BIT_MASK) 
        {
            lm_sco_packet_type = LM_SCO_PACKET_HV1;
        }
        else if (packet_types & HV2_BIT_MASK) 
        {
            lm_sco_packet_type = LM_SCO_PACKET_HV2;
        }
        else if (packet_types & HV3_BIT_MASK)
        {
            lm_sco_packet_type = LM_SCO_PACKET_HV3;
        }
    }

    return lm_sco_packet_type;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Get_SCO_Transitioning_Ref
 *
 * DESCRIPTION :- 
 * Determines if there is a SCO already in the process of being
 * established to any device, a packet change or a disconnect
 * is taking place.  
 * If found then returns a reference to that sco container.
 * If no such container then return NULL.
 *
 * A SCO can be transitioning
 * 1. If being setup by Master or Slave                 
 *      state is SCO_IDLE/SCO_ACTIVATION_PENDING
 * 2. If changing packet type/air mode for Master/Slave 
 *      state is SCO_CHANGE_PKT_ACCEPT_ACK_PENDING
 * 3. If being disconnected
 *      state is SCO_IDLE (before becoming SCO_UNUSED)
 *
 * All SCO containers including the negotiating container are checked.
 ***********************************************************************/
t_sco_info* LMscoctr_Get_SCO_Transitioning_Ref(void)
{
    t_sco_info *p_sco_link;
    t_sco_info *p_sco_under_negotiation = NULL;
    u_int i = 0;

    /*
     * Include the negotiated link as well!
     */
    while (i < PRH_BS_CFG_SYS_MAX_SYN_LINKS+1 && p_sco_under_negotiation==NULL)
    {
        p_sco_link = &sco_link_container[i];
        if (p_sco_link->state==SCO_UNUSED || 
            p_sco_link->state==SCO_ACTIVE 
#if 1 // GF 12 May fix for #2516
            ||
			p_sco_link->state==SCO_ACTIVATION_PENDING
#endif
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
            ||
            p_sco_link->state >= eSCO_MASTER_ACTIVATION_PENDING
#endif
            )
        {
            /*
             * Next container
             */
            i++;
        }
        else
        {
            /*
             * This container is being negotiated.
             */
            p_sco_under_negotiation = p_sco_link;
        }
    }

    return p_sco_under_negotiation;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Find_SCO_Under_Negotiation
 *
 * DESCRIPTION :- 
 * Returns the SCO under negotiation for the given role.  Since the
 * Slave/Master handling is different, the role is required.
 *
 * role         MASTER or SLAVE
 *
 * Returns
 * NULL                     if no SCO is transitioning for this role
 * sco_info reference       if  a SCO is transitioning for this role
 *
 * A SCO can be under negotiation 
 * 1. If changing packet type/air mode for Master/Slave
 *
 ***********************************************************************/
t_sco_info* LMscoctr_Find_SCO_Under_Negotiation(t_role role)
{
    t_sco_info *p_sco_renegotiate_link;

    if (role==SLAVE)
    {
        p_sco_renegotiate_link =  LMscoctr_Find_SCO_By_SCO_Handle(
                    _LMscoctr_lm_sco_handle_for_last_slave_LMP_sco_link_req);
    }
    else
    {
        p_sco_renegotiate_link = mLMscoctr_Get_Negotiate_Sco_Link_Ref();

        if (p_sco_renegotiate_link->state != SCO_UNUSED)
        {
            /*
             * Changing an existing SCO, find the SCO transitioning
             */
            p_sco_renegotiate_link =  LMscoctr_Find_SCO_By_SCO_Handle(
                    p_sco_renegotiate_link->lm_sco_handle);
        }
        else
        {
            /*
             * Possibly new SCO being setup, return NULL
             */
            p_sco_renegotiate_link = NULL;
        }
    }
    return p_sco_renegotiate_link;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Allocate_Negotation_SCO_Link
 *
 * DESCRIPTION :- 
 * Allocate a sco container to support negotiation of an existing link.
 * The only parameters that can be changed are packet type and
 * air mode.
 *
 * p_sco_link   Reference to sco container to be updated with 
 *              negotiated parameters.
 * role         Allocate and change for MASTER or SLAVE
 * packet_types A bit vector of possible packet types
 * lm_air_mode  The air mode to use.
 *
 * PRECONDITIONS :-
 * 1. A sco link can only be renegotiated if there is only one active.
 * 2. For a Master the Timing Control Flags and Dsco are re-established
 * 3. For a Slave the Timing Control Flags are not valid/ignored (0).
 *
 * Currently, a slave device only uses the container as a temporary 
 * sco link holder.
 ***********************************************************************/
t_sco_info *LMscoctr_Allocate_Negotation_SCO_Link(
     const t_sco_info *p_sco_link, t_role role,
     t_packetTypes packet_types, u_int8 lm_air_mode)
{
    t_sco_info *p_sco_renegotiate_link = mLMscoctr_Get_Negotiate_Sco_Link_Ref();
    u_int8 lm_sco_packet = LMscoctr_Convert_HCI_To_LM_SCO_Packet_Type(packet_types);
    
    p_sco_renegotiate_link->state = SCO_IDLE;    

    /* 
     * The t_sco can be derived from proposed packet type IF 
     * the sco packet type is supported.
     * Since only a single SCO active, always d_sco = 0
     */
    p_sco_renegotiate_link->d_sco = 0;
    p_sco_renegotiate_link->t_sco = (lm_sco_packet +1) << 1;

    p_sco_renegotiate_link->packet_types = packet_types;

    p_sco_renegotiate_link->lm_sco_packet = lm_sco_packet;
    p_sco_renegotiate_link->lm_air_mode = lm_air_mode;

    p_sco_renegotiate_link->lm_sco_handle = p_sco_link->lm_sco_handle;

    if (role==SLAVE)
    {
        p_sco_renegotiate_link->timing_ctrl = 0;
        _LMscoctr_lm_sco_handle_for_last_slave_LMP_sco_link_req = p_sco_link->lm_sco_handle;
    }
    else
    {
        p_sco_renegotiate_link->timing_ctrl = 
            (u_int8)((LC_Get_Native_Clock() & 0x08000000) >> 27);
    }

    return p_sco_renegotiate_link;
}            


/***********************************************************************
 * FUNCTION :- LMscoctr_Free_Negotiation_SCO_Link
 *
 * DESCRIPTION :- 
 * Free the SCO container used for MASTER|SLAVE based renegotiation.
 * In a some situations in a slave due to collisions, the
 * container will be Freed twice in a row.  Although, undesirable
 * the alternative is multiple negotiation containers.  Use
 * case scenarios show no side effect.
 ***********************************************************************/
void LMscoctr_Free_Negotiation_SCO_Link(void)
{
    t_sco_info *p_sco_renegotiate_link = mLMscoctr_Get_Negotiate_Sco_Link_Ref();

    p_sco_renegotiate_link->state = SCO_UNUSED;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Accept_And_Free_Negotiated_SCO_Link
 *
 * DESCRIPTION :- 
 * Accept the negotiated parameters and free the negotiation link.
 *
 * p_sco_link   Reference to sco container to be updated with 
 *              negotiated parameters.
 *
 * Currently, only a Master uses the negotiation container.
 ***********************************************************************/
void LMscoctr_Accept_And_Free_Negotiated_SCO_Link(t_sco_info *p_sco_link)
{
    LMscoctr_Assign_SCO_Parameters(p_sco_link, 
        mLMscoctr_Get_Negotiate_Sco_Link_Ref());
    LMscoctr_Free_Negotiation_SCO_Link();
    
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
    /* 
     * Reallow scans if single master SCO renegotiated to HV3. Disallow
     * scans if single master SCO renegotiated to other sco packet type.
     */
    if (LMscoctr_Get_Number_SCO_Connections() == 1)
    {
        if(p_sco_link->lm_sco_packet == LM_SCO_PACKET_HV3)
        {
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_SCO_PREVENTING_SCANS);    
        }
        else
        {
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SCO_PREVENTING_SCANS);        
        }
    }
#endif

}


/***********************************************************************
 *
 * HELPER FUNCTIONS FOR lm_sco_container module
 *
 ***********************************************************************/


/***********************************************************************
 * FUNCTION :- LMscoctr_Convert_HCI_To_LM_SCO_Packet_Type
 *
 * DESCRIPTION :- 
 * Returns the associated LMP SCO packet type for HCI packet type (bits)
 *      0   sco packet HV1
 *      1   sco packet HV2
 *      2   sco packet HV3
 ***********************************************************************/
u_int8 LMscoctr_Convert_HCI_To_LM_SCO_Packet_Type(t_packetTypes packet_type)
{
    u_int8 lm_sco_packet_type;

    if(packet_type & HV1_BIT_MASK)
    {
        lm_sco_packet_type = LM_SCO_PACKET_HV1;
    }
    else if(packet_type & HV2_BIT_MASK)
    {
        lm_sco_packet_type = LM_SCO_PACKET_HV2;
    }
    else /*(packet_type & HV3_BIT_MASK)*/
    {
        lm_sco_packet_type = LM_SCO_PACKET_HV3;
    }

    return lm_sco_packet_type;
}


/***********************************************************************
 * FUNCTION :- _LMscoctr_Derive_D_SCO
 *
 * DESCRIPTION :- 
 * Determines the D-Sco value for a link. The D-Sco value is dependent
 * on what the current sco packet type is and the current allocated
 * d_sco values.  The d_sco values are dynamic and change as the clock
 * wraps and based on timing control flag.
 *
 * If D_SCO == 0 available Then Assign for all sco_packets
 * Else Assign D_SCO = 2 if available and sco_packet indicates HV2 or HV3
 * Else Assign D_SCO = 4 if available and sco_packet indicates HV3
 *
 * Returns  d_sco as        0, 2, 4
 ***********************************************************************/
static u_int8 _LMscoctr_Derive_D_SCO(u_int8 lm_sco_packet, u_int8 timing_ctrl)
{
    enum {D_SCO_0_ASSIGNED=0x01, D_SCO_2_ASSIGNED=0x02, D_SCO_4_ASSIGNED=0x04};
    u_int8 d_sco;               
    u_int8 assigned_d_sco;
    u_int  i;
    t_sco_info *p_sco_link;

    /*
     * Determine D_SCOs which are already allocated.
     */
    assigned_d_sco = 0;
    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        p_sco_link = sco_link_container+i;
        if ((p_sco_link->state != SCO_UNUSED) &&
            (p_sco_link->state != SCO_IDLE) )
        {
            d_sco = BTtimer_Extract_Timer_D_proc_slots(
                p_sco_link->next_activity_time, timing_ctrl, p_sco_link->t_sco);
            assigned_d_sco |= (1 << (d_sco>>1) );
        }
    }
    
    /*
     * Assign next available d_sco, assume initially invalid
     */
    d_sco = 6;
    if (!(assigned_d_sco & D_SCO_0_ASSIGNED))
    {
        d_sco = 0;
    }
    else if ( !(assigned_d_sco & D_SCO_2_ASSIGNED) && 
             (lm_sco_packet > LM_SCO_PACKET_HV1)/*sco HV2 or HV3*/ )
    {

        d_sco = 2;        
    }    
    else if ( !(assigned_d_sco & D_SCO_4_ASSIGNED) && 
             (lm_sco_packet == LM_SCO_PACKET_HV3) )
    {
        d_sco = 4;
    }

    return d_sco;
}


/*********************************************************************
 *
 * eSCO
 *
 ********************************************************************/
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
/***********************************************************************
 * FUNCTION :- LMscoctr_Derive_D_eSCO
 *
 * DESCRIPTION :- 
 * Determines the D-eSCO value for a link. The D-eSCO value is dependent
 * on what the current sco/esco t_sco is, what the
 * current w_esco are what the current allocated
 * d_sco values are.
 *
 * The d_sco values are dynamic and change as the clock
 * wraps and based on timing control flag.
 *
 * If D_SCO == 0 available Then Assign for all sco_packets
 * Else,
 * check if the t_sco/t_esco of the new link is compatible
 * with the existing t_sco/t_esco.
 * The new t_esco must either be a multiple of the
 * existing t_esco t_escos or it must be a divisor
 * of all the existing t_escos ...
 *
 * Finally there must be space for the new t_esco + new w_esco +
 * new packet type (s) in the cycle.
 *
 ***********************************************************************/
s_int16 LMscoctr_Derive_D_eSCO(t_sco_info* p_self_esco_link, u_int8 t_esco, 
    u_int8 w_esco, u_int8 timing_ctrl, t_packet tx_packet_type, 
    t_packet rx_packet_type)
{
	return LMscoctr_Derive_D_eSCO_AtLeast(0, p_self_esco_link,
		t_esco, w_esco, timing_ctrl,
		tx_packet_type, rx_packet_type);
}

/***********************************************************************
 *
 * FUNCTION :- LMscoctr_Derive_D_eSCO_AtLeast
 *
 * DESCRIPTION :- 
 * Determines the smallest possible D-eSCO value for a link
 * greater than or equal to the d_esco constraint passed into
 * the function. The D-eSCO value is dependent
 * on what the current sco/esco t_sco is, what the
 * current w_esco are what the current allocated
 * d_sco, w_esco, t_esco, packet_types, and SCO d_sco, t_sco 
 * values are.
 *
 * The d_sco values are dynamic and change as the clock
 * wraps and based on timing control flag.
 *
 * A pointer to the t_sco_info requesting the d_esco calculation
 * is passed to the function. This is to determine if the d_esco
 * is for a new connection (t_sco_info == NULL) or an existing
 * connection. If it is for an existing connection, that
 * t_sco_info's parameters are disregarded for the purposes
 * of calculating the new d_esco.
 *
 * The new t_esco must either be a multiple of all the
 * existing t_esco t_escos or it must be a divisor
 * of all the existing t_escos.
 *
 * Finally there must be space for the new t_esco + new w_esco +
 * new packet type (s) in the cycle.
 *
 ***********************************************************************/
s_int16 LMscoctr_Derive_D_eSCO_AtLeast(u_int8 init_d_esco, 
    t_sco_info* p_self_esco_link, u_int8 t_esco, 
    u_int8 w_esco, u_int8 timing_ctrl, t_packet tx_packet_type, 
    t_packet rx_packet_type)
{
    s_int16 d_esco = 0;
    u_int16  i = 0, j =0, k=0, problem = 0;
    t_sco_info *p_esco_link = 0;
    u_int8 link_slot_occupancy = 0;
    u_int8 sum_link_slot_occupancy = 0;
    u_int8 largest_t_esco = 0;
    int separation_factor = 0;

    memset(&_LMeScoctr_d_esco_calculator_bitmap, 0, 
        sizeof(_LMeScoctr_d_esco_calculator_bitmap) );

	LMscoctr_Clear_D_eSCO_List();

    /*
     * For all existing active synchronous
     * connections, mark out their known
     * occupancy.
     */
    for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
    {
        p_esco_link = sco_link_container+i;
        if ((p_esco_link->state != SCO_UNUSED) &&
            (p_esco_link->state != SCO_IDLE) &&
            (p_esco_link != p_self_esco_link))
        {
            /*
             * Extract what the d_esco would be if
             * this connection had been set up in
             * the current clock half.
             */
            d_esco = BTtimer_Extract_Timer_D_proc_slots(
                p_esco_link->next_activity_time, timing_ctrl, p_esco_link->t_sco);

            if(p_esco_link->link_type == SCO_LINK)
            {
                link_slot_occupancy = 2;
            }
            else
            {
                link_slot_occupancy = LMscoctr_GetSlotOccupancy(p_esco_link->tx_packet_type)+
                    LMscoctr_GetSlotOccupancy(p_esco_link->rx_packet_type);
            }

            /*
             * Catch the simple cases early.
             * If the new t_esco is neither a
             * multiple nor a divisor of all
             * the existing t_escos, then
             * no d_esco can be found for
             * the new t_esco.
             */
            if(p_esco_link->t_sco > t_esco)
            {
                if(p_esco_link->t_sco % t_esco)
                {
                    return -1;
                }
            }
            else if(p_esco_link->t_sco < t_esco)
            {
                if(t_esco % p_esco_link->t_sco)
                {
                    return -1;
                }
            }


            /* 
             * For MAX_D_ESCO_LOOKAHEAD_FRAMES
             */
            for(j = 0; (j+k) < MAX_D_ESCO_LOOKAHEAD_FRAMES; j+=2)
            {
                /*
                 * Once find a t_sco start point, mark the
                 * correct num_frames as occupied
                 * (1 x frame_occupancy) + (w_esco * frame_occupancy)
                 */
                if((j % p_esco_link->t_sco) == (u_int16)d_esco)
                {
                    /*
                     * If sharing the retransmission window, discount the
                     * w_esco component of any pre-existing links.
                     */
                    separation_factor = link_slot_occupancy+p_esco_link->w_esco;

                    for(k=0; k<separation_factor; k++)
                    {
                        _LMscoctr_SetDEscoCalculatorBit(j+k);
                    }

                    /*
                     * Count over k to optimise the algorithm.
                     */
                    j += t_esco;
                    k = 0;
                }
            }
        }
    }
 
    /*
     * Now, try
     * all possible d_esco values (in the range
     * 0  .. t_esco-2 inclusive) to see if any
     * d_esco value will avoid a clash with the existing
     * d_esco/t_esco/w_esco values.
     *
     */

    /*
     * For each possible d_esco, try to fit the new
     * esco connection into the bitmap.
     *
     * If the bitmap already is set in a particular
     * bit position, try the next d_esco value.
     * Return the d_esco value or -1 if no
     * suitable d_esco value can be found.
     */
    link_slot_occupancy = LMscoctr_GetSlotOccupancy(tx_packet_type)+
                LMscoctr_GetSlotOccupancy(rx_packet_type);;

    if(t_esco <= init_d_esco /* proposed_d_sco */)
    {
        d_esco = -1;
    }
    else
    {
        i = init_d_esco;

        /*
         * i represents all possible d esco candidates.
         */
        while(i <= t_esco)
        {
            problem = 0;

            j = 0; /* test clock value */
            k = 0; /* entire slot occupancy of link (instant + w_esco) */

            while( (j+k) < MAX_D_ESCO_LOOKAHEAD_FRAMES )
            {
                /* if the frame is an esco frame */
                if( (j % t_esco) == i /* proposed_d_esco */ ) 
                {
                    k = 0;
                    /*
                     * test all the bits occupied by the proposed
                     * link to see if any will cause a problem
                     */
                    while((k < (link_slot_occupancy+w_esco))
                        && ((j+k) < MAX_D_ESCO_LOOKAHEAD_FRAMES))
                    {
                        if(_LMscoctr_TestDEscoCalculatorBit(j+k))
                        {
                            problem = 1;
                            break;
                        }

                        k+= 2;
                    }

                    /*
                     * Optimisation. 
                     */
                    if(problem == 1)
                    {
                        /*
                         * No point going any further
                         * with this candidate. Go to
                         * next i candidate.
                         */
                        break;
                    }
                    else
                    {
                        j += t_esco;
                        k = 0;
                    }
                }
                else
                {
                    j+=2;
                }
            }

            /*
             * If have reached this point without
             * a problem, then a candidate has
             * emerged. Get out of i loop.
             */
            if(problem == 0)
			{
				LMscoctr_Add_D_eSCO_Element(i);
			}
            i += 2;
        }
		
        if(0 == _LMscoctr_num_d_escos)
        {
            d_esco = -1;
        }
        else
        {
			d_esco = _LMscoctr_d_esco_list[0];

            /*
             * take account of the reserved slots
             * of the proposed new connection.
             */
            largest_t_esco = t_esco;
            link_slot_occupancy = LMscoctr_GetSlotOccupancy(tx_packet_type)+
                LMscoctr_GetSlotOccupancy(rx_packet_type);
            sum_link_slot_occupancy = link_slot_occupancy;

            /*
             * Check for 100% bandwidth usage by sum of all (t*u) pairs.
             */
            for (j=0; j < PRH_BS_CFG_SYS_MAX_SYN_LINKS; j++)
            {
                p_esco_link = sco_link_container+j;
                if ((p_esco_link->state != SCO_UNUSED) &&
                    (p_esco_link->state != SCO_IDLE) &&
                    (p_esco_link != p_self_esco_link))
                {
                    /*
                     * Find the largest t_esco.
                     */
                    if(largest_t_esco < p_esco_link->t_sco)
                    {
                        sum_link_slot_occupancy *= (p_esco_link->t_sco/largest_t_esco);
                        largest_t_esco = p_esco_link->t_sco;
                    }

                    /*
                     * Find if all the bandwidth is
                     * used by reserved slots.
                     */
                    if(p_esco_link->link_type == SCO_LINK)
                    {
                        /*
                         * SCO links are not counted for 100%
                         * bandwidth. The rules allow for replacement
                         * of SCO packets with ACL packets.
                         * (They do not allow for the replacement
                         * of eSCO packets with ACL packets).
                         */
                        link_slot_occupancy = 0;
                    }
                    else
                    {
                        link_slot_occupancy = LMscoctr_GetSlotOccupancy(p_esco_link->tx_packet_type)+
                            LMscoctr_GetSlotOccupancy(p_esco_link->rx_packet_type);
                    }

                    link_slot_occupancy *= (largest_t_esco/p_esco_link->t_sco);
                    sum_link_slot_occupancy += link_slot_occupancy;
                } /* end for */
            } /* end else */


            /*
             * If the bandwidth would be completely filled by
             * reserved slots if this new connection was added,
             * return -1.
             */
            if(sum_link_slot_occupancy >= largest_t_esco)
                d_esco = -1;
        }
    }

    return d_esco;
}

/***********************************************************************
 * FUNCTION :- LMeScoctr_Get_Tx_Packet_Type_in_Slots
 *
 * DESCRIPTION :- 
 * Returns the size (in slots) of the current Slave to Master packet type. 
 ***********************************************************************/
u_int8 LMeScoctr_Get_Tx_Packet_Type_in_Slots(const t_sco_info *p_sco_link)
{
    /*
     * POLL/NULL/EV3/2-EV2/3-EV3: 1 slot
     * EV4/EV5/2-EV5/3-EV5:       3 slots
     */
    if (is_eSCO_Packet_3_Slot(p_sco_link->tx_packet_type))
    {
        return 3;
    }
    else
    {
        return 1;
    }
}


/***********************************************************************
 * FUNCTION :- LMeScoctr_Get_Rx_Packet_Type_in_Slots
 *
 * DESCRIPTION :- 
 * Returns the size (in slots) of the current Slave to Master packet type. 
 ***********************************************************************/
u_int8 LMeScoctr_Get_Rx_Packet_Type_in_Slots(const t_sco_info *p_sco_link)
{
    /*
     * POLL/NULL/EV3/2-EV2/3-EV3: 1 slot
     * EV4/EV5/2-EV5/3-EV5:       3 slots
     */
    if (is_eSCO_Packet_3_Slot(p_sco_link->rx_packet_type))
    {
        return 3;
    }
    else
    {
        return 1;
    }
}


/***********************************************************************
 * FUNCTION :- LMeScoctr_Get_Number_eSCO_Connections
 *
 * DESCRIPTION :- 
 * Returns number of eSCOs active 
 ***********************************************************************/
u_int8 LMeScoctr_Get_Number_eSCO_Connections(void)
{
    return _LMeScoctr_number_escos_used;
}


/***********************************************************************
 *
 * FUNCTION :- LMeScoctr_Are_Renegotiated_Params_Available
 *
 * DESCRIPTION :- 
 * Returns true if the eSCO link parameters have been renegotiated.
 * The currently used eSCO params are replaced with the proposed 
 * eSCO values. This happens in a synchronised manner to facilitate
 * the smooth transition from the old parameters to the new parameters.
 * 
 ***********************************************************************/
boolean LMeScoctr_Are_Renegotiated_Params_Available(t_sco_info *p_esco_link)
{
    if (p_esco_link->renegotiated_params_available)
    {
        p_esco_link->renegotiated_params_available = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/***********************************************************************
 *
 * FUNCTION :- LMeScoctr_Commit_to_New_Synchronous_Parameters
 *
 * DESCRIPTION :- 
 * The currently used eSCO params are replaced with the proposed 
 * eSCO values. This routine does not synchronise with a pre-existing
 * connection.
 * 
 ***********************************************************************/
void LMscoctr_Commit_to_New_Synchronous_Parameters(t_sco_info *p_esco_link)
{
    t_esco_descr *p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();

    p_esco_link->timing_ctrl = p_esco_descr->timing_ctrl_flag;
    p_esco_link->d_sco = p_esco_descr->d_esco;
    p_esco_link->t_sco = p_esco_descr->t_esco;
    p_esco_link->w_esco = p_esco_descr->w_esco;

    p_esco_link->tx_packet_type = p_esco_descr->tx_pkt_type;
    p_esco_link->rx_packet_type = p_esco_descr->rx_pkt_type;
#if 0 // GF 16 Nov 2009
    if(p_esco_link->tx_packet_type == EV3)
        p_esco_link->tx_packet_type = HV3;
#endif
    if(p_esco_link->rx_packet_type == EV3)
        p_esco_link->rx_packet_type = HV3;

    p_esco_link->tx_packet_length = p_esco_descr->tx_pkt_len;
    p_esco_link->rx_packet_length = p_esco_descr->rx_pkt_len;

    p_esco_link->lm_air_mode = LMscoctr_Map_AirMode_HCI2LMP( (u_int8)(p_esco_descr->voice_setting & 3));
    p_esco_link->lm_sco_handle = p_esco_descr->lm_sco_handle;
    p_esco_link->esco_lt_addr = p_esco_descr->esco_lt_addr;

    /*
     * Update the voice settings to take account of the
     * actual air mode negotiated with the peer.
     */
    p_esco_link->voice_setting = p_esco_link->voice_setting & (~3);
    p_esco_link->voice_setting |= LMscoctr_Map_AirMode_LMP2HCI(p_esco_link->lm_air_mode);

    /*
     * Update the sco cfg with the actual voice setting used on
     * the link.
     */
    p_esco_link->sco_cfg_hardware = LC_Get_Hardware_SCO_CFG(
        p_esco_link->voice_setting);
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Map_eSCO_Pkt_Type_LMP2HCI
 *
 * DESCRIPTION :-
 * Determines the HCI esco packet types (from the LMP packet type).
 *
 ***********************************************************************/
u_int16 LMscoctr_Map_eSCO_Pkt_Type_LMP2HCI(
    u_int8 lm_packet_type)
{
    u_int16 hc_sco_packet_type = HCI_SYN_EV3;
	int i;

    for (i=0;i<7;i++)
	{
		if(eSCO_packet_mapping[0][i] == lm_packet_type)
			return eSCO_HCI_packet_mapping[i];
	}
    return hc_sco_packet_type;
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Map_eSCO_Pkt_Type_BB2HCI
 *
 * DESCRIPTION :-
 * Determines the HCI esco packet types (from the BB packet type).
 *
 ***********************************************************************/
u_int16 LMscoctr_Map_eSCO_Pkt_Type_BB2HCI(
    u_int8 bb_packet_type)
{
    u_int16 hc_sco_packet_type = HCI_SYN_EV3;
	int i;

    for (i=0;i<7;i++)
	{
		if(eSCO_packet_mapping[1][i] == bb_packet_type)
			return eSCO_HCI_packet_mapping[i];
	}
    return hc_sco_packet_type;
}

/*************************************************************************
 * FUNCTION :- LMscoctr_Get_eSCO_BB_Packet_From_LMP_Type
 * 
 * DESCRIPTION 
 *************************************************************************/
t_packet LMscoctr_Get_eSCO_BB_Packet_From_LMP_Type(u_int8 lmp_pkt_type)
{
    t_packet bb_pkt_type = NULLpkt;
	int i;

    for (i=0;i<7;i++)
	{
		if(eSCO_packet_mapping[0][i] == lmp_pkt_type)
			return eSCO_packet_mapping[1][i];
	}

    return bb_pkt_type;
}

/*************************************************************************
 * FUNCTION :- LMscoctr_Get_eSCO_LMP_Type_From_BB_Packet
 * 
 * DESCRIPTION 
 *************************************************************************/
u_int8 LMscoctr_Get_eSCO_LMP_Type_From_BB_Packet(t_packet bb_pkt_type)
{
    u_int8 lmp_pkt_type = LM_SCO_PACKET_POLLNULL;
	int i;

    for (i=0;i<7;i++)
	{
		if(eSCO_packet_mapping[1][i] == bb_pkt_type)
			return eSCO_packet_mapping[0][i];
	}

    return lmp_pkt_type;
}

/********************************************************************
 * FUNCTION :- LMscoctr_Extract_eSCO_Parameters_From_LMP_PDU
 * 
 * DESCRIPTION 
 * This function is responsible for Extacting the eSCO parameters from 
 * an LMP_eSCO_Link_Req PDU. 
 *
 * NOTE :- Additional parameter checking has yet to be performed here.
 * The master has to perform checks on the supplied paramters
 * Reasons for failure (included in the return LMP_NOT_ACCEPTED
 *    0x0a: Max number of SCO connections to a device.
 *    0x1b: SCO offset rejected.
 *    0x1c: SCO interval rejected
 *    0x1d: SCO air mode rejected
 *
 * The LMP_SCO_link_req parameters in p_payload are:
 *  SCO handle 
 *  timing control flags 
 *  D sco
 *  T sco
 *  SCO packet
 *  air mode
 *************************************************************************/

void LMscoctr_Extract_eSCO_Parameters_From_LMP_PDU(
    t_p_pdu p_pdu)
{
    t_esco_descr* p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
	u_int8 _derive_d_esco_list = 0;

    p_esco_descr->lm_sco_handle = p_pdu[0];
    p_esco_descr->esco_lt_addr = p_pdu[1];

	/*
	 * If the new parameter signature is not the same
	 * as the current parameter signature, then a new d esco
	 * list of values must be derived.
	 */
	if ( (p_esco_descr->t_esco != p_pdu[4]) ||
		 (p_esco_descr->w_esco != p_pdu[5]) ||
		 (p_esco_descr->tx_pkt_type != LMscoctr_Get_eSCO_BB_Packet_From_LMP_Type(p_pdu[6])) ||
		 (p_esco_descr->rx_pkt_type != LMscoctr_Get_eSCO_BB_Packet_From_LMP_Type(p_pdu[7])) )
	{
		_derive_d_esco_list = 1;
	}

    p_esco_descr->timing_ctrl_flag = ((p_pdu[2] & 0x2) >> 1);
    p_esco_descr->d_esco = p_pdu[3];

    p_esco_descr->t_esco = p_pdu[4];

    p_esco_descr->w_esco = p_pdu[5];

    p_esco_descr->tx_pkt_type = (t_packet)LMscoctr_Get_eSCO_BB_Packet_From_LMP_Type(p_pdu[6]);
    p_esco_descr->rx_pkt_type = (t_packet)LMscoctr_Get_eSCO_BB_Packet_From_LMP_Type(p_pdu[7]);

    p_esco_descr->rx_pkt_len = HCIparam_Get_Uint16(&(p_pdu[8]));
    p_esco_descr->tx_pkt_len = HCIparam_Get_Uint16(&(p_pdu[10]));

    p_esco_descr->voice_setting &= ~3;
    p_esco_descr->voice_setting |= LMscoctr_Map_AirMode_LMP2HCI(p_pdu[12]);

    p_esco_descr->negotiation_state = p_pdu[13];

	if (_derive_d_esco_list)
	{
		t_sco_info *p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
		/* regenerate d_esco_calculator_bitmap */
		LMscoctr_Derive_D_eSCO(p_esco_link, p_esco_descr->t_esco, p_esco_descr->w_esco,
			p_esco_descr->timing_ctrl_flag, p_esco_descr->tx_pkt_type,
			p_esco_descr->rx_pkt_type);
	}
}   


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Validate_Admission
 * 
 * DESCRIPTION
 *  This function validates the admisison of a new eSCO
 *  connection.
 *************************************************************************/
t_error LMscoctr_Validate_Admission(u_int32 tx_bandwidth)
{
    /* TODO */
    return NO_ERROR;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Detect_Transaction_Collisions
 * 
 * DESCRIPTION
 *  This function detects transaction collisions.
 *
 *************************************************************************/
t_error LMscoctr_Detect_Transaction_Collisions()
{
    /* TODO */
    return NO_ERROR;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Validate_eSCO_Features
 * 
 * DESCRIPTION
 *  This function checks the the air mode and esco packet
 *  types are both in the device's feature mask.
 *
 *************************************************************************/
t_error LMscoctr_Validate_eSCO_Features(u_int8 air_mode, 
    u_int16* esco_packet_types, u_int8* feature_list)
{
    u_int16 supported_packet_types = 0;
    
    if(mFeat_Check_eSco_Link(feature_list))
    {
        supported_packet_types |= HCI_SYN_EV3;
    }
    else
    {
        return UNSUPPORTED_FEATURE;
    }

    if(mFeat_Check_EV4_Packets(feature_list))
    {
        supported_packet_types |= HCI_SYN_EV4;
    }

    if(mFeat_Check_EV5_Packets(feature_list))
    {
        supported_packet_types |= HCI_SYN_EV5;
    }

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)

    if(mFeat_Check_EDR_eSco_2MBPS_Mode(feature_list))
    {
        supported_packet_types |= HCI_SYN_2EV3;

        if(mFeat_Check_3_Slot_EDR_eSco_Packets(feature_list))
           supported_packet_types |= HCI_SYN_2EV5;
    }

    if(mFeat_Check_EDR_eSco_3MBPS_Mode(feature_list))
    {
        supported_packet_types |= HCI_SYN_3EV3;

        if(mFeat_Check_3_Slot_EDR_eSco_Packets(feature_list))
           supported_packet_types |= HCI_SYN_3EV5;
    }

#endif

    if(*esco_packet_types != 0) /* if ! poll/null */
    {
        if(supported_packet_types & *esco_packet_types) /* if any packets are available */
        {
            /* set available packet types */
            *esco_packet_types = (supported_packet_types & *esco_packet_types);
        }
        else
        {
            return UNSUPPORTED_FEATURE;
        }
    }

    if(air_mode == 0) /* u-law */
    {
        if(!mFeat_Check_U_Law(feature_list))
        {
            return UNSUPPORTED_FEATURE;
        }
    }

    if(air_mode == 1) /* A-law */
    {
        if(!mFeat_Check_A_Law(feature_list))
        {
            return UNSUPPORTED_FEATURE;
        }
    }

    if(air_mode == 2) /* CVSD */
    {
        if(!mFeat_Check_CVSD(feature_list))
        {
            return UNSUPPORTED_FEATURE;
        }
    }

    if(air_mode == 3) /* Transparent */
    {
        if(!mFeat_Check_Transparent_Sync_Data(feature_list))
        {
            return UNSUPPORTED_FEATURE;
        }
    }

    return NO_ERROR;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Validate_Local_eSCO_Features
 * 
 * DESCRIPTION
 *  This function checks the the air mode and esco packet
 *  types are both in the local device's feature mask.
 *
 *************************************************************************/
t_error LMscoctr_Validate_Local_eSCO_Features(u_int8 air_mode, 
    u_int16* esco_packet_types)
{
    t_error result = LMscoctr_Validate_eSCO_Features(air_mode, esco_packet_types, 
        SYSconfig_Get_LMP_Features_Ref());

    if(result != NO_ERROR)
        result = UNSUPPORTED_FEATURE;

    return result;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Validate_Remote_eSCO_Features
 * 
 * DESCRIPTION
 *  This function checks the the air mode and esco packet
 *  types are both in the remote device's feature mask.
 *
 *************************************************************************/
t_error LMscoctr_Validate_Remote_eSCO_Features(t_lmp_link* p_link, 
    u_int8 air_mode, u_int16 *esco_packet_types)
{
    t_error result = LMscoctr_Validate_eSCO_Features(air_mode, esco_packet_types, p_link->remote_features);

    if(result != NO_ERROR)
        result = UNSUPPORTED_REMOTE_FEATURE;

    return result;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_RangeCheck_Host_eSCO_Parameters
 * 
 * DESCRIPTION
 *  This function checks that the proposed eSCO parameters are
 *  within the host controller's allowable range.
 *
 *************************************************************************/
t_error LMscoctr_RangeCheck_Host_eSCO_Parameters(u_int16 max_latency, u_int8
        retransmission_effort, u_int16 esco_packet_types)
{
    t_error error_level = NO_ERROR;

    if(max_latency < 4)
        error_level = INVALID_HCI_PARAMETERS;

    if((retransmission_effort > 2) && (retransmission_effort != 0xff))
        error_level = INVALID_HCI_PARAMETERS;

    if( (esco_packet_types & HCI_SYN & ~HCI_SCO_BR) == 0 )
        error_level = INVALID_HCI_PARAMETERS;

    return error_level;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_IsValid_eSCO_Packet_Type
 * 
 * DESCRIPTION
 *  This function checks that the lm_pkt_type is
 *  a valid eSCO packet type.
 *
 *************************************************************************/
boolean LMscoctr_IsValid_eSCO_Packet_Type(u_int8 lm_pkt_type)
{
	int i;
    if(lm_pkt_type == LM_SCO_PACKET_POLLNULL)
        return TRUE;
    for (i=0;i<=6;i++)
	{
		if(eSCO_packet_mapping[0][i] == lm_pkt_type)
			return TRUE;
	}

    return FALSE;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_IsValid_eSCO_Packet_Length
 * 
 * DESCRIPTION
 *  This function checks that the lm_pkt_length is
 *  within range for the eSCO packet type proposed.
 *
 *************************************************************************/
boolean LMscoctr_IsValid_eSCO_Packet_Length(u_int8 lm_pkt_type, u_int8* len)
{
    u_int16 length = LMutils_Get_Uint16(len);

    t_packet bb_pkt_type = LMscoctr_Get_eSCO_BB_Packet_From_LMP_Type(lm_pkt_type);

    return (length <= SYSconst_Get_Packet_Length(bb_pkt_type));
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_RangeCheck_Peer_eSCO_Parameters
 * 
 * DESCRIPTION
 *  This function range checks the peer's eSCO parameters for
 *  sanity.
 *
 *************************************************************************/
t_error LMscoctr_RangeCheck_Peer_eSCO_Parameters(t_lmp_link* p_link, u_int8* p_pdu)
{
    u_int8 low_limit = 0;

    /*
     * Peer allowed to use any eSCO handle.
     */

    /*
     * eSCO LT_ADDR must be between 0 -- 7 if IUT is master, else 1--7
     */
    low_limit = (p_link->role == MASTER) ? 0 : 1;
    if((p_pdu[1]<low_limit) || (p_pdu[1]>7) )
        return INVALID_LMP_PARAMETERS;

    /*
     * eSCO LT_ADDR cannot be the same as ACL/SCO LT_ADDR
     * Guard and reject
     */
    if(p_link->role == SLAVE)
    {
        if(p_pdu[1] == DL_Get_Am_Addr(DL_Get_Device_Ref(p_link->device_index)))
            return INVALID_LMP_PARAMETERS;
    }

    /*
     * Timing control flags must be between 0 and 7.
     */
    if(p_pdu[2] > 7)
        return INVALID_LMP_PARAMETERS;

    /*
     * At conformance testing with Bite tester,
     * bite tester sets the third bit of the
     * timing control flag, even though this
     * field has no meaning for eSCO. To work
     * better with this device, remove all
     * bits except the two bits of interest
     * in this parameter.
     */
    if(p_pdu[2] > 3)
        p_pdu[2] &= 3;

    /*
     * D eSCO must be between 0 and 254.
     * The D eSCO parameter must be even.
     */
    if(p_pdu[3]>254)
        return INVALID_LMP_PARAMETERS;
    if(p_pdu[3] & 1)
        return INVALID_LMP_PARAMETERS;

    /*
     * T eSCO must be beteen 2 and 254.
     * The T eSCO parameter must be even
     */
    if((p_pdu[4]<2) || (p_pdu[4]>254) )
        return INVALID_LMP_PARAMETERS;
    if(p_pdu[4] & 1)
        return INVALID_LMP_PARAMETERS;
    
    /*
     * W eSCO must be between 0 and 254.
     * The W eSCO parameter must be even
     */
    if(p_pdu[5] & 1)
        return INVALID_LMP_PARAMETERS;

    /*
     * The eSCO packet type (M->S) must be either 0,7,c,d.
     */
    if(!LMscoctr_IsValid_eSCO_Packet_Type(p_pdu[6]))
        return INVALID_LMP_PARAMETERS;

    /*
     * The eSCO packet type (S->M) must be either 0,7,c,d.
     */
    if(!LMscoctr_IsValid_eSCO_Packet_Type(p_pdu[7]))
        return INVALID_LMP_PARAMETERS;

    /*
     * The eSCO packet length (M->S) must fall within
     * the allowed values for the selected packet type.
     */
    if(!LMscoctr_IsValid_eSCO_Packet_Length(p_pdu[6], &(p_pdu[8])))
        return INVALID_LMP_PARAMETERS;

    /*
     * The eSCO packet length (S->M) must fall within
     * the allowed values for the selected packet type.
     */
    if(!LMscoctr_IsValid_eSCO_Packet_Length(p_pdu[7], &(p_pdu[10])))
        return INVALID_LMP_PARAMETERS;

    /*
     * The air-mode must be between 0 and 3
     */
    if(p_pdu[12] > 3)
        return INVALID_LMP_PARAMETERS;

    /*
     * The negotiation state must be between 0 and 4
     */
    if(p_pdu[13] > 4)
        return INVALID_LMP_PARAMETERS;

    return NO_ERROR;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Is_An_LMP_Parameter_Set
 * 
 * DESCRIPTION
 *  This function determines if an LMP parameter set proposed
 *  by the peer device is acceptable based on the host constraints.
 *
 *************************************************************************/
int LMscoctr_Is_An_LMP_Parameter_Set(u_int8 t_esco, u_int8 w_esco,
    t_packet pkt_type, u_int16 pkt_len)
{
	t_sco_info *p_esco_link;
	u_int8 reserved_slots;

	p_esco_link = LMscoctr_Get_Topical_eSCO_Link();

	reserved_slots = is_eSCO_Packet_3_Slot(pkt_type)?(6):(2);
	if ( ((u_int32)(t_esco + reserved_slots + w_esco)*625 <= (u_int32)p_esco_link->max_latency*1000)
		&& (t_esco >= reserved_slots + w_esco) )
	{
		if (((LMscoctr_Map_eSCO_Pkt_Type_BB2HCI(pkt_type)) & (p_esco_link->esco_packet_types)) != 0)
		{
            if(((u_int32)(p_esco_link->tx_bandwidth)*t_esco) == (u_int32)(pkt_len*1600))
			{
				u_int8 retransmission_effort = p_esco_link->retransmission_effort;
               if ((w_esco == 0) && ((retransmission_effort==1) || (retransmission_effort==2)))
				   return 0;
			   else
				   return 1;
			}
		}
	}

    return 0; 
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Get_LMP_Parameter_Set
 * 
 * DESCRIPTION
 *  This function returns an alternative LMP paramater set based on the
 * ongoing negotiation with teh peer device.
 *
 *************************************************************************/
static t_lm_esco_pkt_type_desc lm_pkt[LM_SCO_CTR_NUM_PKT_TYPES] = {
/* hci_type, bb_type, max_len, w_esco_slots, min_t_esco */
	{HCI_SYN_EV3,		EV3,  30, 2, 4},
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	{HCI_SYN_2EV3, EDR_2EV3,  60, 2, 4},
	{HCI_SYN_3EV3, EDR_3EV3,  90, 2, 4},
#endif
	{HCI_SYN_EV4,		EV4, 120, 6, 8},
	{HCI_SYN_EV5,		EV5, 180, 6, 8},
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	{HCI_SYN_2EV5, EDR_2EV5, 360, 6, 8},
	{HCI_SYN_3EV5, EDR_3EV5, 540, 6, 8}
#endif
	 };

boolean LMscoctr_Get_LMP_Parameter_Set(t_esco_descr* p_esco_descr)
{
	u_int8 t_esco, w_esco; //d_esco;
	u_int32 len;
	int i;
	t_sco_info *p_esco_link;
	u_int32 tx_bandwidth;
	u_int8 max_latency;
    u_int8 retransmission_effort;
    u_int16 esco_packet_types;
	//t_lmp_link *p_link;
	//u_int8 timing_ctrl_flag;

	p_esco_link = LMscoctr_Get_Topical_eSCO_Link();

	tx_bandwidth = p_esco_link->tx_bandwidth;
	max_latency = p_esco_link->max_latency;
    retransmission_effort = p_esco_link->retransmission_effort;
    esco_packet_types = p_esco_link->esco_packet_types;
	
	//p_link = LMscoctr_Find_ACL_Link_From_SCO_Link(p_esco_link);
	
	//if(0 != p_link)
	//{
	  //  timing_ctrl_flag = mLMpol_Get_Timing_Ctrl(p_link->device_index);
	//}

	if (ESCO_NGT_INIT == p_esco_descr->negotiation_state)
	{ /* peer or local has initiated negotiation, check p_esco_link */
		LMscoctr_Initialise_LMP_Paramater_Sets();

		if ((p_esco_link->tx_bandwidth == 0) || (p_esco_link->rx_bandwidth == 0))
			return FALSE;
	}
	else if (ESCO_NGT_LAT_VIOL == p_esco_descr->negotiation_state)
	{ /* peer has indicated a latency violation, check latency */

		max_latency = LMscoctr_Calculate_Latency_In_uSec(p_esco_descr->t_esco,
		     p_esco_descr->w_esco, p_esco_descr->tx_pkt_type, p_esco_descr->rx_pkt_type);

		if (max_latency < p_esco_link->max_latency)
			p_esco_link->max_latency = max_latency; /* assign new max latency */
	}
    else if (ESCO_NGT_UNSUPPORTED_FEATURE == p_esco_descr->negotiation_state)
	{ /* peer has indicated unsupported feature, check tx/rx pkt types */

		u_int16 esco_pkt_type = lm_pkt[LMscoctr_last_tx_lm_pkt_index].hci_type;

		if ( (LMscoctr_Map_eSCO_Pkt_Type_BB2HCI(p_esco_descr->tx_pkt_type) != esco_pkt_type)
			|| (LMscoctr_Map_eSCO_Pkt_Type_BB2HCI(p_esco_descr->rx_pkt_type) != esco_pkt_type))
		{
            esco_packet_types &= ~esco_pkt_type;
			p_esco_link->esco_packet_types = esco_packet_types; /* remove unsupported pkt type */
		}
	}
	else if(ESCO_NGT_RS_VIOL == p_esco_descr->negotiation_state)
	{ /* peer has indicated reserved slot violations, eliminate t_esco option */
	    lm_pkt[LMscoctr_last_tx_lm_pkt_index].max_len--;
	}

	/* recalculated after negotiation state specific adjustments - */
	{
		for (i = 0; i < mNum_Elements(lm_pkt); i++)
		{
			if (0 == (esco_packet_types & lm_pkt[i].hci_type))
			    continue; /* packet type not selected */

			if (0xFF == retransmission_effort) /* don't care */
				w_esco = 0;
			else /* preferred w_esco based packet size and num retransmissions */
			    w_esco = (retransmission_effort)*(lm_pkt[i].w_esco_slots);

			/*
			 * Going from the longest to the shortest length, check if the t_esco can meet the bandwidth.
			 */
			for (len = lm_pkt[i].max_len; len > 0; len--)
			{
				if( ( (len*1600) % tx_bandwidth) == 0)
				{
					/*
					 * If the t_esco passes the bandwidth check, check that the latency is acceptable and add to set.
					 */
					t_esco = (len*1600)/tx_bandwidth;

					if (t_esco < lm_pkt[i].min_t_esco)
						continue; /* require larger packet sizes - try other packet type */

					//for (j = 0; j < _LMscoctr_num_rs_t_escos; j++)
					//{
					//	if (_LMscoctr_rs_t_esco_list[_LMscoctr_num_rs_t_escos] == t_esco)
					//		continue; /* this t_esco causes RS violations on peer */
					//}
						
					if (((t_esco % 2)==0) && (t_esco >= lm_pkt[i].w_esco_slots + w_esco))
					{
						if(((u_int32)(t_esco + lm_pkt[i].w_esco_slots + w_esco)*625) <= (u_int32)(max_latency*1000))
						{
							//d_esco = LMscoctr_Derive_D_eSCO(p_esco_link, t_esco, w_esco, timing_ctrl_flag, 
								//lm_pkt[i].bb_type, lm_pkt[i].bb_type);
							
							//if (-1 != d_esco)
							{
								p_esco_descr->t_esco = t_esco;
								p_esco_descr->w_esco = w_esco;
								p_esco_descr->tx_pkt_len = (t_esco * p_esco_link->tx_bandwidth)/1600;
								p_esco_descr->rx_pkt_len = (t_esco * p_esco_link->rx_bandwidth)/1600;
								p_esco_descr->tx_pkt_type = lm_pkt[i].bb_type;
								p_esco_descr->rx_pkt_type = lm_pkt[i].bb_type;
								//p_esco_descr->d_esco = d_esco;
								//p_esco_descr->timing_ctrl_flag = timing_ctrl_flag;
								lm_pkt[i].max_len = len;
								LMscoctr_last_tx_lm_pkt_index = i;
								return TRUE;
							}
						}
					}
				}
			}
		}
	}

    return FALSE;
}

/********************************************************************
 *
 * FUNCTION :- LMscoctr_Initialise_LMP_Paramater_Sets
 * 
 * DESCRIPTION
 *  Some of the values in the lm_pkt[] arrays are reduced within the
 * LMscoctr_Get_LMP_Parameter_Set. This restores initial values.
 *
 *************************************************************************/
void LMscoctr_Initialise_LMP_Paramater_Sets(void)
{
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
		lm_pkt[0].max_len = 30; // EV3
		lm_pkt[1].max_len = 60; // EDR_2EV3
		lm_pkt[2].max_len = 90; // EDR_3EV3
		lm_pkt[3].max_len = 120; // EV4
		lm_pkt[4].max_len = 180; // EV5
		lm_pkt[5].max_len = 360; // EDR_2EV5
		lm_pkt[6].max_len = 540; // EDR_3EV5
#else
		lm_pkt[0].max_len = 30; // EV3
		lm_pkt[1].max_len = 120; // EV4
		lm_pkt[2].max_len = 360; // EV5
#endif
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Calculate_Latency_In_uSec
 * 
 * DESCRIPTION
 *  This function calculates the latency (in usec) required
 *  by a set of LMP parameters.
 *
 *************************************************************************/
u_int16 LMscoctr_Calculate_Latency_In_uSec(u_int8 t_esco, u_int8 w_esco, 
    t_packet tx_pkt_type, t_packet rx_pkt_type)
{
    u_int16 latency_in_us;

    latency_in_us = (t_esco + w_esco 
        + ((is_eSCO_Packet_3_Slot(tx_pkt_type)<<1)+1)
        + ((is_eSCO_Packet_3_Slot(rx_pkt_type)>>1)+1) ) * 625;

    return latency_in_us;
}

/********************************************************************
 *
 * FUNCTION :- LMscoctr_CanMap_eSCO_Parameters
 * 
 * DESCRIPTION
 * Check that the host controller has sufficient resources
 * free to accomodate the new request from the host.
 *
 *************************************************************************/
t_error LMscoctr_CanMap_eSCO_Parameters(u_int32 tx_bandwidth, 
        u_int32 rx_bandwidth, u_int16 max_latency, 
        u_int8 retransmission_effort, u_int16 esco_packet_types,
        t_lmp_link* p_link)
{
	u_int8 t_esco, w_esco;
	s_int16 d_esco;
	u_int16 len;
	int i;
	u_int8 timing_ctrl_flag=0;

	if(LMscoctr_Get_Number_SYN_Connections() > PRH_BS_CFG_SYS_MAX_SYN_LINKS)
        return MAX_NUM_SCO;
	
	if(0 != p_link)
	{
	    timing_ctrl_flag = mLMpol_Get_Timing_Ctrl(p_link->device_index);
	}

	LMscoctr_Initialise_LMP_Paramater_Sets();

	for (i = 0; i < mNum_Elements(lm_pkt); i++)
	{
		if (0 == (esco_packet_types & lm_pkt[i].hci_type))
		    continue; /* packet type not selected */

		if (0xFF == retransmission_effort) /* don't care */
			w_esco = 0;
		else /* preferred w_esco based packet size and num retransmissions */
		    w_esco = (retransmission_effort)*(lm_pkt[i].w_esco_slots);

		/*
		 * Going from the longest to the shortest length, check if the t_esco can meet the bandwidth.
		 */
		for (len = lm_pkt[i].max_len; len > 0; len--)
		{
			if( ( (len*1600) % tx_bandwidth) == 0)
			{
				/*
				 * If the t_esco passes the bandwidth check, check that the latency is acceptable and add to set.
				 */
				t_esco = (len*1600)/tx_bandwidth;

				if (t_esco < lm_pkt[i].min_t_esco)
					continue; /* require larger packet sizes - try other packet type */
						
				if (((t_esco % 2)==0) && (t_esco >= lm_pkt[i].w_esco_slots + w_esco))
				{
					if(((t_esco + lm_pkt[i].w_esco_slots + w_esco)*625) <= (max_latency*1000))
					{
						d_esco = LMscoctr_Derive_D_eSCO(LMscoctr_Get_Topical_eSCO_Link(), t_esco, w_esco, timing_ctrl_flag, 
							lm_pkt[i].bb_type, lm_pkt[i].bb_type);
							
						if (-1 != d_esco)
						{
							return NO_ERROR;
						}
					}
				}
			}
		}
	}

    return EC_QOS_UNACCEPTABLE_PARAMETER;
}

/********************************************************************
 *
 * FUNCTION :- LMscoctr_Validate_Host_eSCO_Parameters
 * 
 * DESCRIPTION
 * Check that the host parameters are sane.
 *
 *************************************************************************/
t_error LMscoctr_Validate_Host_eSCO_Parameters(u_int16 handle,
        u_int32 tx_bandwidth, u_int32 rx_bandwidth,
        u_int16 max_latency, u_int16 voice_setting, u_int8 retransmission_effort, 
        u_int16* esco_packet_types)
{
    t_error status = NO_ERROR;
    t_lmp_link* p_link = 0;
	u_int8 lm_air_mode = LMscoctr_Map_AirMode_HCI2LMP((u_int8)(voice_setting & 3));
    u_int8 air_mode = (u_int8)(lm_air_mode & 3);
    t_sco_info* p_esco_link = 0;

    /*
     * Check the handle is in either the ACL handle map
     * or the eSCO handle map.
     */
    p_link = LMaclctr_Find_Handle(handle);

    p_esco_link = LMscoctr_Find_SCO_By_Connection_Handle(handle);
    if (p_esco_link)
	{
		LMscoctr_Set_Topical_eSCO_Link(p_esco_link, ESCO_RECONFIGURE);
	}

	if(!p_link)
    {
        if(p_esco_link)
        {
            p_link = LMscoctr_Find_ACL_Link_From_SCO_Link(LMscoctr_Find_SCO_By_Connection_Handle(handle));
        }
    }
    if(!p_link)
    {
        return NO_CONNECTION;
    }


    /*
     * Check parameters to ensure they are comprehensible
     */
    status = LMscoctr_RangeCheck_Host_eSCO_Parameters(max_latency, 
        retransmission_effort, *esco_packet_types);
    if(NO_ERROR != status)
        return status;

    /* 
     * Check local features (and remove unsuitable eSCO packet types) 
     */
    status = LMscoctr_Validate_Local_eSCO_Features(air_mode, esco_packet_types);
    if(NO_ERROR != status)
        return status;
    
    /* 
     * Check remote features (and remove unsuitable eSCO packet types) 
     */
    status = LMscoctr_Validate_Remote_eSCO_Features(p_link, air_mode, esco_packet_types);
    if(NO_ERROR != status)
        return status;

    /*
     * Validate that the local device can map the
     * HCI air mode to the requested air-mode.
     * 
     */
    if(!LC_Is_Valid_SCO_Conversion(voice_setting))
        return INVALID_HCI_PARAMETERS;

    /*
     * Check parameters to ensure that at least one set of LMP
     * parameters can be matched to them
     */
    status = LMscoctr_CanMap_eSCO_Parameters(tx_bandwidth, rx_bandwidth, 
        max_latency, retransmission_effort, *esco_packet_types, p_link);

    return status;
}


/********************************************************************
 *
 * FUNCTION :- LMscoctr_Validate_Peer_eSCO_Parameters
 * 
 * DESCRIPTION
 * Check that the peer eSCO parameters are sane.
 *
 *************************************************************************/
t_error LMscoctr_Validate_Peer_eSCO_Parameters(t_lmp_link* p_link,
    u_int8* p_pdu)
{
    t_error status = NO_ERROR;
    u_int8 air_mode = p_pdu[12];
    u_int16 esco_packet_types = 0;
    
    esco_packet_types = LMscoctr_Map_eSCO_Pkt_Type_LMP2HCI(p_pdu[6]);
    esco_packet_types |= LMscoctr_Map_eSCO_Pkt_Type_LMP2HCI(p_pdu[7]);

    /*
     * Check parameters to ensure they are comprehensible
     */
    status = LMscoctr_RangeCheck_Peer_eSCO_Parameters(p_link, p_pdu);
    if(NO_ERROR != status)
        return status;

    /* 
     * Check local features (and remove unsuitable eSCO packet types) 
     */
    status = LMscoctr_Validate_Local_eSCO_Features(air_mode, &esco_packet_types);
    if(NO_ERROR != status)
        return status;

    /* 
     * Check remote features (and remove unsuitable eSCO packet types) 
     */
    status = LMscoctr_Validate_Remote_eSCO_Features(p_link, air_mode, &esco_packet_types);
    if(NO_ERROR != status)
        return status;

    return status;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Get_Topical_eSCO_Link()
 *
 * Note: This function only expects one eSCO link to be in the
 * process of being setup/manipulated at any one time.
 *
 * Only two routines should be able to set the
 * "Topical eSCO Link". These are:
 *
 * 1) The incoming LMP_eSCO_link_req handler once the PDU has been
 *    handled to the point where it is definitely a new
 *    LMP transaction that has started.
 * 2) The incoming HCI_Setup_Synchronous_Connection handler
 *    once the PDU has been handled to the point where a new
 *    LMP transaction that has started.
 *
 * I am using the Topical eSCO link to represent the eSCO link
 * which has most recently been brought to the eSCO link
 * handlers attention.
 * 
 ***********************************************************************/
t_sco_info *LMscoctr_Get_Topical_eSCO_Link(void)
{
    return _p_LMscoctr_Topical_eSco_Link;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Set_Topical_eSCO_Link()
 *
 * Note: This function only expects one eSCO link to be in the
 * process of being setup/manipulated at any one time.
 *
 * Only two routines should be able to set the
 * "Topical eSCO Link". These are:
 *
 * 1) The incoming LMP_eSCO_link_req handler once the PDU has been
 *    handled to the point where it is definitely a new
 *    LMP transaction that has started.
 * 2) The incoming HCI_Setup_Synchronous_Connection handler
 *    once the PDU has been handled to the point where a new
 *    LMP transaction that has started.
 *
 * I am using the Topical eSCO link to represent the eSCO link
 * which has most recently been brought to the eSCO link
 * handlers attention.
 * 
 ***********************************************************************/
void LMscoctr_Set_Topical_eSCO_Link(t_sco_info *p_esco_link, t_esco_transaction_type type)
{
    _p_LMscoctr_Topical_eSco_Link = p_esco_link;
    _LMscoctr_Topical_eSco_Link_Transaction_Type = type;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Get_Topical_eSCO_Link_Transaction_Type()
 *
 * Determine if the topical eSCO link is being created, destroyed,
 * or being changed.
 * 
 ***********************************************************************/
t_esco_transaction_type LMscoctr_Get_Topical_eSCO_Link_Transaction_Type(void)
{
    return _LMscoctr_Topical_eSco_Link_Transaction_Type;
}


/***********************************************************************
 * FUNCTION :- LMscoctr_Find_ACL_Link_From_SCO_Link()
 *
 * Determine the LMP ACL link associated with an LMP SCO or eSCO link.
 * 
 ***********************************************************************/
t_lmp_link* LMscoctr_Find_ACL_Link_From_SCO_Link(t_sco_info* p_esco)
{
    return LMaclctr_Find_Device_Index(p_esco->device_index);
}

/***********************************************************************
 * FUNCTION :- _LMscoctr_SetDEscoCalculatorBit
 *
 * Set the specified bit in the d esco calculator matrix.
 * 
 ***********************************************************************/
void _LMscoctr_SetDEscoCalculatorBit(int bit_pos)
{
    int byte_pos;
    unsigned char bit_in_byte_pos;

    byte_pos = bit_pos/8;
    bit_in_byte_pos = bit_pos%8;

    _LMeScoctr_d_esco_calculator_bitmap[byte_pos] |=
        (1<<bit_in_byte_pos);
}


/***********************************************************************
 * FUNCTION :- _LMscoctr_ClearDEscoCalculatorBit
 *
 * Unset the specified bit in the d esco calculator matrix.
 * 
 ***********************************************************************/
void _LMscoctr_ClearDEscoCalculatorBit(int bit_pos)
{
    int byte_pos;
    unsigned char bit_in_byte_pos;

    byte_pos = bit_pos/8;
    bit_in_byte_pos = bit_pos%8;

    _LMeScoctr_d_esco_calculator_bitmap[byte_pos] &=
        ~( (1<<bit_in_byte_pos)) ;
}


/***********************************************************************
 * FUNCTION :- _LMscoctr_TestDEscoCalculatorBit
 *
 * check if the specified bit in the d esco calculator matrix.
 * is set or not.
 * 
 ***********************************************************************/
int  _LMscoctr_TestDEscoCalculatorBit(int bit_pos)
{
    int byte_pos;
    unsigned char bit_in_byte_pos;

    byte_pos = bit_pos/8;
    bit_in_byte_pos = bit_pos%8;

    if(_LMeScoctr_d_esco_calculator_bitmap[byte_pos] &
        (1<<bit_in_byte_pos))
        return 1;

    return 0;
}

/***********************************************************************
 * FUNCTION :- LMscoctr_GetSlotOccupancy
 *
 * Determine the slot occupancy of the specified baseband packet type.
 * 
 ***********************************************************************/
u_int8 LMscoctr_GetSlotOccupancy(t_packet packet_type)
{
    if((1L<<packet_type) & ((1L<<EV4)|(1L<<EV5)|(1L<<EDR_2EV5)|(1L<<EDR_3EV5)))
        return 3;

    return 1;
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Get_eSCO_Scratchpad
 *
 * Locate the scratchpad being used for negotiating the
 * topical eSCO connection.
 * 
 ***********************************************************************/
t_esco_descr* LMscoctr_Get_eSCO_Scratchpad(void)
{
    return &(LMscoctr_eSCO_Scratchpad);
}

/***********************************************************************
 * FUNCTION :- LMscoctr_Clean_eSCO_Scratchpad
 *
 * Zero the scratchpad for negotiating the topical eSCO connection
 * before use.
 * 
 ***********************************************************************/
void LMscoctr_Clean_eSCO_Scratchpad(void)
{
    hw_memset(&LMscoctr_eSCO_Scratchpad, 0, sizeof(LMscoctr_eSCO_Scratchpad));
}


/***********************************************************************
 * LMscoctr_Set_D_eSCO_and_Timing_Flag.
 *
 * This function sets up the d esco and timing control 
 * flag pair to control the timing of an eSCO connection.
 ***********************************************************************/
void LMscoctr_Set_D_eSCO_and_Timing_Flag(t_esco_descr* p_esco_descr)
{
    t_sco_info* p_esco_link = 0;
    t_lmp_link* p_link = 0;
	s_int16 d_esco;
    
    p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
    if(p_esco_link)
    {
        p_link = LMscoctr_Find_ACL_Link_From_SCO_Link(p_esco_link) ;

        if(p_link)
        {
            p_esco_descr->timing_ctrl_flag  = mLMpol_Get_Timing_Ctrl(p_link->device_index);
            d_esco = LMscoctr_Derive_D_eSCO(p_esco_link, 
                p_esco_descr->t_esco, 
                p_esco_descr->w_esco,
                p_esco_descr->timing_ctrl_flag, 
                p_esco_descr->tx_pkt_type,
                p_esco_descr->rx_pkt_type);
            if (d_esco != -1)
			{
                p_esco_descr->d_esco = (u_int8) d_esco;
			}
            else
			{
                /* TBD */
			}
        }
    }
}


/***********************************************************************
 * LMscoctr_Calculate_Bandwidth.
 *
 * This function calculates the bandwidth required for an eSCO
 * link from the LMP packet length and t_esco parameters.
 ***********************************************************************/
u_int32 LMscoctr_Calculate_Bandwidth(t_esco_descr* p_esco_descr)
{
    return ( (p_esco_descr->tx_pkt_len * 1600) / (p_esco_descr->t_esco) );
}


/***********************************************************************
 * LMscoctr_Is_D_eSCO_Acceptable.
 *
 * This function determines if the d esco proposed by the peer
 * is acceptable or if it clashes with some existing
 * SCO/eSCO link.
 ***********************************************************************/
int LMscoctr_Is_D_eSCO_Acceptable(t_esco_descr* p_esco_descr, 
    t_lmp_link* p_link)
{
    /*
     * If the timing ctrl flag proposed by the peer is unacceptable,
     * automatically return FALSE
     */
    u_int8 cur_timing_ctrl_flag = mLMpol_Get_Timing_Ctrl(p_link->device_index);


    if( (p_link->role == MASTER) && 
        ( p_esco_descr->timing_ctrl_flag != cur_timing_ctrl_flag) ) 
    {
        return 0;

    }
	else
	{
		s_int16 sD_eSCO;

		sD_eSCO =LMscoctr_Derive_D_eSCO_AtLeast(p_esco_descr->d_esco,
			0, p_esco_descr->t_esco, p_esco_descr->w_esco,
			p_esco_descr->timing_ctrl_flag,
			p_esco_descr->tx_pkt_type, p_esco_descr->rx_pkt_type);

		if (sD_eSCO >= 0)
		{
			p_esco_descr->d_esco = (u_int8)sD_eSCO;
			return 1;
		}
		else
		    return 0;
	}
}

/***********************************************************************
 * LMscoctr_Get_Alt_D_eSCO.
 *
 * Gets an alternative D_eSCO where proposed value is unacceptable, as
 * was determined to clashe with some existing SCO/eSCO link.
 ***********************************************************************/
s_int16 LMscoctr_Get_Alt_D_eSCO(t_esco_descr* p_esco_descr,
    t_lmp_link* p_link)
{
	_LMscoctr_active_d_esco_index++;

	if (_LMscoctr_active_d_esco_index < _LMscoctr_num_d_escos)
	{
		return _LMscoctr_d_esco_list[_LMscoctr_active_d_esco_index];
	}

	return -1;
}

/***********************************************************************
 * LMscoctr_Suspend_Link.
 *
 * This function suspends an eSCO link for a very short while
 * while the controller derives the list of acceptable LMP parameters
 * when changing a link.
 ***********************************************************************/
void LMscoctr_Suspend_Link(t_sco_info* p_esco_link)
{
    LMscoctr_saved_state = p_esco_link->state;
    p_esco_link->state = SCO_UNUSED;
}


/***********************************************************************
 * LMscoctr_Resume_Link.
 *
 * This function resumes an eSCO link after
 * while the controller derives the list of acceptable LMP parameters
 * when changing a link.
 ***********************************************************************/
void LMscoctr_Resume_Link(t_sco_info* p_esco_link)
{
    p_esco_link->state = LMscoctr_saved_state;

}

/***********************************************************************
 * LMscoctr_Add_D_eSCO_Element.
 *
 * This function adds a d esco value to the list of acceptable
 * d esco values for the set of LMP parametrs currently in the 
 * scratchpad.
 ***********************************************************************/
void LMscoctr_Add_D_eSCO_Element(u_int8 d_esco)
{
	// Need ensure we dont go out of bounds in _LMscoctr_d_esco_list
	if (_LMscoctr_num_d_escos < MAX_NUM_D_ESCOS_IN_LIST)
	{
		_LMscoctr_d_esco_list[_LMscoctr_num_d_escos] = d_esco;
		_LMscoctr_num_d_escos++;
	}
}

/***********************************************************************
 * LMscoctr_Clear_D_eSCO_List.
 ***********************************************************************/
void LMscoctr_Clear_D_eSCO_List(void)
{
	_LMscoctr_num_d_escos = 0;
	_LMscoctr_active_d_esco_index = 0;

	memset(&_LMscoctr_d_esco_list, 0, sizeof(_LMscoctr_d_esco_list));

	_LMscoctr_num_rs_t_escos = 0;
	memset(&_LMscoctr_rs_t_esco_list, 0, sizeof(_LMscoctr_rs_t_esco_list));
}

#endif

/***********************************************************************
 * LMscoctr_Derive_Tx_Dequeue_Length.
 *
 * This function returns the dequeue length appropriate to a
 * particular conversion technique. Normally the tx_packet_length
 * is returned but if converting from 16-bit samples to 8-bit
 * samples, then twice the tx_packet_length is more appropriate.
 ***********************************************************************/
u_int16 LMscoctr_Derive_Tx_Dequeue_Length(t_sco_info* p_esco_link)
{
    u_int16 dequeue_length = 0;
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    dequeue_length = p_esco_link->tx_packet_length;

    /*
     * If input sample size is greater than 8 bit AND
     * the input mode is linear PCM AND
     * the air mode is not transparent, need
     * 16 bits per sample on the HCI interface. Need
     * to return twice the tx_packet_length from this
     * function here.
     */
    if((8 < LMscoctr_Get_Input_Sample_Size(p_esco_link->voice_setting) ) &&
        (LINEAR_IC == LMscoctr_Get_Input_Coding(p_esco_link->voice_setting) )&& 
        (p_esco_link->lm_air_mode != LM_AIR_MODE_TRANSPARENT))
    {
        dequeue_length <<= 1;
    }
#endif
    return dequeue_length;
}

#if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)
/***********************************************************************
 * LMscoctr_Derive_Rx_Dequeue_Length.
 *
 * This function returns the dequeue length appropriate to a
 * particular conversion technique. Normally the rx_packet_length
 * is returned but if converting from 16-bit samples to 8-bit
 * samples, then twice the rx_packet_length is more appropriate.
 ***********************************************************************/
u_int16 LMscoctr_Derive_Rx_Dequeue_Length(t_sco_info* p_esco_link)
{
    u_int16 dequeue_length = 0;
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    dequeue_length = p_esco_link->rx_packet_length;

    /*
     * If input sample size is greater than 8 bit AND
     * the input mode is linear PCM AND
     * the air mode is not transparent, need
     * 16 bits per sample on the HCI interface. Need
     * to return twice the tx_packet_length from this
     * function here.
     */
    if((8 < LMscoctr_Get_Input_Sample_Size(p_esco_link->voice_setting) ) &&
        (LINEAR_IC == LMscoctr_Get_Input_Coding(p_esco_link->voice_setting) )&& 
        (p_esco_link->lm_air_mode != LM_AIR_MODE_TRANSPARENT))
    {
        dequeue_length <<= 1;
    }
#endif
    return dequeue_length;
}
#endif

/************************************************************************************
 *
 * FUNCTION: LMscoctr_Get_Input_Coding
 *
 * DESCRIPTION:
 *  Return the input coding part of the voice setting
 *
 ***********************************************************************************/
t_input_coding LMscoctr_Get_Input_Coding(u_int16 voice_setting)
{
    if(voice_setting & 0x4000)
        return CVSD_IC;
    else
        return (t_input_coding)((voice_setting >> 8) & 0x0003);
}


/************************************************************************************
 *
 * FUNCTION: LMscoctr_Get_Input_Data_Format
 *
 * DESCRIPTION:
 *  Return the input data format part of the voice setting
 *
 ***********************************************************************************/
t_input_data_format LMscoctr_Get_Input_Data_Format(u_int16 voice_setting)
{
    return (t_input_data_format)((voice_setting >> 6) & 0x0003);
}


/************************************************************************************
 *
 * FUNCTION: LMscoctr_Get_PCM_Sample_Size
 *
 * DESCRIPTION:
 *  Return the input sample size part of the voice setting
 *
 ***********************************************************************************/
t_input_sample_size LMscoctr_Get_PCM_Sample_Size(u_int16 voice_setting)
{
    return (t_input_sample_size)((voice_setting >> 5) & 0x0001);
}


/************************************************************************************
 *
 * FUNCTION: _Get_Linear_PCM_Bit_Pos
 *
 * DESCRIPTION:
 *  Return the linear PCM bit pos part of the voice setting
 *
 ***********************************************************************************/
u_int8 LMscoctr_Get_Linear_PCM_Bit_Pos(u_int16 voice_setting) 
{
    return ((voice_setting >> 2) & 0x0007);
}


/************************************************************************************
 *
 * FUNCTION: LMscoctr_Get_Input_Sample_Size
 *
 * DESCRIPTION:
 *  Return the size of the linear PCM samples. Determine if the base
 *  sample size is 16 bit or 8 bit.
 *  Then subtract the PCM bit pos from the base sample size.
 *
 ***********************************************************************************/
u_int8 LMscoctr_Get_Input_Sample_Size(u_int16 voice_setting)
{
    u_int8 linear_pcm_sample_size = LMscoctr_Get_PCM_Sample_Size(voice_setting);
    if(linear_pcm_sample_size == 1)
        linear_pcm_sample_size = 16;
    else
        linear_pcm_sample_size = 8;
    linear_pcm_sample_size -= LMscoctr_Get_Linear_PCM_Bit_Pos(voice_setting);

    return linear_pcm_sample_size;
}


/************************************************************************************
 *
 * FUNCTION: LMscoctr_Get_Air_Coding
 *
 * DESCRIPTION:
 *  Return the air coding portion of the voice setting
 *
 ***********************************************************************************/
t_air_coding_format LMscoctr_Get_Air_Coding(u_int16 voice_setting)
{
    return (t_air_coding_format)((voice_setting) & 0x0003);
}

/*
 * The following routines are used on slave-initiated SCO when
 * the IUT is a slave - it frees and re-allocates the SCO container
 * (losing the voice setting information. Store that voice
 * information in a holding variable.
 */
u_int16 _LMscoctr_Topical_Voice_Setting;
void LMscoctr_Store_Topical_Voice_Setting(u_int16 voice_setting)
{
    _LMscoctr_Topical_Voice_Setting = voice_setting;
}

u_int16 LMscoctr_Restore_Topical_Voice_Setting(void)
{
    return _LMscoctr_Topical_Voice_Setting;
}

/*
 * The following routines get and set the eSCO retransmission mode.
 * There are two eSCO retransmission modes: "shared" and "regular".
 *
 * In "shared" mode, all eSCO connections share any retransmission
 * opportunities.
 *
 * In "regular" mode, each eSCO connection has its own retransmission
 * opportunities.
 *
 * All eSCO connections MUST either (a) be ALL shared OR (b) be
 * ALL regular.
 */
t_esco_retrans_mode LMscoctr_Get_eSCO_Retransmission_Mode()
{
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    return _LMscoctr_esco_retrans_mode;
#else
    return REGULAR_RETRANS;
#endif
}


void LMscoctr_Set_eSCO_Retransmission_Mode(t_esco_retrans_mode mode)
{
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    _LMscoctr_esco_retrans_mode = mode;
#endif
}


#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
/*******************************************************************
 *
 *******************************************************************/
static u_int8 _LMscoctr_Derive_D_SCO_Repeater_Mode(t_sco_info *p_sco_link_new,u_int8 timing_ctrl)
{
    u_int8 d_sco;               
    u_int  i;
    t_sco_info *p_sco_link;
    t_clock clk_ofs;

    u_int found=FALSE;

    /*
     * Determine D_SCOs which are already allocated.
     */
    for (i=0; (i < PRH_BS_CFG_SYS_MAX_SYN_LINKS)&&(!found); i++)
    {
        p_sco_link = sco_link_container+i;
        if ((p_sco_link->state != SCO_UNUSED) &&
            (p_sco_link->state != SCO_IDLE) )
        {
            if (MASTER == DL_Get_Role_Peer(DL_Get_Device_Ref(p_sco_link->device_index)))
               found=TRUE;
        }
    }

/*
 * Dsco = (Dsco_assigned-((CLK_OFS>>1) mod Tsco )+3) mod Tsco
 * simply map next activity time to NT clk 
 * and +3 will give searching d_sco!
 */
        p_sco_link = sco_link_container;

     {

/* map this BT clk activity 
 * to NT own clock
 * as difference is ISO= 625
 * then:
 *        NT_activity = BT_activity - Ofs + 2
 *
 * and for 3 slots (+6) in advance will provide correct value of Dsco
 * and +1 as it is timer!
 */

     u_int32 next_activity = p_sco_link->next_activity_time;
     clk_ofs = DL_Get_Clock_Offset(DL_Get_Device_Ref(p_sco_link->device_index));
     next_activity -= clk_ofs;

     next_activity = BTtimer_Clock_Add_Safe_Clockwrap( next_activity,((3<<1)+2+1));

     d_sco = BTtimer_Extract_Timer_D_proc_slots(
       next_activity, timing_ctrl, p_sco_link->t_sco);
     }

    return d_sco;

}
#endif

t_length LMscoctr_Determine_SCO_Pkt_Length(u_int8 sco_packet)
{
	t_length pkt_contents_length = 0;

	if (sco_packet == LM_SCO_PACKET_HV1)
	{
		pkt_contents_length = 10;
	}
	else if (sco_packet == LM_SCO_PACKET_HV2)
	{
		pkt_contents_length = 20;
	}
	else if (sco_packet == LM_SCO_PACKET_HV3)
	{
		pkt_contents_length = 30;
	}
	return pkt_contents_length;
}


#if 1 // New Transcode/CVSD Functions
void LMscoctr_Advance_Tx_Transcode_State(t_sco_info *p_sco_link)
{
	p_sco_link->Start_Tx_Hpf_Filt = p_sco_link->End_Tx_Hpf_Filt;
	p_sco_link->Start_Tx_Pf1_Filt_A = p_sco_link->End_Tx_Pf1_Filt_A;
	p_sco_link->Start_Tx_Pf1_Filt_B = p_sco_link->End_Tx_Pf1_Filt_B;
	p_sco_link->Start_Tx_Pf2_Filt = p_sco_link->End_Tx_Pf2_Filt;
	p_sco_link->Start_Tx_Pf3_Filt = p_sco_link->End_Tx_Pf3_Filt;
	p_sco_link->Start_Tx_Cvsd_Filt_A = p_sco_link->End_Tx_Cvsd_Filt_A;
	p_sco_link->Start_Tx_Cvsd_Filt_B = p_sco_link->End_Tx_Cvsd_Filt_B;
}

void LMscoctr_Advance_Rx_Transcode_State(t_sco_info *p_sco_link)
{
	p_sco_link->Start_Rx_Hpf_Filt = p_sco_link->End_Rx_Hpf_Filt;
	p_sco_link->Start_Rx_Pf1_Filt_A = p_sco_link->End_Rx_Pf1_Filt_A;
	p_sco_link->Start_Rx_Pf1_Filt_B = p_sco_link->End_Rx_Pf1_Filt_B;
	p_sco_link->Start_Rx_Pf2_Filt = p_sco_link->End_Rx_Pf2_Filt;
	p_sco_link->Start_Rx_Pf3_Filt = p_sco_link->End_Rx_Pf3_Filt;
	p_sco_link->Start_Rx_Cvsd_Filt_A = p_sco_link->End_Rx_Cvsd_Filt_A;
	p_sco_link->Start_Rx_Cvsd_Filt_B = p_sco_link->End_Rx_Cvsd_Filt_B;
}

void LMscoctr_Read_End_Tx_Transcode_State(t_sco_info *p_sco_link)
{
	p_sco_link->End_Tx_Hpf_Filt = (mGetHWEntry32(JAL_TRANSCODE_END_TX_STATE_HFP_FILT) & 0x0000FFFF);
	p_sco_link->End_Tx_Pf1_Filt_A = mGetHWEntry32(JAL_TRANSCODE_END_TX_STATE_PF1_FILT_A);
	p_sco_link->End_Tx_Pf1_Filt_B = mGetHWEntry32(JAL_TRANSCODE_END_TX_STATE_PF1_FILT_B);
	p_sco_link->End_Tx_Pf2_Filt = mGetHWEntry32(JAL_TRANSCODE_END_TX_STATE_PF2_FILT);
	p_sco_link->End_Tx_Pf3_Filt = mGetHWEntry32(JAL_TRANSCODE_END_TX_STATE_PF3_FILT);
	p_sco_link->End_Tx_Cvsd_Filt_A = mGetHWEntry32(JAL_TRANSCODE_END_TX_STATE_CVSD_FILT_A);
	p_sco_link->End_Tx_Cvsd_Filt_B = (mGetHWEntry32(JAL_TRANSCODE_END_TX_STATE_CVSD_FILT_B) & 0x03FFFFFF);
}

void LMscoctr_Read_End_Rx_Transcode_State(t_sco_info *p_sco_link)
{
	p_sco_link->End_Rx_Hpf_Filt = (mGetHWEntry32(JAL_TRANSCODE_END_RX_STATE_HFP_FILT) & 0x0000FFFF);
	p_sco_link->End_Rx_Pf1_Filt_A = mGetHWEntry32(JAL_TRANSCODE_END_RX_STATE_PF1_FILT_A);
	p_sco_link->End_Rx_Pf1_Filt_B = mGetHWEntry32(JAL_TRANSCODE_END_RX_STATE_PF1_FILT_B);
	p_sco_link->End_Rx_Pf2_Filt = mGetHWEntry32(JAL_TRANSCODE_END_RX_STATE_PF2_FILT);
	p_sco_link->End_Rx_Pf3_Filt = mGetHWEntry32(JAL_TRANSCODE_END_RX_STATE_PF3_FILT);
	p_sco_link->End_Rx_Cvsd_Filt_A = mGetHWEntry32(JAL_TRANSCODE_END_RX_STATE_CVSD_FILT_A);
	p_sco_link->End_Rx_Cvsd_Filt_B = (mGetHWEntry32(JAL_TRANSCODE_END_RX_STATE_CVSD_FILT_B) & 0x03FFFFFF);
}

void LMscoctr_Write_Start_Tx_Transcode_State(t_sco_info *p_sco_link)
{
	mSetHWEntry32(JAL_TRANSCODE_START_TX_STATE_HFP_FILT,p_sco_link->Start_Tx_Hpf_Filt);
	mSetHWEntry32(JAL_TRANSCODE_START_TX_STATE_PF1_FILT_A,p_sco_link->Start_Tx_Pf1_Filt_A);
	mSetHWEntry32(JAL_TRANSCODE_START_TX_STATE_PF1_FILT_B,p_sco_link->Start_Tx_Pf1_Filt_B);
	mSetHWEntry32(JAL_TRANSCODE_START_TX_STATE_PF2_FILT,p_sco_link->Start_Tx_Pf2_Filt);
	mSetHWEntry32(JAL_TRANSCODE_START_TX_STATE_PF3_FILT,p_sco_link->Start_Tx_Pf3_Filt);
	mSetHWEntry32(JAL_TRANSCODE_START_TX_STATE_CVSD_FILT_A,p_sco_link->Start_Tx_Cvsd_Filt_A);
	mSetHWEntry32(JAL_TRANSCODE_START_TX_STATE_CVSD_FILT_B,((p_sco_link->Start_Tx_Cvsd_Filt_B & 0x03FFFFFF) | 0x04000000));
}


void LMscoctr_Write_Start_Rx_Transcode_State(t_sco_info *p_sco_link)
{
	mSetHWEntry32(JAL_TRANSCODE_START_RX_STATE_HFP_FILT,p_sco_link->Start_Rx_Hpf_Filt);
	mSetHWEntry32(JAL_TRANSCODE_START_RX_STATE_PF1_FILT_A,p_sco_link->Start_Rx_Pf1_Filt_A);
	mSetHWEntry32(JAL_TRANSCODE_START_RX_STATE_PF1_FILT_B,p_sco_link->Start_Rx_Pf1_Filt_B);
	mSetHWEntry32(JAL_TRANSCODE_START_RX_STATE_PF2_FILT,p_sco_link->Start_Rx_Pf2_Filt);
	mSetHWEntry32(JAL_TRANSCODE_START_RX_STATE_PF3_FILT,p_sco_link->Start_Rx_Pf3_Filt);
	mSetHWEntry32(JAL_TRANSCODE_START_RX_STATE_CVSD_FILT_A,p_sco_link->Start_Rx_Cvsd_Filt_A);
	mSetHWEntry32(JAL_TRANSCODE_START_RX_STATE_CVSD_FILT_B,((p_sco_link->Start_Rx_Cvsd_Filt_B & 0x03FFFFFF) | 0x04000000));

}


u_int8 LMscoctr_Current_eSCO_Link_In_eSCO_Window(t_sco_info *p_sco_link)
{
	if((p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_RX) ||
	   (p_sco_link->state == eSCO_MASTER_RETRANSMISSION_WINDOW_TX) ||
	   (p_sco_link->state == eSCO_MASTER_RETRANSMISSION_WINDOW_RX) ||
	   (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_RX) ||
	   (p_sco_link->state == eSCO_SLAVE_RETRANSMISSION_WINDOW_TX) ||
	   (p_sco_link->state == eSCO_SLAVE_RETRANSMISSION_WINDOW_RX))
       return 1;
	else
	   return 0;
}

u_int8 LMscoctr_Current_eSCO_Link_In_Reserved_Slot(t_sco_info *p_sco_link)
{
	if((p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_RX) ||
	   (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_RX))
       return 1;
	else
	   return 0;
}

#else

void LMscoctr_Advance_Tx_Transcode_State(t_sco_info *p_sco_link)
{
}

void LMscoctr_Advance_Rx_Transcode_State(t_sco_info *p_sco_link)
{
}

void LMscoctr_Read_End_Tx_Transcode_State(t_sco_info *p_sco_link)
{
}

void LMscoctr_Read_End_Rx_Transcode_State(t_sco_info *p_sco_link)
{
}

void LMscoctr_Write_Start_Tx_Transcode_State(t_sco_info *p_sco_link)
{
}


void LMscoctr_Write_Start_Rx_Transcode_State(t_sco_info *p_sco_link)
{
}


u_int8 LMscoctr_Current_eSCO_Link_In_eSCO_Window(t_sco_info *p_sco_link)
{
	if((p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_RX) ||
	   (p_sco_link->state == eSCO_MASTER_RETRANSMISSION_WINDOW_TX) ||
	   (p_sco_link->state == eSCO_MASTER_RETRANSMISSION_WINDOW_RX) ||
	   (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_RX) ||
	   (p_sco_link->state == eSCO_SLAVE_RETRANSMISSION_WINDOW_TX) ||
	   (p_sco_link->state == eSCO_SLAVE_RETRANSMISSION_WINDOW_RX))
       return 1;
	else
	   return 0;
}

u_int8 LMscoctr_Current_eSCO_Link_In_Reserved_Slot(t_sco_info *p_sco_link)
{
	if((p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_MASTER_RESERVED_SLOT_RX) ||
	   (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_TX) ||
       (p_sco_link->state == eSCO_SLAVE_RESERVED_SLOT_RX))
       return 1;
	else
	   return 0;
}

#endif

t_slots LMscoctr_Check_Slots_to_next_eSCO_On_Device(u_int32 clock)
{
/*
 * Include the negotiated link as well!
 */
	u_int8 i=0;
	t_sco_info* p_sco_link;
	//t_clock clock = LC_Get_Native_Clock();
	u_int32 slots_to_nearest_sco=0xFFFF;

	while (i < PRH_BS_CFG_SYS_MAX_SYN_LINKS+1)
	{
		p_sco_link = &sco_link_container[i];

		if ((p_sco_link->state!=SCO_IDLE) && (p_sco_link->state !=SCO_UNUSED))
		{
			u_int32 slots_to_sco_activity = (BTtimer_Clock_Difference(clock,p_sco_link->next_activity_time)/2);

			if (slots_to_sco_activity < slots_to_nearest_sco)
				slots_to_nearest_sco = slots_to_sco_activity;
		}

		i++;
    }

	return slots_to_nearest_sco;

}

u_int8 LMscoctr_Find_Lowest_Tsco(void)
{
	u_int8 j;
	t_sco_info* p_sco_link;
	u_int8 smallest_t_sco = 0xFF;

	for (j=0; j < PRH_BS_CFG_SYS_MAX_SYN_LINKS; j++)
	{
		p_sco_link = sco_link_container+j;

		if ((p_sco_link->state != SCO_UNUSED) && (p_sco_link->state != SCO_IDLE))
		{
			/*
			 * Find the smallest t_esco.
			 */
			if(smallest_t_sco > p_sco_link->t_sco)
			{
				smallest_t_sco = p_sco_link->t_sco;
			}
		}
	}
	return smallest_t_sco;
}


u_int8 LMscoctr_Find_Largest_Tsco(void)
{
	u_int8 j;
	u_int8 largest_t_sco = 0;
	t_sco_info* p_sco_link;

	for (j=0; j < PRH_BS_CFG_SYS_MAX_SYN_LINKS; j++)
	{
		p_sco_link = sco_link_container+j;

		if ((p_sco_link->state != SCO_UNUSED) && (p_sco_link->state != SCO_IDLE))
		{
			/*
			 * Find the largest t_esco.
			 */
			if(largest_t_sco < p_sco_link->t_sco)
			{
				largest_t_sco = p_sco_link->t_sco;
			}
		}
	}
	return largest_t_sco;
}

#else
 /*
  * Ensure non-empty source file for pedantic C
  */
 void LMPscoctr_Dummy(void);
#endif

