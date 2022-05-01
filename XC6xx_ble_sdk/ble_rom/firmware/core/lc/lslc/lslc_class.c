/******************************************************************************
 * MODULE NAME:    lslc_class.h
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LSLC Adaptive Frequency Classification
 * MAINTAINER:     John Nelson
 * DATE:           10 Feb 2003
 *
 * SOURCE CONTROL: $Id: lslc_class.c,v 1.12 2008/03/10 17:26:01 nicolal Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2003-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     10 Feb 2003 -   jn       - Initial Version based on V1.2 Draft2
 *
 * NOTES TO USERS:
 ******************************************************************************/

#include "sys_config.h"
#include "lc_types.h"
#include "lslc_class.h"
#include "lslc_hop.h"
#include "lslc_afh.h"
#include "dl_dev.h"
#include "bt_timer.h"
#include "hw_memcpy.h"

#if (PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_SUPPORTED==1)

/*
 * Maintain a bit vector representing the device channel classifications
 * on an individual bit position.
 *
 * The nth (numbering from 0) such field (in the range 0 to 78) contains the 
 * value for channel n. Bit 79 is reserved (set to 0 when transmitted and 
 * ignored when received). The 1-bit field is interpreted as follows:
 * 0: channel n is unused, 1: channel n is used
 *
 * This vector should be updated as required to the master 
 * assessed channels.
 */
u_int8 LSLCcls_device_channel_map[AFH_BYTES_PER_CHANNEL_MAP];

/*
 * Maintain the piconet reported classifications
 *
 * AM_ADDR 0    is this devices current classification
 *         1-7  represents the corresponding slaves
 * 
 * The channel classification is 40 2-bit fields.  
 * The nth (numbering from 0) such field defines the classification of channels 
 * 2n and 2n+1, other than the 39th field which just contains the classification 
 * of channel 78.   Each field interpreted as an integer whose values indicate:
 *  0 = unknown   1 = good    2 = reserved  3 = bad
 */
STATIC u_int8 LSLCcls_AFH_channel_classification[MAX_NUM_AMADDR][AFH_BYTES_PER_CLASSIFICATION];

/************************************************************************
 * LSLCcls_Initialise
 *
 * Initialise all the specified AFH classification structures
 * 
 ************************************************************************/
void LSLCcls_Initialise(void)
{
    t_am_addr am_addr;

    LSLCcls_Record_Device_Channel_Map(mLSLCafh_Get_79_Bit_Vector_Set_Ref());
    
    /*
     * Initialise slaves to all good.
     */
    for (am_addr = 0; am_addr < MAX_NUM_AMADDR; am_addr++)
    {
        LSLCcls_Initialise_Slave_Channel_Classification(am_addr);
    }

    /*
     * Initialise Channel Assessment if supported
     */
    LSLCass_Initialise_Device_Channel_Assessment();
}

/************************************************************************
 * LSLCcls_Initialise_Slave_Channel_Classification
 *
 * Initialise the latest reported slave AFH classification to all good.
 *
 * am_addr              am_addr of slave 
 ************************************************************************/
void LSLCcls_Initialise_Slave_Channel_Classification(t_am_addr am_addr)
{
    hw_memset8(LSLCcls_AFH_channel_classification[am_addr], 
            0x55, AFH_BYTES_PER_CLASSIFICATION);
}

/************************************************************************
 * LSLCcls_Record_Slave_Channel_Classification
 *
 * Record the latest reported slave AFH classification by the
 * LMP_channel_classification report.
 *
 * The reported classification will be used to generate the channel
 * map to report to other devices.
 *
 * am_addr                      master's am_addr of slave 
 * afh_channel_classification   reported channel classifications          
 ************************************************************************/
void LSLCcls_Record_Slave_Channel_Classification(
    t_am_addr am_addr, u_int8 afh_channel_classification[])
{
    hw_memcpy8(LSLCcls_AFH_channel_classification[am_addr], 
        afh_channel_classification, AFH_BYTES_PER_CLASSIFICATION);
}


