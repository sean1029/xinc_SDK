/*************************************************************************
 *
 * MODULE NAME:    lslc_assmnt.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Bluetooth AFH Channel Assessment
 *
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  08 July 2003
 *
 * SOURCE CONTROL: $Id: lslc_assmnt.c,v 1.34 2013/12/13 18:54:29 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2003-2012 Ceva Inc.
 *     All rights reserved.
 *
 * NOTES:
 *
 *     Methods of classification based upon RSSI with PER (HEC/CRC) & NAK.
 *     Concepts adopted from REF: IEEE P802.15.2/Draft #09 March 24, 2003,
 *     with some proprietary extensions.
 *
 ***************************************************************************/

#include "sys_config.h"
#include "sys_types.h"
#include "lc_types.h" 
#include "lslc_freq.h"
#include "hw_lc.h"
#include "hw_radio.h"
#include "bt_timer.h"
#include "uslc_chan_ctrl.h"
#include "lmp_acl_container.h"
#include "lmp_afh.h"

#if (PRH_BS_CFG_SYS_CHANNEL_ASSESSMENT_SCHEME_SUPPORTED==1)

#define LSLCASS_GOLDEN_RECEIVER_RSSI_DEVIATION 8
#define LSLCASS_BACKGROUND_RSSI_AVERAGE_MAX_THRESHOLD (GOLDEN_RECEIVER_RSSI_MIN)
#define LSLCASS_BACKGROUND_RSSI_AVERAGE_MIN_THRESHOLD (-70)
#define LSLCASS_RSSI_MIN (-80)
#define LSLCASS_BACKGROUND_RSSI_EXPECTED_VARIANCE  25
#define LSLCASS_BG_RSSI_LO_THRESHOLD 4

#define LSLCASS_MIN_FREQ 0
#define LSLCASS_MAX_FREQ 78

#define LSLCASS_MIN_USED_CHANNELS 30 /* spec mandates min 20 channels */
#define LSLCASS_MAX_UNUSED_CHANNELS (79 - LSLCASS_MIN_USED_CHANNELS)

#define LSLCASS_CHANNEL_MAP_REFRESH_SLOTS ((1600)*(30)) /* 30 seconds */
#define LSLCASS_EXPIREY_SLOTS ((1600)*(60)*(1)) /* 1 minute */
#define LSLCASS_UNUSED_EXPIREY_SLOTS ((1600)*(60)*(2)) /* 2 minutes */

STATIC u_int8 LSLCass_freq_interference[LSLCASS_MAX_FREQ+1];
STATIC t_slots LSLCass_expirey_slots[LSLCASS_MAX_FREQ+1];

typedef struct t_Assessment
{
    s_int8 background_rssi;
    s_int8 background_rssi_average;
    t_timer channel_map_refresh_timer;
} t_Assessment;

t_Assessment _assessment;

//Wifi Channel Details:
//#define LSLCASS_NUM_WIFI_CHANNELS 11
//#define WIFI_CH_LOWER 0
//#define WIFI_CH_MID   1
//#define WIFI_CH_UPPER 2
//const u_int8 wifi_ch[LSLCASS_NUM_WIFI_CHANNELS][3] =
//    {{  1, 12, 23},
//     {  4, 17, 28},
//     { 11, 22, 33},
//     { 16, 27, 38},
//     { 21, 32, 43},
//     { 26, 37, 48},
//     { 31, 42, 53},
//     { 36, 47, 58},
//     { 41, 52, 63},
//     { 46, 57, 68},
//     { 51, 62, 73}};
#define WIFI_CHANNEL_WIDTH 23

/************************************************************************
 * DEVICE_CHANNEL_MAP (EXTERN)
 *
 * Record the device channel map as a result of channel assessment
 * on this device. For now a single bit is used as for the HCI reported
 * channel classification where 0 is unused, 1 is used.
 *
 * The nth (numbering from 0) such field (in the range 0 to 78) contains
 * value for channel n. The 1-bit field is interpreted as follows:
 * 0: channel n is unused, 1: channel n is used
 *
 ************************************************************************/

