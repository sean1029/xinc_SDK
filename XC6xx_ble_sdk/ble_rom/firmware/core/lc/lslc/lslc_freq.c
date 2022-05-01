/*************************************************************************
 * MODULE NAME:    lslc_freq.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LSLC_Frequency     Determine and Set Radio Frequency
 * MAINTAINER:     John Nelson
 * CREATION DATE:  19 Jun 2000
 *
 * SOURCE CONTROL: $Id: lslc_freq.c,v 1.102 2013/05/30 16:07:48 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * Notes:
 *    1. Extracted all frequency aspects from lslc_access.c
 *    2. Lookahead is always used
 *
 * ISSUES:
 *    1. This file should contain any radio specific code.
 *    2. Lookahead frequency calculation is only done for Connection
 ************************************************************************/
#include "sys_config.h"

#include "uslc_chan_ctrl.h"
#include "hw_radio.h"

#include "lslc_hop.h"
#include "lslc_freq.h"
#include "lslc_access.h"
#include "lc_interface.h"
#include "lmp_sco_container.h"

#include "hw_lc.h"
#include "hw_leds.h"

#include "lmp_config.h"

/*
 * Support Sequencing frequencies to radio using interrupts.
 * Frequencies corresponding to TX_START TX_MID RX_START RX_MID
 * The V1.1 TX_MID_SPR is redundant, RX_START used by radio driver,
 * but leave at 5 since all radio drivers not updated.
 */
t_freq _LSLCfreq_channel[4];

/* 
 * Accessor LSLCfreq_Get_Last_Frequency_Clock() which is used:
 * - to store the freq_clk for DWH Init
 * - to allow the Return to Piconet Tx freq to be referenced
 *   to the receive slot frequency (actual clock may be unstable)
 */
static t_clock LSLCfreq_last_freq_clk; 

/*
 * Clock value of next radio frequency if already calculated/set
 */
static t_clock _LSLCfreq_tx_lookahead_clock   /*= 0 */;
static t_freq  _LSLCfreq_tx_lookahead_freq_start /*= 0*/;
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
static t_deviceIndex _LSLCfreq_tx_lookahead_piconet_index /*= 0*/;
static u_int8 _LSLCfreq_tx_lookahead_afh_mode /*= 0*/;
#endif

/************************************************************************
 * LSLCfreq_Initialise
 *
 * Initialise any frequency related variables
 * 
 ************************************************************************/
void LSLCfreq_Initialise(void)
{
    /*
     * Set lookahead clock to invalid value
     */
    _LSLCfreq_tx_lookahead_clock = 1 << 28;  
}

/*****************************************************************************
 * LSLCfreq_Get_Frequency_Channel
 *
 * Get the last frequency calculated for the presented position
 *      Tx_Start   Tx_Mid       Rx_Start     Rx_Mid
 ****************************************************************************/
t_freq LSLCfreq_Get_Frequency(t_frame_pos freq_pos_required)
{
    return _LSLCfreq_channel[freq_pos_required];
}

/*****************************************************************************
 * LSLCfreq_Prepare_Frequency_Context
 *
 * Calculates the Tx or Rx frequencies and stores into the appropriate
 * positions in the frequency table.
 * Sets the clock registers appropriately.
 *
 * 1.   Interrupt Driven to load the next frequency from the table to hardware
 *      Interrupt  Loads frequency
 *      =========  ===============
 *      Rx_Mid     Tx_Start
 *      Tx_Start   Tx_Mid
 *      Tx_Mid     Rx_Start
 *      Rx_Start   Rx_Mid
 *
 * 2.   Clock values are used only for frequency calculation.
 *
 * 3.   Return to Piconet or Connection should never be handled by this 
 *      function, use:
 *          LSLCfreq_Prepare_Rx_Frequency_R2P()
 *          LSLCfreq_Prepare_Frequency_Connection()
 *
 *      The exception is that Connection can be used for Slave Park ID
 *
 * p_device_link    Pointer to active device link
 * context          State other than Return_To_Piconet
 ****************************************************************************/