/************************************************************************
 * LSLCcls_Record_Device_Channel_Map
 *
 * Record the device channel map as a result of channel assessment
 * on this device.  For now a single bit is used as for the HCI
 * reported channel classification where 0 is unused, 1 is used.
 *
 * afh_device_channel_map       An array of channel bits
 *
 * The nth (numbering from 0) such field (in the range 0 to 78) contains the 
 * value for channel n. Bit 79 is reserved (set to 0 when transmitted and 
 * ignored when received). The 1-bit field is interpreted as follows:
 * 0: channel n is unused, 1: channel n is used
 *
 * This function does not request an Adaptive Hopping Set update via
 * the LMP_set_afh.
 * This may be done byt the caller of this function, if required.
 *
 * The definition of this function, including its interface, is 
 * subject to change, as channel assessment progresses.
 *
 ************************************************************************/
void LSLCcls_Record_Device_Channel_Map(const u_int8 afh_device_channel_map[])
{
    hw_memcpy8(LSLCcls_device_channel_map, afh_device_channel_map,
        AFH_BYTES_PER_CHANNEL_MAP);
}

/****************************************************************************
 * LSLCcls_Build_Channel_Classification
 *
 * Description
 *  Build the channel classification vector by combining the host 
 *  classification and the slave reported information
 *
 *  For now just use the Host classification
 *
 * The channel classification is 40 2-bit fields.  
 * The nth (numbering from 0) such field defines the classification of channels 
 * 2n and 2n+1, other than the 39th field which just contains the classification 
 * of channel 78.   Each field interpreted as an integer whose values indicate:
 *  0 = unknown   1 = good    2 = reserved  3 = bad
 *
 *  Odd/Even channel map pair   Channel Classification Pair
 *  00      Bad                 11   Bad             
 *  01      Unknown/Bad         00   Unknown
 *  10      Bad/Unknown         00   Unknown
 *  11      Unknown             00   Unknown
 *
 *  Returns     TRUE        classification vector has changed
 *              FALSE       no change
 *
 * Algorithm
 * 1. Extract each pair of channels for the host and device classifications
 *    and classify as bad if both are bad, else classify the pair as unknown.
 *
 * Notes
 * 1. The algorithm only uses bad or unknown.
 *
 ****************************************************************************/ 
boolean LSLCcls_Build_Channel_Classification(u_int8 afh_channel_classification[], 
    const u_int8 afh_host_channel_classification[])
{
    boolean change_occurred = FALSE;
    int i, j;

    for(i=0; i < AFH_BYTES_PER_CLASSIFICATION; i++)
    {
        u_int  afh_channel_class_byte = 0;
        u_int  mask = 0x03;
        u_int  afh_host_device_byte;
        
        afh_host_device_byte = afh_host_channel_classification[i] &
                             LSLCcls_device_channel_map[i];

        /*
         * For each odd/even channel pair, mark as bad if both
         * adjacent even/odd are unused in the combined 
         * host or device individual channel classifications
         */
        for (j=0; j < 4; j++)
        {
            if ((afh_host_device_byte & mask) == 0 )
            {
                afh_channel_class_byte |= mask;
            }
            mask <<= 2;
        }
        /*
         * Handle the special case of 79th channel, if 0 then report pair as bad
         */
        if (i==9 && ((afh_host_channel_classification[9] & 0x40) == 0) )
        {
            afh_channel_class_byte |= 0xC0; 
        }

        if (afh_channel_classification[i] != afh_channel_class_byte)
        {
            /*
             * Record change
             */
            afh_channel_classification[i] = afh_channel_class_byte;
            change_occurred = TRUE;
        }
    }

    return change_occurred;
}

/****************************************************************************
 * LSLCcls_Build_Channel_Map
 *
 * Description
 *  Build the channel map vector by combining the host 
 *  classification and the slave reported information
 *
 *  Channel Classification Pair     Odd/Even channel map pair
 *  00      Unknown                 11   Used             
 *  01      Good                    11   Used
 *  10      Reserved                11   Used
 *  11      Bad                     00   Unused
 *
 *  Returns     TRUE        map has changed due to master/slave assessment
 *              FALSE       map is as set by host
 *
 * Algorithm
 * 1. Initialise the channel map to the classification reported by the HCI.
 * 2. Classify each unknown according to master's assessment
 * 3. Classify according to each slave report in order of am_addr
 *
 * Notes
 * 1. The Master Assessment will default to good until superceded by
 *    measurement updates
 * 2. The Slave Assessments will default to good until superceded by
 *    slave sent LMP_channel_classification
 * 3. The algorithm doesn't differentiate good versus unknown.  In
 *    effect, only bad is used to mark a channel as unused
 * 4. Each channel map/classification byte is classified in turn.
 * 5. The algorithm will terminate when all channels have been 
 *    classified, or the minimum number of channels is reached.
 *
 ****************************************************************************/ 
