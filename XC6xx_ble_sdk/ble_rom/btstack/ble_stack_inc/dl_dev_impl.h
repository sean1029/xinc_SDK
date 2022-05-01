#ifndef _PARTHUS_USLC_DEV_IMPL_
#define _PARTHUS_USLC_DEV_IMPL_

/******************************************************************************
 * MODULE NAME:    dl_dev_impl.h
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LC_DeviceLink
 * MAINTAINER:     John Nelson,Ivan Griffin, Conor Morris, Gary Fleming
 * DATE:           27 May 1999
 *
 * SOURCE CONTROL: $Id: dl_dev_impl.h,v 1.121 2012/05/24 14:33:36 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 1999-2004 Ceva Inc.
 *     All rights reserved.
 *
 ******************************************************************************/

/*
 * Asserts are used here only for test.  An extra check!
 */
#ifndef assert
#include <assert.h>
#endif

#include "sys_config.h"
#include "lc_types.h"
#include "dl_dev.h"
#include "hw_lc.h"
#include "lslc_access.h"

#include "lmp_sco_container.h"

#if (PRAGMA_INLINE==1)
#pragma inline(DL_Initialise,\
DL_Set_Ctrl_State,\
DL_Get_Ctrl_State,\
DL_Set_Role_Peer,\
DL_Get_Role_Peer,\
DL_Get_Role_Local,\
DL_Set_AM_Addr,\
DL_Get_Am_Addr,\
DL_Get_Tx_LT_Am_Addr,\
DL_Get_Rx_LT_Am_Addr,\
DL_Set_Access_Syncword,\
DL_Get_Access_Syncword,\
DL_Get_Access_Syncword_Ref,\
DL_Get_Remote_Rxbuf_Full,\
DL_Set_Remote_Rxbuf_Full,\
DL_Get_Local_Rxbuf_Full,\
DL_Set_Local_Rxbuf_Full,\
DL_Set_Rx_Expected_Seqn,\
DL_Set_Rx_Expected_Broadcast_Seqn,\
DL_Toggle_Rx_Expected_Broadcast_Seqn,\
DL_Get_Rx_Expected_Broadcast_Seqn,\
DL_Get_Tx_Ack_Pending,\
DL_Set_Tx_Ack_Pending,\
DL_Get_Tx_toMaster,\
DL_Set_Tx_toMaster,\
DL_Get_Rx_Status,\
DL_Set_Rx_Status,\
DL_Get_Power_Level,\
DL_Set_Power_Level,\
DL_Get_Local_Slot_Offset,\
DL_Set_Local_Slot_Offset,\
DL_Get_Peer_Slot_Offset,\
DL_Set_Peer_Slot_Offset,\
DL_Get_Piconet_Clock_Last_Access,\
DL_Set_Piconet_Clock_Last_Access,\
DL_Get_Clock_Jitter,\
DL_Set_Clock_Jitter,\
DL_Get_Clock_Drift,\
DL_Set_Clock_Drift,\
DL_Is_Local_Rx_Buffer_Full,\
DL_Get_Rx_Packet_Type,\
DL_Get_Rx_Length,\
DL_Get_Rx_Payload,\
DL_Get_Rx_Qpdu,\
DL_Get_Rx_Message_Type,\
DL_Set_Rx_Packet_Type,\
DL_Set_Rx_Length,\
DL_Set_Rx_Payload,\
DL_Set_Rx_Message_Type,\
DL_Get_Tx_Packet_Type,\
DL_Get_Tx_Length,\
DL_Get_Tx_Payload,\
DL_Get_Tx_Qpdu,\
DL_Get_Tx_Message_Type,\
DL_Set_Tx_Packet_Type,\
DL_Set_Tx_Length,\
DL_Set_Tx_Payload,\
DL_Set_Tx_Qpdu,\
DL_Set_Tx_Message_Type,\
DL_Set_Encryption_Mode,\
DL_Get_Encryption_Mode,\
DL_Set_Encryption_Key_Ref,\
DL_Get_Encryption_Key_Ref,\
DL_Set_Encryption_Key_Length,\
DL_Get_Encryption_Key_Length,\
DL_Get_Local_Device_Ref,\
DL_Get_Local_Device_Index,\
DL_Get_Device_Ref,\
DL_Get_Device_Index,\
DL_Get_Local_Tx_L2CAP_Flow,\
DL_Set_Local_Tx_L2CAP_Flow,\
DL_Get_Local_Rx_L2CAP_Flow,\
DL_Set_Local_Rx_L2CAP_Flow,\
DL_Get_Tx_SCO_Packet_Type,\
DL_Set_Tx_SCO_Packet_Type,\
DL_Get_Active_SCO_Index,\
DL_Set_Active_SCO_Index,\
DL_Decode_Valid_LT_address,\
DL_Set_Active_Tx_LT,\
DL_Set_Active_Rx_LT,\
DL_Get_Active_Rx_LT,\
DL_Get_Active_Tx_LT,\
DL_Set_eSCO_LT_Address,\
DL_Get_eSCO_LT_Address,\
DL_Get_eSCO_Rx_Length,\
DL_Set_eSCO_Rx_Length,\
DL_Set_LT_Address,\
DL_Get_LT_Address,\
DL_Get_Tx_LT_Previous_Arqn,\
DL_Set_Rx_LT_Previous_Arqn,\
DL_Toggle_Rx_LT_Seqn,\
DL_Set_Tx_LT_Seqn,\
DL_Get_Tx_LT_Seqn,\
DL_Toggle_Tx_LT_Seqn,\
DL_Set_Rx_LT_Expected_Seqn,\
DL_Get_Rx_LT_Expected_Seqn,\
DL_Toggle_Rx_LT_Expected_Seqn,\
DL_Get_Tx_LT_Tx_Ack_Pending,\
DL_Set_Tx_LT_Tx_Ack_Pending,\
DL_Get_Rx_LT_Tx_Ack_Pending,\
DL_Set_Rx_LT_Tx_Ack_Pending,\
DL_Set_UAP_LAP,\
DL_Get_UAP_LAP,\
DL_Set_NAP,\
DL_Get_NAP,\
DL_Get_Bd_Addr_Ex,\
DL_Get_Clock_Offset,\
DL_Set_Clock_Offset,\
DL_Set_Tx_Seqn_Active,\
DL_Set_Tx_Seqn_Not_Active,\
DL_Is_Tx_Seqn_Active\
)