extern u_int8 LSLCcls_device_channel_map[];

STATIC u_int8 LSLCass_num_channels_unused;
STATIC u_int8 LSLCass_control_log;

#define LSLCASS_IS_FREQ_USED(x) ((LSLCcls_device_channel_map[(x)>>3]&(1<<((x)&7)))!=0)

__INLINE__ void LSLCASS_SET_FREQ_USED(u_int8 x)
{
    if(!LSLCASS_IS_FREQ_USED(x))
    {
        LSLCcls_device_channel_map[(x)>>3]|=(1<<((x)&7));
        LSLCass_num_channels_unused--;
        LSLCass_control_log <<=1;
    }
}

__INLINE__ void LSLCASS_SET_FREQ_UNUSED(u_int8 x)
{
    if(LSLCASS_IS_FREQ_USED(x))
    {
        LSLCcls_device_channel_map[(x)>>3]&=~(1<<((x)&7));
        LSLCass_num_channels_unused++;
        LSLCass_control_log <<=1;
        LSLCass_control_log++;
    }
}

__INLINE__ boolean LSLCASS_IS_FREQ_VALID(u_int8 x)
{
    /* unsigned x >= LSLCASS_MIN_FREQ(0) always valid */
    return (x <= LSLCASS_MAX_FREQ);
}

/************************************************************************
 * LSLCass_Initialise_Device_Channel_Assessment
 *
 * Initialises channel assessment module.
 *
 ************************************************************************/