boolean LSLCcls_Build_Channel_Map(u_int8 afh_channel_map[], 
    const u_int8 afh_host_channel_classification[])
{
    u_int8 new_afh_channel_map[AFH_BYTES_PER_CLASSIFICATION];
    u_int8 num_unknown_channels;
    u_int8 channel_bit_mask;
	u_int8 *p_map_channel_byte;
	boolean change_occurred = FALSE;
    int i, j;

    /*
     * Default to Host classification, and count number of unknown channels
     */
    num_unknown_channels = LSLCafh_Count_1_Bits(afh_host_channel_classification);
    hw_memcpy8(new_afh_channel_map, afh_host_channel_classification,
        AFH_BYTES_PER_CLASSIFICATION);
	
	/*
     * The master assesment is done first. Followed by the slave assessment.
	 * This gives the master assesment more weight. Giving the master's assesment
	 * more weight improves the algorithm. Otherwise, when you reach the threshold 
	 * having only 20 good channels left, the channels at the end of the channel
	 * map will not be assessed. This change increases the liklihood of the last 
	 * channels getting assessed.
     */
    for (i = 0; i < AFH_N_MAX_USED_CHANNELS &&
        num_unknown_channels > AFH_N_MIN_USED_CHANNELS; i++)
    {
        channel_bit_mask = 1<<(i&0x07);
        p_map_channel_byte = new_afh_channel_map + (i>>3);

        /*
         * If unknown and device has marked channel as unused Then
         *     Mark as unused
         * Else
         *     Leave channels as unknown
         * Endif
         */
        if ((*p_map_channel_byte & channel_bit_mask) &&
            !(LSLCcls_device_channel_map[i>>3] & channel_bit_mask))
        {
            *p_map_channel_byte &= ~channel_bit_mask;
            --num_unknown_channels;
        }
	}
	
	/*
	 * Slave assessment
	 */
	for (i = 0; i < AFH_N_MAX_USED_CHANNELS &&
        num_unknown_channels > AFH_N_MIN_USED_CHANNELS; i++)
    {
		channel_bit_mask = 1<<(i&0x07);
        p_map_channel_byte = new_afh_channel_map + (i>>3);

		/*
         * Classify each slave device (simplified by checking all slaves)
         */
        for(j=1; j < MAX_NUM_AMADDR; j++)
        {
            /* 
			 * If still unknown, check if slave has classified as bad
             */
            if (*p_map_channel_byte & channel_bit_mask)
            {
                /*
                 * Extract and apply the slave device assessment
                 */
                u_int8 classification_pair_pos = (i & 0x06);
                u_int8 class_pair_mask = 0x03 << classification_pair_pos;
                u_int8 device_channel_pair_classification =
                        (LSLCcls_AFH_channel_classification[j][i>>3] & class_pair_mask)
                        >> classification_pair_pos;
                /*
                 * If classification for this pair is bad, then mark as Unused.
                 */
                if (device_channel_pair_classification == AFH_CH_CLASS_BAD)
                {
                    *p_map_channel_byte &= ~channel_bit_mask; 
                     --num_unknown_channels;
                }       
            }
        }
    }

	for(i = 0; i<AFH_BYTES_PER_CLASSIFICATION && !change_occurred; i++)
	{
        if(new_afh_channel_map[i] != afh_channel_map[i])
		{
            change_occurred = TRUE;
            hw_memcpy8(afh_channel_map, new_afh_channel_map, AFH_BYTES_PER_CLASSIFICATION);
		}
	}

    return change_occurred;
}

#endif /* (PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_SUPPORTED==1) */