#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
#pragma inline(DL_Inc_Tx_ACL_Data_Packets,\
DL_Inc_Tx_ACL_Data_Packets_Acked,\
DL_Inc_Rx_ACL_Data_Packets,\
DL_Inc_Rx_ACL_Data_Packets_Nacked,\
DL_Inc_Tx_SYN_Data_Packets,\
DL_Inc_Tx_SYN_Data_Packets_Acked,\
DL_Inc_Rx_SYN_Data_Packets,\
DL_Inc_Rx_SYN_Data_Packets_Nacked,\
DL_Get_Channel_Quality,\
DL_Initialise_Channel_Quality)
#endif  /*(PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)*/


#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
#pragma inline(DL_Is_EDR_Enabled,\
DL_Set_PTT_ACL,\
DL_Get_PTT_ACL,\
DL_Set_PTT_ESCO,\
DL_Get_PTT_ESCO)
#endif


#endif
#endif

/*
 * EDR related
 */
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
/*
 * DL_Is_EDR_Enabled
 */
__INLINE__ boolean DL_Is_EDR_Enabled(const t_devicelink* pDevLink)
{
   return ((pDevLink->ptt_acl) | (pDevLink->ptt_esco)) ;
}

/*
 * DL_Get_PTT_ACL
 */
__INLINE__ boolean DL_Get_PTT_ACL(const t_devicelink* pDevLink)
{
	return pDevLink->ptt_acl;
}
/*
 * DL_Set_PTT_ACL
 */
__INLINE__ void DL_Set_PTT_ACL(t_devicelink* pDevLink, boolean ptt_acl)
{
	pDevLink->ptt_acl=ptt_acl;
}

/*
 * DL_Get_PTT_ESCO
 */
__INLINE__ boolean DL_Get_PTT_ESCO(const t_devicelink* pDevLink)
{
	return pDevLink->ptt_esco;
}

/*
 * DL_Set_PTT_ESCO
 */
__INLINE__ void DL_Set_PTT_ESCO(t_devicelink* pDevLink, boolean ptt_esco)
{
	pDevLink->ptt_esco=ptt_esco;
}
#endif

#if (PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1)
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
__INLINE__ void DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(t_devicelink* pDevLink,t_q_descr *p_descriptor)
{
	pDevLink->p_CurrentSCOdescriptor = p_descriptor;
}

__INLINE__ t_q_descr* DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(t_devicelink* pDevLink)
{
	return pDevLink->p_CurrentSCOdescriptor;
}
#endif
#endif

