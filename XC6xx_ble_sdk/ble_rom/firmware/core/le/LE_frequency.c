/***********************************************************************
 *
 * MODULE NAME:    le_frequency.c
 * PROJECT CODE:   CEVA Low Energy Single Mode
 * DESCRIPTION:    Low Energy module for handling RF frequency/Channel aspects
 * 				   of the Low Energy Host Controller.
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  September 2011
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2011-2012 Ceva Inc.
 *     All rights reserved.
 *
 ***********************************************************************/

#include "sys_config.h"
#include "hc_const.h"
#include "hci_params.h"
#include "lmp_utils.h"
#include "le_connection.h"
#include "le_config.h"
#include "le_link_layer.h"

extern t_LE_Config LE_config;

/*****************************************************
 * The LE Host Controller channel map structure type
 *****************************************************/

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)

typedef struct
{
	u_int8  host_channel_map[5];
} t_LE_Frequency;

extern t_LE_Frequency LE_frequency;
/**************************************************************************
 * FUNCTION :- LEfreq_Set_Host_Channel_Classificiation
 *
 * DESCRIPTION
 * The function corresponds to the HCI_LE_Set_Host_Channel_Classification.
 *
 * The new map is copied to LE_config.
 *
 * In the non-optimised version, the new version of the Channel_Map is
 * copied in turn to each active link and a LL_CHANNEL_MAP_REQ send on each
 * link. The instance of the channel update for each link is 10 CEs in the future
 * for each link.
 *
 * In the optimised version the new channel map is stored in LE_config (as new_channel_map).
 * A common update time is determined for all links and the CE instant calculated
 * for each link. The LL_CHANNEL_MAP_REQ is sent on each link.
 *************************************************************************/

t_error LEfreq_Set_Host_Channel_Classificiation(t_p_pdu p_pdu)
{
	u_int8 i=0;
#if 1
	u_int8 valid_channel_exists = 0x00;

	// First Check if we have a valid channel set from the Host

	while((i<5) && (!valid_channel_exists))
	{
		if (p_pdu[i]!=0x00)
			valid_channel_exists = 0x01;
		i++;
	}

	if (!valid_channel_exists)
		return INVALID_HCI_PARAMETERS;
#else
	if (!((p_pdu[0]!=0x00) || (p_pdu[1]!=0x00) || (p_pdu[2]!=0x00) ||
		(p_pdu[3]!=0x00) || (p_pdu[4]!=0x00)))
		return INVALID_HCI_PARAMETERS;
#endif


	for(i=0;i<5;i++)
	{
		LE_frequency.host_channel_map[i] = p_pdu[i];
	}

	// Update the Channel Map on the available links.

	for (i=0;i<DEFAULT_MAX_NUM_LE_LINKS;i++)
	{
		t_LE_Connection* p_connection = LEconnection_Find_Link_Entry(i);

		if (LEconnection_Is_Active(p_connection))
		{
			u_int8 j;

			if (LEconnection_Get_Role(p_connection) == SLAVE)
			{
				return COMMAND_DISALLOWED;
			}

			for(j=0;j<5;j++)
				p_connection->channel_map_new[j] = LE_frequency.host_channel_map[j];

			if (!p_connection->ll_action_pending)
			{
				p_connection->channel_map_update_instant = p_connection->connection_event_counter + 10;
				p_connection->ll_action_pending |= LL_CHANNEL_MAP_REQ_PENDING;
				/* No Response timeout is applied to the Channel Map Update procedures for which there are no timeout
				 * rules
				 */
				//p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);
				LEllc_Encode_PDU(LL_CHANNEL_MAP_REQ,p_connection,0);
			}
		}
	}
	return NO_ERROR;
}
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
/**************************************************************************
 * FUNCTION :- LEfreq_Read_Channel_Map
 *
 * DESCRIPTION
 * This function corresponds to the HCI LE Read Channel Map Command.
 *************************************************************************/

u_int8* LEfreq_Read_Channel_Map(u_int16 handle)
{
	// Prior to this function being called the handle has already been
	// validated.

	t_LE_Connection*  p_connection = LEconnection_Find_P_Connection(handle);

	if ((p_connection->ll_action_pending & LL_CHANNEL_MAP_REQ_PENDING))
	{
		return p_connection->channel_map_new;
	}
	else
	{
		return p_connection->data_channel_map;
	}
}

/**************************************************************************
 * FUNCTION :- LEfreq_Get_Next_Data_Channel_Frequency
 *
 * DESCRIPTION
 * Get the next Data Channel to be used on a link.
 *************************************************************************/