void LSLCfreq_Prepare_Frequency_Context(const t_devicelink *p_device_link,
    t_frame_pos freq_pos_required, t_state context)
{
    t_clock   base_clk;
    t_clock   freq_clk;
    t_freq    freq_channel_start, freq_channel_mid;
    t_role    role_of_peer = DL_Get_Role_Peer(p_device_link);
    u_int     is_lookahead_complete;

    t_state   local_context;

    t_radio_freq_mask freq_mask = RADIO_FREQ_MASK(freq_pos_required);

    /*
     * Some sanity checks for V1.2 code changes
     */
    SYSdebug_Assert(context != Return_To_Piconet);
    SYSdebug_Assert(Return_To_Piconet != USLCchac_get_device_state()); 

    /*
     * The following are also Connection states
     */
    if (Master_Slave_Switch == context ||
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1) 
        Connection_Park_Access_Window_Rx == context || 
#endif
        Connection_2ndHalf_Slot_Tx == context )
    {
        local_context = Connection;
    }
    else
    {
        local_context = context;
    }

    /*
     * For BT_CLK register: write an offset from native, read the actual value.
     * The bt_clk_offset remains valid as long as Slave bit is set.
     * Bt clock [-Offset] is only appropriate if I'm a slave and
     *   state = Connection.
     */
    if (role_of_peer == MASTER && local_context == Connection)
    {
        base_clk = HW_get_bt_clk();
    }
    else  /* I'm a Master or Device is Scanning */
    {
        base_clk = HW_get_native_clk();   /* Default piconet clock        */
    }

    /* ASSUMPTION
     * ==========
     * If context is SlavePageResponse Then
     *    The correspoding Tx frequency is used (see lslc_hop.c)
     * Else If This device is Paging Then
     *    Clock for frequency is the Estimated Clock (CLKE = BT clock + Offset)
     * Else If (Not Master_Slave_Switch) EXOR    ! Opposite if MSS
     *   (This device is a Master and  requesting Transmit  OR
     *    This device is a Slave  and  requesting Receive) Then
     *    Clock for frequency is the next 00 slot
     * Else
     *    Clock for frequency is the next 10 slot
     * Endif
     *
     * Note that even as Slave where I've received ID in 2nd
     * half of slot with respect to the Master the preparation
     * below for FHS will assume that he is incorrectly going to be
     * received in the second half and
     * result in correct calculation of N for HOP.
     * It is also to be noted that the Hop calculator determines
     * the Y1/Y2 i.e. Rx from bit1 of clock. Hence, this must be valid
     * in both the SlavePageResponse and Inquiry Response state.
     *
     */
    if (local_context == Page || local_context == MasterPageResponse)
    {
        /*
         * Determine estimated clock of slave (CLKE) as base clock
         *
         * Can fix train frequencies by fixing page CLK[16:12]
         *        base_clk = (base_clk&0x0000FFF) |
         *          (DL_Get_Clock_Offset(pDevLink) & 0x001F000);
         */
        base_clk += DL_Get_Clock_Offset(p_device_link);
    }

    /*
     *   Role    Freq_pos   Slot
     *   ----    --------   ----
     *   MASTER  TX_START   Master Tx
     *   MASTER  RX_START   Master Rx      
     *   SLAVE   TX_START   Master Rx
     *   SLAVE   RX_START   Master Tx
     *  
     * For Master Slave Switch the Tx/Rx slots are transposed (hence XOR)
     * if ( (Master_Slave_Switch == context) ^
     *     (((role_of_peer == SLAVE)  && (freq_pos_required == TX_START) ) ||
     *      ((role_of_peer == MASTER) && (freq_pos_required == RX_START ))))
     */
    if ( (Master_Slave_Switch == context) ^
         ((role_of_peer == MASTER) ^ (freq_pos_required == TX_START)) )
    {
        /*
         * Determine next Master Tx (Slave Rx) slot start
         */
        freq_clk = ((base_clk+4) & 0xFFFFFFC );
    }
    else
    {
        /*
         * Determine next Master Rx (Slave Tx) slot start
         */
        freq_clk = ((base_clk+2) & 0xFFFFFFC ) + 2;
    }

    /*
     * Check for MSS delta between slave and master clocks at MSS switch
     * The BT_CLK is incremented mid-frame due to TDD
     * In effect the TDD adds on 625us causing an early TIM0 hence an
     * additional BT_CLK clock increment
     */
    if ( (Master_Slave_Switch == context) &&
         (role_of_peer == MASTER) && (freq_pos_required==TX_START))
    {
        freq_clk -= 4;
    }

#ifdef LC_ERROR_CHECK_PAGE_CLOCK
    /*
     * To allow basic test of frequency hop
     */
    if (local_context == Page)
    {
        freq_clk = LC_ERROR_CHECK_PAGE_CLOCK;
    }