void LSLCass_Initialise_Device_Channel_Assessment(void)
{
    u_int8 freq;

    t_slots expirey_slots = BTtimer_Set_Slots(LSLCASS_EXPIREY_SLOTS);

    for(freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    {
        LSLCass_freq_interference[freq] = 0;
        LSLCass_expirey_slots[freq] = expirey_slots;
    }

    _assessment.channel_map_refresh_timer = BTtimer_Set_Slots(LSLCASS_CHANNEL_MAP_REFRESH_SLOTS);

    _assessment.background_rssi_average = LSLCASS_RSSI_MIN;

    LSLCass_num_channels_unused = 0;
    LSLCass_control_log = 0;
}

/************************************************************************
 * LSLCass_Add_Channel_Interference
 *
 * Adds an interference measurement to a specified channel region.
 *
 ************************************************************************/

void LSLCass_Add_Channel_Interference(u_int8 freq, u_int8 interference)
{
    if (interference >= LSLCass_freq_interference[freq])
    {
        LSLCass_freq_interference[freq] = interference;
        LSLCass_expirey_slots[freq] = BTtimer_Set_Slots(LSLCASS_EXPIREY_SLOTS);
    }
}

/************************************************************************
 * LSLCass_Update_Background_RSSI_Average
 *
 * Updates background RSSI average.
 *
 ************************************************************************/ 
void LSLCass_Update_Background_RSSI_Average(s_int8 update_bg_rssi)
{
    s_int8 ave = _assessment.background_rssi_average;
    s_int32 sum = ave + ave + ave + update_bg_rssi;
    _assessment.background_rssi_average = (s_int8)(sum >> 2);
}

/************************************************************************
 * LSLCass_Is_Assessment_State
 *
 * Checks whether link state appropriate for channel assessment.
 *
 ************************************************************************/
boolean LSLCass_Is_Assessment_State(t_devicelink* p_dev_link)
{
    t_lmp_link* p_link = LMaclctr_Find_Device_Index(p_dev_link->device_index);
    return (p_link && mLMafh_Is_Assessment_State(p_link->state) &&
       ((p_link->role == MASTER) || LMafh_Is_Classification_Reporting(p_link)));
}

/************************************************************************
 * LSLCass_Determine_Interference_Presence
 *
 * Handler to determine if interference present based on Backround RSSI.
 *
 ************************************************************************/ 
boolean LSLCass_Determine_Interference_Presence(s_int8 bg_rssi)
{
    boolean interference = FALSE;
    s_int8 bg_rssi_ave = _assessment.background_rssi_average;

    if((bg_rssi_ave > LSLCASS_BACKGROUND_RSSI_AVERAGE_MAX_THRESHOLD)
    && (bg_rssi > (GOLDEN_RECEIVER_RSSI_MAX+LSLCASS_GOLDEN_RECEIVER_RSSI_DEVIATION)))
    {   /*jammer source is very close*/
        interference = TRUE;
    }
    else if((bg_rssi > (bg_rssi_ave + LSLCASS_BACKGROUND_RSSI_EXPECTED_VARIANCE))
    && (bg_rssi_ave < LSLCASS_BACKGROUND_RSSI_AVERAGE_MIN_THRESHOLD))
    {   /*possibly jammer is coming here*/
        interference = TRUE;
    }
    else if((bg_rssi >= GOLDEN_RECEIVER_RSSI_MAX)
    && (bg_rssi_ave <= LSLCASS_BACKGROUND_RSSI_AVERAGE_MAX_THRESHOLD))
    {   /*possibly jammer source is around here*/
        interference = TRUE;
    }

    return interference;
}

/************************************************************************
 * LSLCass_Record_Rx_CRC_Event_Channel
 *
 * Records RX CRC event on channel. This can be used in conjunction
 * with some other method, in this case RSSI contributes also.
 * REF: IEEE P802.15.2/Draft #09 March 24, 2003, 11.1.2 PER (page 40).
 *
 ************************************************************************/
void LSLCass_Record_Rx_CRC_Event_Channel(t_devicelink* p_dev_link)
{
    if (LMafh_LM_Read_AFH_Channel_Assessment_Mode() && LSLCass_Is_Assessment_State(p_dev_link))
    {
        if((p_dev_link->device_index) && (USLCchac_Get_Active_Procedure()==NO_PROCEDURE))
	    {
            t_freq rx_freq = LSLCfreq_Get_Frequency(RX_START);

            if(LSLCASS_IS_FREQ_VALID(rx_freq) /*&& LSLCASS_IS_FREQ_USED(rx_freq)*/)
            {
            	s_int8 rssi = HWradio_Read_RSSI(); /* refreshed via HWradio_Update_Internal_RSSI_Cache on SYNC DET */

                if(rssi > (p_dev_link->rssi_average + LSLCASS_GOLDEN_RECEIVER_RSSI_DEVIATION))
                {
                    LSLCass_Add_Channel_Interference(rx_freq, (rssi - p_dev_link->rssi_average));
                }
            }
        }
    }
}

/************************************************************************
 * LSLCass_Record_Rx_HEC_Event_Channel
 *
 * Records RX HEC event on channel. This can be used in conjunction
 * with some other method, in this case RSSI contributes also.
 * REF: IEEE P802.15.2/Draft #09 March 24, 2003, 11.1.2 PER (page 40).
 *
 ************************************************************************/
void LSLCass_Record_Rx_HEC_Event_Channel(t_devicelink* p_dev_link)
{
    if (LMafh_LM_Read_AFH_Channel_Assessment_Mode() && LSLCass_Is_Assessment_State(p_dev_link))
    {
        if((p_dev_link->device_index) && (USLCchac_Get_Active_Procedure()==NO_PROCEDURE))
	    {
            t_freq rx_freq = LSLCfreq_Get_Frequency(RX_START);

            if(LSLCASS_IS_FREQ_VALID(rx_freq) /*&& LSLCASS_IS_FREQ_USED(rx_freq)*/)
            {
            	s_int8 rssi = HWradio_Read_RSSI(); /* refreshed via HWradio_Update_Internal_RSSI_Cache on SYNC DET */

                if(rssi > (p_dev_link->rssi_average + LSLCASS_GOLDEN_RECEIVER_RSSI_DEVIATION))
                {
                    LSLCass_Add_Channel_Interference(rx_freq, (rssi - p_dev_link->rssi_average));
                }
            }
        }
    }
}

/************************************************************************
 * LSLCass_Record_No_Rx_Packet
 *
 * Records no RX packet on channel.
 *
 * Can be used as implicit NAK packet acknowledgement. Used to assess
 * as interference in scenarios when packet was expected.
 * REF: IEEE P802.15.2/Draft #09 March 24, 2003, 11.1.4. Implicit NAK.
 *
 * For radios capable of monitoring for background interference, this
 * should be measured here in scenarios where packet was not expected.
 * REF: IEEE P802.15.2/Draft #09 March 24, 2003, 11.2.6. RF Scan.
 *
 ************************************************************************/
void LSLCass_Record_No_Rx_Packet(t_devicelink* p_dev_link)
{
    if (LMafh_LM_Read_AFH_Channel_Assessment_Mode() && LSLCass_Is_Assessment_State(p_dev_link))
    {
        if((p_dev_link->device_index) && (USLCchac_Get_Active_Procedure()==NO_PROCEDURE))
        {
            t_freq rx_freq = LSLCfreq_Get_Frequency(RX_START);

            if(LSLCASS_IS_FREQ_VALID(rx_freq) /*&& LSLCASS_IS_FREQ_USED(rx_freq)*/)
            {
                s_int8 bg_rssi = HWradio_Read_Background_RSSI();

                _assessment.background_rssi = bg_rssi;
                LSLCass_Update_Background_RSSI_Average(bg_rssi);

                if (LSLCass_Determine_Interference_Presence(bg_rssi))
                { /*probable interference detected on receiver*/
                    LSLCass_Add_Channel_Interference(rx_freq, (bg_rssi - LSLCASS_RSSI_MIN));
                }
            }
	    }
    }
}

/************************************************************************
 * LSLCass_Record_Rx_RSSI_Channel
 *
 * Records RX RSSI read on channel.
 *
 * Note: Based on RSSI it is possible to distinguish whether the channel
 * should be classified as bad either due to interference or propogation
 * effects. For example, if the packet has not been decoded successfully
 * and RSSI has been low the error(s) nature is propogation effects.
 * On the other hand if the packet has not been decoded successfully but
 * RSSI has been high the error(s) nature is interference.
 * REF: IEEE P802.15.2/Draft #09 March 24, 2003, 11.1.1 RSSI (page 40).
 *
 ************************************************************************/
void LSLCass_Record_Rx_RSSI_Channel(t_devicelink* p_dev_link, s_int8 rssi)
{
    if (LMafh_LM_Read_AFH_Channel_Assessment_Mode() && LSLCass_Is_Assessment_State(p_dev_link))
    {
        if((p_dev_link->device_index) && is_SCO_Packet(p_dev_link->packet_type))
	    { /* high RSSI without HEC/CRC - specifically used for SCO packets as do not have a CRC field */
            t_freq rx_freq = LSLCfreq_Get_Frequency(RX_START);

            if(LSLCASS_IS_FREQ_VALID(rx_freq) /*&& LSLCASS_IS_FREQ_USED(rx_freq)*/)
            {
                if(rssi > (p_dev_link->rssi_average + LSLCASS_GOLDEN_RECEIVER_RSSI_DEVIATION))
                {
                    LSLCass_Add_Channel_Interference(rx_freq, (rssi - p_dev_link->rssi_average));
                }
            }
        }
    }
}

/************************************************************************
 * LSLCass_Evaluate_Device_Channel_Map
 *
 * Evaluates the device channel map based on the historical assessments
 * of currently UNUSED channels, and recent assessments on USED channels,
 * and generates an updated channel map based on these evaluations.
 *
 ************************************************************************/ 
void LSLCass_Evaluate_Device_Channel_Map(void)
{
    u_int8 freq, freq_a1, freq_a2, iter;
    u_int8 LSLCass_side_interference[LSLCASS_MAX_FREQ+1];
    u_int32 total_interference, average_interference;
    boolean lower_freq_used, upper_freq_used;
    t_slots expirey_refresh;

    /*
     * Massage the interference in bands of 3 for all USED and adjacent channels, as
     * interferers typically span multiple channels. This also allows for adjacent channel
     * reassessment of UNUSED channels, as are updated via the adjacent USED channels.
     */

    for (freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    {
        lower_freq_used = ((freq > LSLCASS_MIN_FREQ) && LSLCASS_IS_FREQ_USED(freq-1));
        upper_freq_used = ((freq < LSLCASS_MAX_FREQ) && LSLCASS_IS_FREQ_USED(freq+1));

        freq_a1 = (lower_freq_used)?(freq-1):(freq);
        freq_a2 = (upper_freq_used)?(freq+1):(freq);

        if (lower_freq_used || upper_freq_used) 
        { /* update used side-channels */
            LSLCass_side_interference[freq] = LSLCass_freq_interference[freq_a1]
                + LSLCass_freq_interference[freq_a2];
        }
    }

    for (freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    {
        lower_freq_used = ((freq > LSLCASS_MIN_FREQ) && LSLCASS_IS_FREQ_USED(freq-1));
        upper_freq_used = ((freq < LSLCASS_MAX_FREQ) && LSLCASS_IS_FREQ_USED(freq+1));

        if (lower_freq_used || upper_freq_used) 
        { /* update as 50% centre channel + 25% per side channel */ 
            LSLCass_freq_interference[freq] = ((LSLCass_side_interference[freq]
                + (LSLCass_freq_interference[freq]<<1))+2)>>2;
        }
    }

    /*
     * Calculates the average interference on USED channels. This is used to determine the
     * location of banks of apparent interference, relative to the background average.
     */

    total_interference = 0;
    for (freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    {
        if (LSLCASS_IS_FREQ_USED(freq))
        {
            total_interference += LSLCass_freq_interference[freq];
        }
    }

    average_interference = (total_interference + ((79-LSLCass_num_channels_unused)/2))
        /(79-LSLCass_num_channels_unused);

    /*
     * For each bank of interference, update the expirey timer of each channel to the
     * latest expirey time in that bank of channels. As the presence of the interferer on
     * a channel typically indicates that the interferer is still present in that range.
     */

    for (freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    {
        if (LSLCass_freq_interference[freq] > average_interference)
        {
            expirey_refresh = LSLCass_expirey_slots[freq];
            for(iter = freq; (iter<=LSLCASS_MAX_FREQ) && (LSLCass_freq_interference[iter] > average_interference)
                && ((iter-freq) <= WIFI_CHANNEL_WIDTH); iter++)
            {
                if (!BTtimer_Is_Expired_For_Time(LSLCass_expirey_slots[iter], expirey_refresh))
                    expirey_refresh = LSLCass_expirey_slots[iter];
            }

            while (freq < iter)
            {
                LSLCass_expirey_slots[freq++] = expirey_refresh;
            }
        }
    }

    for (freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    { /* start to depreciate interference on expirey timeout */
        if (BTtimer_Is_Expired(LSLCass_expirey_slots[freq]) && (LSLCass_freq_interference[freq]))
        {
            lower_freq_used = ((freq > LSLCASS_MIN_FREQ) && LSLCASS_IS_FREQ_USED(freq-1));
            upper_freq_used = ((freq < LSLCASS_MAX_FREQ) && LSLCASS_IS_FREQ_USED(freq+1));

            if (LSLCASS_IS_FREQ_USED(freq))
            { /* immediate scrub history of expired used channels */
                LSLCass_freq_interference[freq]=0;
            }
            else if ((lower_freq_used || upper_freq_used) || (((freq)%(WIFI_CHANNEL_WIDTH/2))==0))
            { /* accelerated decay rate of unused side-channels & nominal anchor points */
                LSLCass_freq_interference[freq]=0; //>>=1;
            }
            //else
            //{ /* allow slow decay rate of all other unused channels */
            //    LSLCass_freq_interference[freq]--;
            //}
        }
    }

    /*
     * Calculates the average interference on all channels. This is used to determine the
     * threshold over which USED/UNUSED channels are compared for assessment.
     */

    total_interference = 0;
    for (freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    {
        total_interference += LSLCass_freq_interference[freq];
    }

    average_interference = (total_interference+40)/79;

#if 1 // TK 15/3/2012
    /*
     * If the background RSSI average indicates that there is no significant interferer, adjust
     * threshold by the golden receiver RSSI deviation to exclude less channels.
     */
    if (_assessment.background_rssi_average < LSLCASS_BACKGROUND_RSSI_AVERAGE_MIN_THRESHOLD)
        average_interference += LSLCASS_GOLDEN_RECEIVER_RSSI_DEVIATION;
#endif

    /*
     * Determine which channels should be USED and UNUSED for the next update based on
     * their relative interference, allowing for clustering of UNUSED channels.
     */

    for (freq=LSLCASS_MIN_FREQ; freq<=LSLCASS_MAX_FREQ; freq++)
    {
        if (LSLCass_freq_interference[freq] > (average_interference + LSLCASS_BG_RSSI_LO_THRESHOLD))
        { /* block use of channels with relatively high interference */
            if (LSLCASS_IS_FREQ_USED(freq) && (LSLCass_num_channels_unused < LSLCASS_MAX_UNUSED_CHANNELS))
            {
                LSLCass_expirey_slots[freq] = BTtimer_Set_Slots(LSLCASS_UNUSED_EXPIREY_SLOTS);
                LSLCASS_SET_FREQ_UNUSED(freq);
            }
        }
        else if (LSLCass_freq_interference[freq] <= average_interference)
        { /* reintroduce channels with relatively low interference */
            if (!LSLCASS_IS_FREQ_USED(freq))
            {
                LSLCass_expirey_slots[freq] = BTtimer_Set_Slots(LSLCASS_EXPIREY_SLOTS);
                LSLCASS_SET_FREQ_USED(freq);
            }
        }
        else if ((freq > LSLCASS_MIN_FREQ) && (freq < LSLCASS_MAX_FREQ))
        { /* attempt clustering of UNUSED channels where mid-range interference */
            if (!LSLCASS_IS_FREQ_USED(freq-1) && !LSLCASS_IS_FREQ_USED(freq+1))
            {
                if (LSLCASS_IS_FREQ_USED(freq) && (LSLCass_num_channels_unused < LSLCASS_MAX_UNUSED_CHANNELS))
                {
                    LSLCass_expirey_slots[freq] = BTtimer_Set_Slots(LSLCASS_UNUSED_EXPIREY_SLOTS);   
                    LSLCASS_SET_FREQ_UNUSED(freq);
                }
            }
        }
    }
}

/************************************************************************
 * LSLCass_Handle_Assessments
 *
 * Non-realtime processing of recently recorded assessment information.
 *
 ************************************************************************/ 
void LSLCass_Handle_Assessments()
{
    t_slots lslcass_channel_map_refresh_slots;

    if (LMafh_LM_Read_AFH_Channel_Assessment_Mode())
    {
        if (BTtimer_Is_Expired(_assessment.channel_map_refresh_timer))
        {
            LSLCass_Evaluate_Device_Channel_Map();
            LMafh_Device_Channel_Map_Update_Notification();

#if 1 // TK 15/3/2012
            if (_assessment.background_rssi_average >= LSLCASS_BACKGROUND_RSSI_AVERAGE_MAX_THRESHOLD)
                lslcass_channel_map_refresh_slots = (1600)*(5); // 5 seconds
            else if (_assessment.background_rssi_average >= LSLCASS_BACKGROUND_RSSI_AVERAGE_MIN_THRESHOLD)
                lslcass_channel_map_refresh_slots = (1600)*(10); // 10 seconds
            else
#endif
                lslcass_channel_map_refresh_slots = LSLCASS_CHANNEL_MAP_REFRESH_SLOTS; // 30 seconds

            _assessment.channel_map_refresh_timer = BTtimer_Set_Slots(lslcass_channel_map_refresh_slots);
        }
    }
}

/************************************************************************
 * LSLCass_Get_Latest_Background_RSSI
 *
 * Background RSSI noise. This may be used with random number generation.
 *
 ************************************************************************/ 
s_int8 LSLCass_Get_Latest_Background_RSSI(void)
{
    return _assessment.background_rssi;
}

#endif