u_int8 LEfreq_Get_Next_Data_Channel_Frequency(t_LE_Connection* p_connection)
{
	u_int8 unmappedChannel;
	u_int8 channelId;
	u_int8 remapping_index;

	// This function does the remapped from unused channels to
	// used data channels. Also does mapping from Data Channels
	// to RF Frequency.

	// channel status are stored as bit fields in 5 bytes.
	//
	// The used channels are stored in a byte array, in accending order
	// This allows quick indexing for remapping of channels.

	// Step 1
	// Add the Hop Increment to the last unmapped Channel used. Note :- this is the unmapped channel
	// and does not include any remapping. Ensure the result is under 37 - for valid data channel index.

	unmappedChannel = p_connection->lastUnmapped_Channel + p_connection->hop_increment;

	if (unmappedChannel >= 37)
		unmappedChannel-=37;

	// Store the unmapped channel as the last unmapped channel for further connection events
	p_connection->lastUnmapped_Channel = unmappedChannel;

	// Need to determine of the unmappedChannel is in use ?.

	//
	// Determine if the channel is in use - using the bits of the data channel map.
	// if (channelInUse)
	//		channel - unmappedChannel
	// else
	if (p_connection->data_channel_map[(unmappedChannel/8)] & (1<<(unmappedChannel&0x07)))
	{
		// Channel in use no remapping to occur.
		channelId = unmappedChannel;
	}
	else
	{
		// Channel not used - has to be remapped

		remapping_index = unmappedChannel % p_connection->numUsedChannels;
		channelId =  p_connection->channelRemapTable[remapping_index];
	}

	return channelId;

}

/**************************************************************************
 * FUNCTION :- LEfreq_Update_Channel_Remapping_Table
 *
 * DESCRIPTION
 * This function is called when the CE instant for a Channel Map Update occurs.
 * Following a channel map update it creates a new re-mapping table for a link.
 *************************************************************************/

void LEfreq_Update_Channel_Remapping_Table(t_LE_Connection* p_connection, u_int8* new_channel_map)
{
	u_int8 i;
	u_int8 current_index_into_remap_table = 0;

	u_int8* p_channelRemapTable = p_connection->channelRemapTable;

	for(i=0;i<37;i++)
	{
		if(new_channel_map[(i/8)] & (1<<(i&0x07)))
		{
			// Channel i in use - add to remap table
			p_channelRemapTable[current_index_into_remap_table] = i;
			current_index_into_remap_table++;
		}
	}

	p_connection->numUsedChannels = current_index_into_remap_table;

	for (i=current_index_into_remap_table ; i<37; i++)
	{
		p_connection->channelRemapTable[i] = 0;
	}
}

/**************************************************************************
 * FUNCTION :- LEfreq_Update_Channel_Map
 *
 * DESCRIPTION
 * Updates the channel map on a specific link.
 *************************************************************************/

t_error LEfreq_Update_Channel_Map(t_LE_Connection* p_connection)
{
	if (p_connection)
	{
		u_int8 i;

		for(i=0;i<5;i++)
			p_connection->data_channel_map[i] = p_connection->channel_map_new[i];

		LEfreq_Update_Channel_Remapping_Table(p_connection, p_connection->channel_map_new);

		p_connection->ll_action_pending &= ~LL_CHANNEL_MAP_REQ_PENDING;

	}
	return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
/**************************************************************************
 * FUNCTION :- LEfreq_Update_Channel_Map_Req
 *
 * DESCRIPTION
 * Handles an incoming LL_CHANNEL_MAP_REQ LLC PDU on a link. The instant is obtained
 * and the channel map stored in p_connection->channel_map_new.
 *
 * The new channel map will not be used until the instant has expired.
 *************************************************************************/

t_error LEfreq_Update_Channel_Map_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	if (p_connection)
	{
		u_int16 instant;
		u_int8 i;

		instant = LMutils_Get_Uint16(p_pdu+5);


		// Instant – connEventCount) modulo 65536
		if (((u_int16)(instant - p_connection->connection_event_counter)) >= 32767) // Instant in the past
		{
			LEconnection_Local_Disconnect(p_connection,INSTANT_PASSED);

			return NO_ERROR;
		}

		for(i=0;i<5;i++)
			p_connection->channel_map_new[i] = p_pdu[i];

		p_connection->channel_map_update_instant = instant;
		p_connection->ll_action_pending |= LL_CHANNEL_MAP_REQ_PENDING;
	}
	return NO_ERROR;
}
#endif

#endif


/**************************************************************************
 * FUNCTION :- LEfreq_Map_Channel_Index_2_RF_Freq
 *
 * DESCRIPTION
 * Converts a channel Index to the RF Frequency.
 *************************************************************************/

u_int8 LEfreq_Map_Channel_Index_2_RF_Freq(u_int8 channel_index)
{

	u_int8 rfChannel = 0;

	//
	// Map the advertising channels to RF index
	if (channel_index == 37)
	{
		rfChannel = 0;
	}
	else if (channel_index == 38)
	{
		rfChannel = 12;
	}
	else if (channel_index == 39)
	{
		rfChannel = 39;
	}
	// else this is a Data Channel which is mapped
	// to the RF index.
	else if (channel_index < 39)
	{
		if (channel_index < 11)
			rfChannel = channel_index+1;
		else if (channel_index < 37)
			rfChannel = channel_index+2;
	}
	return (rfChannel*2);
}