#endif
#ifdef LC_ERROR_CHECK_SLAVE_SCAN_CLOCK
     /*
      * To allow basic test of frequency hop
      */
    if (local_context == Page_Scan || local_context == Inquiry_Scan)
    {
        freq_clk = LC_ERROR_CHECK_SLAVE_SCAN_CLOCK;
    }
#endif
#ifdef LC_ERROR_CHECK_USE_FIRST_HALFSLOT_ONLY
    /*
     * To allow basic test of frequency hop
     */
    if (local_context == Page_Scan || local_context == Inquiry_Scan)
    {
        freq_clk &= 0xFFFE000;
    }
#endif

    /*
     * The next radio TX_START frequency is if possible calculated in advance
     * for the Connection state.  If unused and a subsequent TX_START ensure
     * immediate calculation.
     */

    is_lookahead_complete = (freq_pos_required == TX_START &&
        _LSLCfreq_tx_lookahead_clock == freq_clk && 
        local_context == Connection);

    /*
     * Determine the slot start frequency and middle if necessary
     */
    if (is_lookahead_complete)
    {
        /*
         * Use pre-calculated frequencies
         */
        freq_channel_start = _LSLCfreq_tx_lookahead_freq_start;
        freq_channel_mid   = LSLChop_INVALID_FREQUENCY;
    }
    else
    {
        /*
         * Calculate the frequencies
         */
        if (local_context == Connection)
        {
            /*
             * Must use AFH engine if the local context is connection!!!!!!!
             */
            freq_channel_start = mLSLCfreq_Get_Frequency_Channel_Connection(
                p_device_link->device_index, freq_clk);
        }
        else
        {
            /*
             * Not a Connection frequency, no need for AFH
             */
            freq_channel_start = 
                LSLChop_Get_Next_Frequency(local_context, freq_clk);
        }

        /*
         * If half slots used then calculate the 2nd frequency.
         * Note that clock cannot wrap, no need to mask!
         */
#ifdef LC_ERROR_CHECK_USE_FIRST_HALFSLOT_ONLY
        /*
         * To allow basic test of frequency hop
         */
        if (local_context == Page)
        {
            freq_channel_mid = 80 /*Just out of range */;
            freq_mask |= RADIO_FREQ_MASK(freq_pos_required+1);
        }
        else
#endif

        if ((local_context == Page) || (local_context == Inquiry) || (local_context == MasterExtendedInquiryResponse))
        {
            /*
             * Half slot frequency must be calculated (no AFH impact)
             */
            freq_channel_mid = LSLChop_Get_Next_Frequency(local_context, freq_clk+1);
            freq_mask |= RADIO_FREQ_MASK(freq_pos_required+1);
        }
        else
        {
            freq_channel_mid = LSLChop_INVALID_FREQUENCY;
        }
    }

    /*
     * Store channel(s) in table for later delivery/extraction by logs etc
     */
    _LSLCfreq_channel[freq_pos_required] = freq_channel_start;
    _LSLCfreq_channel[freq_pos_required+1] = freq_channel_mid;              

    /*
     * Handle special cases
     */
    if (freq_pos_required == TX_START)
    {
        if (Connection_2ndHalf_Slot_Tx == context)
        {
           /*
            * The transmitter will be set to RX_MODE=2
            * (i.e. must be capable of transmitting an ID packet
            * in 2nd hal of Tx slot .
            * The freq used will be the same in both halves
            * (i.e. normal connection freq for this bt_clk).
            */
           _LSLCfreq_channel[TX_MID] = freq_channel_start;
           freq_mask |= RADIO_FREQ_MASK(TX_MID);
        }
    }
    else /* if (freq_pos_required == RX_START) */
    {         
        if (context == Inquiry)
        {
            LSLChop_Set_X_for_Inquiry(freq_clk & 0xFFFFFFE);
        }
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1) 
        else if (context == Connection_Park_Access_Window_Rx)
        {
            /*
             * In Park ID receive context:
             * The receiver will be set to RX_MODE=2 (i.e. must be capable
             * of receiving an ID packet in either half of the Rx slot.
             * The frequency used will be the same in both halves
             * (i.e. normal connection freq for this bt_clk).
             */        
            _LSLCfreq_channel[RX_MID] = freq_channel_start;
            freq_mask |= RADIO_FREQ_MASK(RX_MID);
        }
#endif
        /*
         * Calculate and set the next radio frequency for Connection 
         */
        if (context == Connection)
        {
            /*
             * Must use AFH engine for the Connection
             */
            _LSLCfreq_tx_lookahead_clock = (freq_clk+2) & 0xFFFFFFF;
            _LSLCfreq_tx_lookahead_freq_start = 
                mLSLCfreq_Get_Frequency_Channel_Connection(
                    p_device_link->device_index, _LSLCfreq_tx_lookahead_clock);
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
            _LSLCfreq_tx_lookahead_afh_mode = mLSLCafh_Get_AFH_Current_Mode(p_device_link->device_index);
            _LSLCfreq_tx_lookahead_piconet_index = p_device_link->piconet_index;
#endif
        }
        else
        {
            /*
             * Set clock to invalid value
             */
            _LSLCfreq_tx_lookahead_clock = 1 << 28;
        }
    }

    /*
     * Record the clock used for this frequency
     */
    LSLCfreq_last_freq_clk = freq_clk;

    if (Master_Slave_Switch == context)
    {
        if ((RX_START == freq_pos_required) && (SLAVE == role_of_peer))
        {
            /*
             * A bottleneck may be resolvable by calculating the ID TX 
             * in advance in case its late!
             */
            t_freq tx_lookahead_start =
                mLSLCfreq_Get_Frequency_Channel_Connection(
                    p_device_link->device_index, freq_clk +2);
            _LSLCfreq_channel[TX_START] = tx_lookahead_start;
            freq_mask |= RADIO_FREQ_MASK(TX_START);
        }
    }

    HWradio_SetFrequency(_LSLCfreq_channel, freq_mask);
}

/*****************************************************************************
 * LSLCfreq_Prepare_Rx_Frequency_R2P
 *
 * p_dev_link          piconet uap_lap for frequency hop engine
 * rx_freq_clk      bt clock for expected rx (master tx) frequency
 *
 * Calculates the Rx frequency and store into the appropriate
 * position in the frequency table.
 ****************************************************************************/
void LSLCfreq_Prepare_Rx_Frequency_R2P(
    const t_devicelink *p_device_link, t_clock rx_freq_clk)
{
    t_freq    freq_channel_start;

    freq_channel_start = mLSLCfreq_Get_Frequency_Channel_Connection(
            p_device_link->device_index, rx_freq_clk);

    _LSLCfreq_channel[RX_START] = freq_channel_start;

    /*
     * Store clock value, used for initialising the whitening registers
     */
    LSLCfreq_last_freq_clk = rx_freq_clk;

    /*
     * Calculate and set the next tx radio frequency
     */
    _LSLCfreq_tx_lookahead_clock = rx_freq_clk | 0x02;
    _LSLCfreq_tx_lookahead_freq_start = 
        mLSLCfreq_Get_Frequency_Channel_Connection(
            p_device_link->device_index, _LSLCfreq_tx_lookahead_clock);
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
    _LSLCfreq_tx_lookahead_afh_mode = mLSLCafh_Get_AFH_Current_Mode(p_device_link->device_index);
    _LSLCfreq_tx_lookahead_piconet_index = p_device_link->piconet_index;
#endif

    /*
     * Write frequency to serial sequencer for immediate delivery to radio.
     */
    HWradio_SetFrequency(_LSLCfreq_channel, RX_START_FREQ);
}

/*****************************************************************************
 * LSLCfreq_Early_Prepare_Frequency_FHS_Response_ID
 *
 * Setup the outgoing frequency for the response to a Rx FHS
 * This is necessary since response ID is sent on PKD
 * For MSS, this frequency is calculated on the pre-existing Connection
 ****************************************************************************/
void LSLCfreq_Early_Prepare_Frequency_FHS_Response_ID(void)
{
    switch (USLCchac_get_device_state())
    {
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
        case Master_Slave_Switch:
        {
            /*
             * Replaces: _LSLCfreq_Prepare_Frequency_MSS_FHS_Response_ID_
             *
             * Setup the outgoing frequency for the response to a Rx FHS
             * This is necessary since response ID is sent on PKD
             * For MSS, this frequency is calculated on pre-existing Connection
             */
            LSLCfreq_Prepare_Frequency_Context(
                DL_Get_Local_Device_Ref(), TX_START, Master_Slave_Switch);
            break;
        }
#endif
        case SlavePageResponse:
        {
            /*
             * Replaces: _LSLCfreq_Prepare_Frequency_Page_FHS_Response_ID_
             *
             * Setup the outgoing frequency for the response to a Rx FHS
             * This is necessary since response ID is sent on PKD
             * Same X as FHS receive but TX frequency for slave.
             */
             _LSLCfreq_channel[TX_START] = 
                    LSLChop_Get_Table_Frequency_Page_FHS_Response_ID();
             HWradio_SetFrequency(_LSLCfreq_channel, TX_START_FREQ);
            break;
        }

        default:
            break;
    }
}

/*****************************************************************************
 * LSLCfreq_Early_Prepare_TxRxFreqs_SlavePageResponse
 *
 * Setup the outgoing frequency for the response to a Rx ID and the
 * incoming frequency for the FHS packet 312.5us/625us subsequent.
 ****************************************************************************/
void LSLCfreq_Early_Prepare_TxRxFreqs_SlavePageResponse(void)
{    
    HWradio_SetRadioMode(RADIO_MODE_SLAVE_PAGE_RESP);
    LSLChop_Get_TxRxFreqs_SlavePageResponse(&(_LSLCfreq_channel[TX_START]),
                                            &(_LSLCfreq_channel[RX_START]));
    HWradio_SetFrequency(_LSLCfreq_channel, TX_START_FREQ|RX_START_FREQ);
}

/*****************************************************************************
 * LSLCfreq_Get_Last_Frequency_Clock
 *
 * Get the clock used for the last frequency calculation
 ****************************************************************************/
t_clock LSLCfreq_Get_Last_Frequency_Clock(void)
{
   return LSLCfreq_last_freq_clk;
}

/*****************************************************************************
 * LSLCfreq_Prepare_Frequency_Connection
 *
 * Calculates the Tx or Rx frequencies and stores into the appropriate
 * positions in the frequency table for the Connection state only
 * Sets the clock registers appropriately.
 *
 * 1.   Interrupt Driven to load the next frequency from the table to hardware
 *      Interrupt  Loads frequency
 *      =========  ===============
 *      Rx_Mid     Tx_Start
 *      Tx_Start   Tx_Mid
 *      Tx_Mid     Rx_Start
 *      Rx_Start   Rx_Mid
 * 2.   Clock values are used only for frequency calculation.
 *
 * 3.   Always calculates the next Rx/Tx frequencies
 ****************************************************************************/
void LSLCfreq_Prepare_Frequency_Connection(
    const t_devicelink *p_device_link, t_frame_pos freq_pos_required)
{
    t_clock   base_clk;
    t_clock   freq_clk;
    t_freq    freq_channel_start;
    t_role    role_of_peer = DL_Get_Role_Peer(p_device_link);

#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
    u_int8    afh_mode = mLSLCafh_Get_AFH_Current_Mode(p_device_link->device_index);
#endif

    /*
     * Note Role of peer has to be used to determine the appropiate clk.
     * The correct clock is critical to allow SCOs during scans.
     */
     
    if (role_of_peer == MASTER)
    {
#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
        if ( (1== LC_Get_SCO_Repeater_Bit()) && (LMscoctr_Get_Number_SCOs_Active() == 2))
        {
            base_clk = LC_Get_Piconet_Clock(LC_Get_Active_Piconet());
        }
        else
#endif
        {
            base_clk = HW_get_bt_clk();
        }
    }
    else  /* I'm a Master or Device is Scanning */
    {
        base_clk = HW_get_native_clk();   /* Default piconet clock        */
    }

    /* 
     * if (SLAVE AND TX_START) OR (MASTER AND RX_START) then
     *   Determine next MASTER Tx
     * else
     *   Determine next Master Rx
     * endif
     */  
    if ( (role_of_peer == MASTER) ^ (freq_pos_required == TX_START) )
    {
        /*
         * Determine next Master Tx (Slave Rx) slot start
         */
        freq_clk = ((base_clk+4) & 0xFFFFFFC );
    }
    else
    {
        /*
         * Determine next Master Rx (Slave Tx) slot start
         */
        freq_clk = ((base_clk+2) & 0xFFFFFFC ) + 2;
    }

#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
    /*
     * B1620/1621
     * In Return To Piconet the TX frequency is already calculated
     * in LSLCfreq_Prepare_Rx_Frequency_R2P().
     * Need update freq_clk since the bt clock may not have 
     * recovered before getting to here.
     */
    if ( (Return_To_Piconet == USLCchac_get_device_state()) 
        && (freq_pos_required == TX_START) )
    {
        t_packet rx_packet_type = (t_packet)HW_get_rx_type();

        freq_clk = LSLCfreq_last_freq_clk | 2;
                
        /* Adjust freq_clk for Multislot R2P procedure */      
        if(is_Packet_3_Slot(rx_packet_type))
        {
            freq_clk += 4;
        }
        else if(is_Packet_5_Slot(rx_packet_type))
        {
            freq_clk += 8;
        }
    }
#endif

    /*
     * The next radio TX_START frequency is if possible calculated in advance
     * for the Connection state.  If unused and a subsequent TX_START ensure
     * immediate calculation.
     *
     * When AFH capable, do not use lookahead across autonomous hop selections
     * due to hop mode or piconet channel map.
     */
    if ((_LSLCfreq_tx_lookahead_clock == freq_clk)
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
     && (_LSLCfreq_tx_lookahead_afh_mode == afh_mode)
     && (_LSLCfreq_tx_lookahead_piconet_index == p_device_link->piconet_index)
#endif
       )
    {
        /*
         * Use pre-calculated frequencies
         */
        freq_channel_start = _LSLCfreq_tx_lookahead_freq_start;
    }
    else
    {
        freq_channel_start = mLSLCfreq_Get_Frequency_Channel_Connection(
            p_device_link->device_index, freq_clk);
        /*
         * Calculate and set the next radio frequency
         */
        if (freq_pos_required == RX_START)
        {
            _LSLCfreq_tx_lookahead_clock = (freq_clk+2) & 0xFFFFFFF;
            _LSLCfreq_tx_lookahead_freq_start = 
                mLSLCfreq_Get_Frequency_Channel_Connection(
                    p_device_link->device_index, _LSLCfreq_tx_lookahead_clock);
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
            _LSLCfreq_tx_lookahead_afh_mode = afh_mode;
            _LSLCfreq_tx_lookahead_piconet_index = p_device_link->piconet_index;
#endif
        }
    }

     /*
      * Store in table for later extraction by logs etc
      */
     _LSLCfreq_channel[freq_pos_required] = freq_channel_start;
     _LSLCfreq_channel[freq_pos_required+1] = LSLChop_INVALID_FREQUENCY;

    /*
     * Record the clock used for this frequency and program radio
     */
    LSLCfreq_last_freq_clk = freq_clk;
    HWradio_SetFrequency(_LSLCfreq_channel, RADIO_FREQ_MASK(freq_pos_required));
}

/*****************************************************************************
 * LSLCfreq_Assign_Frequency_Kernel
 *
 * Assign and setup the either normal hop or the afh kernel functions
 *
 * AFH:         LSLCafh_Assign_AFH_Kernel
 * Normal hop:  LSLChop_Setup_Hop_Kernel
 *
 ****************************************************************************/
void LSLCfreq_Assign_Frequency_Kernel(const t_devicelink *p_device_link)
{
    t_uap_lap master_uap_lap;

    if (DL_Get_Role_Peer(p_device_link)==SLAVE)
    {
        /*
         * Master UAP_LAP is always in local device link
         */
        master_uap_lap = DL_Get_UAP_LAP(DL_Get_Local_Device_Ref());
    }
    else
    {
        master_uap_lap = DL_Get_UAP_LAP(p_device_link);
    }

#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
    LSLCafh_Assign_AFH_Kernel(
        DL_Get_Piconet_Index(p_device_link),
        DL_Get_Device_Index(p_device_link),
        master_uap_lap);

#elif (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==0)
    LSLChop_Setup_Hop_Kernel(master_uap_lap);
#endif
}

/*****************************************************************************
 * LSLCfreq_Prepare_Frequency_FirstFHS_MasterPageResponse
 *
 * Setup the outgoing frequency for the first FHS in master page response.
 ****************************************************************************/
#if (PRH_BS_DEV_EARLY_PREPARE_PAGE_ID_FHS_TURNAROUND==1)
void LSLCfreq_Prepare_Frequency_FirstFHS_MasterPageResponse(t_clock base_clk)
{
    t_freq ch = LSLChop_Get_Frequency_FirstFHS_MasterPageResponse(base_clk);

    //HWradio_Assign_TX_START_Channel(ch); /* for MIPS compromised scenario */

    HWradio_SetRadioMode(RADIO_MODE_TX_RX);

    _LSLCfreq_channel[TX_START] = ch;
    _LSLCfreq_channel[TX_MID] = LSLChop_INVALID_FREQUENCY;

    HWradio_SetFrequency(_LSLCfreq_channel, TX_START_FREQ);

    LSLCfreq_last_freq_clk = ((base_clk+4) & 0xFFFFFFC);
}
#endif
