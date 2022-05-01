/******************************************************************************
 * MODULE NAME:    lslc_afh.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LSLC Adaptive Frequency Hopping
 * MAINTAINER:     John Nelson
 * DATE:           10 Feb 2003
 *
 * SOURCE CONTROL: $Id: lslc_afh.c,v 1.15 2011/11/24 15:42:12 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2003-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     10 Feb 2003 -      jn       - Initial Version based on V1.2 Draft2
 *
 * NOTES TO USERS:
 ******************************************************************************/

#include "sys_config.h"

#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)

#include "hc_event_gen.h"
#include "lc_types.h"
#include "lslc_hop.h"
#include "lslc_afh.h"
#include "dl_dev.h"
#include "bt_timer.h"
#include "hw_memcpy.h"
#include "hci_params.h"
#include "lmp_afh.h"

/*
 * Constant array for macros below to build 79 bit and 80 bit all on arrays
 * mLSLCafh_Get_79_Bit_Vector_Set_Ref(), mLSLCafh_Get_80_Bit_Vector_Set_Ref()
 */
const u_int8 _LSLCafh_channel_bit_vector[AFH_BYTES_PER_CHANNEL_MAP+1] =
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F };
   
/*
 * _LSLCafh_AFH_link required per link
 *
 * _LSLCafh_AFH_hse_kernel required per simultaneous active piconet
 */
t_afh_link _LSLCafh_AFH_link[MAX_ACTIVE_DEVICE_LINKS];
STATIC t_afh_kernel _LSLCafh_AFH_hse_kernel[PRH_BS_CFG_SYS_MAX_PICONETS];

u_int8 LMafh_Is_AFH_Change_Pending(t_deviceIndex device_index);


/************************************************************************
 * LSLCafh_Initialise
 *
 * Initialise all the specified AFH kernels, and the associated
 *
 ************************************************************************/
void LSLCafh_Initialise(void)
{
    t_deviceIndex index;

    /*
     * For each device, initialise the AFH engine to non-AFH,
     * (default to single piconet for non AFH capable compatibility)
     */
    for (index = 0; 
         index < MAX_ACTIVE_DEVICE_LINKS;
         index++)
    {
        LSLCafh_Assign_AFH_Kernel(0, index, 0x0000);
    }

    /*
     * For each piconet, initialise the HSE kernel
     */
    for (index = 0; 
         index < PRH_BS_CFG_SYS_MAX_PICONETS;
         index++)
    {
        t_afh_kernel *p_afh_kernel = _LSLCafh_AFH_hse_kernel + index;

        /*
         * Initialise all fields to zero first
         */
        hw_memset8(p_afh_kernel, 0x00, sizeof(t_afh_kernel));

        /*
         * Default to all frequencies supported.
         */
        hw_memcpy8(p_afh_kernel->afh_current_channel_map, 
                mLSLCafh_Get_79_Bit_Vector_Set_Ref(), AFH_BYTES_PER_CHANNEL_MAP);

        /*
         * Ensure that the mapping tables are correctly setup
         */
        p_afh_kernel->p_current_mapping_table = p_afh_kernel->afh_mapping_table_1;
        p_afh_kernel->p_next_mapping_table = p_afh_kernel->afh_mapping_table_2;
    }    
}

/************************************************************************
 * LSLCafh_Assign_AFH_Kernel
 *
 * Assign a kernel, and initialise the AFH link state to non_AHS.
 *
 * For each link there will be an associated AFH kernel (which in
 * turn has its own HSE kernel).
 *
 ************************************************************************/
void LSLCafh_Assign_AFH_Kernel(
     u_int8 piconet_index, t_deviceIndex device_index, t_uap_lap uap_lap)
{
    t_afh_link *p_afh_link = _LSLCafh_AFH_link + device_index;

    /*
     * The link starts of in non-AFH operating mode
     */
    p_afh_link->afh_current_mode = AFH_MODE_DISABLED;
    p_afh_link->afh_next_mode = AFH_MODE_EX_INACTIVE;

    /*
     * Associate afh kernel with the afh link
     */
    if (piconet_index >= PRH_BS_CFG_SYS_MAX_PICONETS)
    {
        HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_INVALID_PICONET_INDEX);
        piconet_index = 0;
    }
    p_afh_link->p_afh_kernel = _LSLCafh_AFH_hse_kernel + piconet_index;

    /*
     * Build the hop kernel for this AFH kernel
     */
    LSLChop_Build_Hop_Kernel(&p_afh_link->p_afh_kernel->hs_kernel, uap_lap);
}

/************************************************************************
 * LSLCafh_Backup_AFH_Link_State
 *
 * Backup the current link state for identified device_index
 * 
 * Uses:
 * In MSS to record the link state of the link before MSS in case
 * a recovery is required due to MSS failure.  
 * Since link state 0 is never used, it is a useful place to store!
 ************************************************************************/
void LSLCafh_Backup_AFH_Link_State(t_deviceIndex device_index)
{
    _LSLCafh_AFH_link[0] = _LSLCafh_AFH_link[device_index];
}

/************************************************************************
 * LSLCafh_Restore_AFH_Link_State
 *
 * Restore the current link state for identified device_index
 * 
 * Uses:
 * In MSS to restore the link state of the link where
 * a recovery is required due to MSS failure.  
 * Since link state 0 is never used, it is a useful place to store!
 ************************************************************************/
void LSLCafh_Restore_AFH_Link_State(t_deviceIndex device_index)
{
    _LSLCafh_AFH_link[device_index] = _LSLCafh_AFH_link[0];
}

/************************************************************************
 * LSLCafh_set_AFH
 *
 * Processes the AFH PDU establishing the mode, map and instant.
 * 
 * Used to initialised a Kernel, on connection setup/before MSS.
 ************************************************************************/
t_error LSLCafh_set_AFH(t_deviceIndex device_index,
                        const t_p_pdu p_LMP_set_AFH_PDU)
{
    t_afh_link *p_afh_link = _LSLCafh_AFH_link + device_index;
    t_afh_kernel *p_afh_kernel = p_afh_link->p_afh_kernel;
    t_error status = NO_ERROR;
    
    if (p_afh_link->afh_next_mode != AFH_MODE_EX_INACTIVE ||
        !p_afh_kernel)
    {
        /*
         * AFH command pending, instant not passed
         */
        status = LMP_PDU_NOT_ALLOWED;
    }
    else
    {
        /*
         * Count the used channels and build the mapping table
         * Even frequencies first then odd
         */
        u_int8 num_used_chns = 0;
        u_int8 chn, even_odd;
        u_int8 *p_mapping_entry;
        u_int8 afh_mode;

        p_mapping_entry = p_afh_kernel->p_next_mapping_table;

        for (even_odd = 0; even_odd < 2; even_odd++)
        {
            for (chn=even_odd; chn < AFH_N_MAX_USED_CHANNELS; chn+=2)
            {
                u_int8 channel_byte = p_LMP_set_AFH_PDU[5+(chn>>3)];
                if (channel_byte & (1<<(chn&0x07)))
                {
                    num_used_chns++;
                    *p_mapping_entry++ = chn;
                }
            }
        }

        /*
         * Validate the parameters:
         *    Mode must be 0 Disable, 1 Enable
         *    If mode=1 => Num Used Channels >= NUMBER MINIMUM USED CHANNELS
         */
        afh_mode = p_LMP_set_AFH_PDU[4];
        if (afh_mode > 1 || 
            (afh_mode == 1 && num_used_chns < AFH_N_MIN_USED_CHANNELS))
        {
            status = INVALID_LMP_PARAMETERS;
        }

        if (status == NO_ERROR)
        {
            /*
             * Translate the instant in slots to actual clock ticks
             */
            p_afh_kernel->afh_instant = 
                HCIparam_Get_Uint32(p_LMP_set_AFH_PDU) << 1;

            /*
             * Determine if we are effectively in AFH_79
             */
            if (num_used_chns == 79 && afh_mode == AFH_MODE_ENABLED)
            {
                p_afh_link->afh_next_mode = AFH_MODE_EX_79;
            }
            else
            {
                p_afh_link->afh_next_mode = afh_mode;

                /*
                 * Must engage possibility of AFH(79) recovery where
                 * we are performing an AFH update from a Master.
                 */
                if (p_afh_link->afh_next_mode == AFH_MODE_ENABLED && 
                    p_afh_link->afh_current_mode == AFH_MODE_ENABLED &&
                    DL_Get_Role_Peer(DL_Get_Device_Ref(device_index)) == SLAVE)
                {
                    p_afh_link->afh_next_mode = AFH_MODE_EX_ENABLED_ON_ACK;
                }
            }
            p_afh_kernel->afh_next_num_used_chns = num_used_chns;
            hw_memcpy8(p_afh_kernel->afh_next_channel_map, 
                p_LMP_set_AFH_PDU+5, AFH_BYTES_PER_CHANNEL_MAP);
        }
    }

    return status;
}

/************************************************************************
 * LSLCafh_Process_Baseband_Ack
 *
 * Process the baseband ACK.
 *
 * The baseband ACK if it occurs after HSSI, required a recovery 
 * procedure where an AFH update is taking place.
 * This effectively means using AFH(79) between HSSI and ACK.
 ************************************************************************/
void LSLCafh_Process_Baseband_Ack(t_deviceIndex device_index)
{
    t_afh_link *p_afh_link = _LSLCafh_AFH_link + device_index;

    if (p_afh_link->afh_next_mode == AFH_MODE_EX_ENABLED_ON_ACK)
    {
        p_afh_link->afh_next_mode = AFH_MODE_ENABLED;
    }
}

/************************************************************************
 * LSLCafh_Get_Frequency_Channel
 *
 * Get the AFH frequency if enabled.
 * Remaps to new frequency if necessary.  
 * Also updates to new table if instant has passed.
 *
 ************************************************************************/
t_freq LSLCafh_Get_Frequency_Channel(t_deviceIndex device_index, t_clock clk)
{
    t_afh_link *p_afh_link = _LSLCafh_AFH_link + device_index;
    t_afh_kernel *p_afh_kernel = p_afh_link->p_afh_kernel;
    t_freq afh_freq;
   
    if (p_afh_link->afh_next_mode != AFH_MODE_EX_INACTIVE)
    {
        /*
         * If AFH change pending AND at next change instant Then
         *      If Recovery Required Then
         *          Move to AFH(79)
         *      Else
         *          Select Next AFH mode 
         *      Endif
         * Endif
         */
        if (((p_afh_kernel->afh_instant-clk-1) & BT_CLOCK_MAX_TICKS)
                >= BT_CLOCK_MAX_TIMEOUT)
        {
            if ((p_afh_link->afh_next_mode == AFH_MODE_EX_ENABLED_ON_ACK)
                    && (LMafh_Is_AFH_Change_Pending(device_index)))
            {
                /*
                 * Must recover immediately to 79 mode
                 */
                p_afh_link->afh_current_mode = AFH_MODE_EX_79;
            }
            else
            {
                if((p_afh_link->afh_next_mode == AFH_MODE_EX_ENABLED_ON_ACK)
                    || (LMafh_Is_AFH_Change_Pending(device_index)))
                {
                    LMafh_LMP_Set_AFH_Ack(device_index);
                }
                p_afh_link->afh_current_mode = p_afh_link->afh_next_mode;
                p_afh_link->afh_next_mode = AFH_MODE_EX_INACTIVE;
            }

            /*
             * A kernel update must only be completed once per afh instant:
             * since multiple slaves and broadcast device may be associatated 
             * with the same kernel.
             */
            if (p_afh_kernel->afh_next_num_used_chns!=0)
            {
                p_afh_kernel->afh_current_num_used_chns = 
                    p_afh_kernel->afh_next_num_used_chns;
                p_afh_kernel->afh_next_num_used_chns = 0;

                hw_memcpy8(p_afh_kernel->afh_current_channel_map, 
                        p_afh_kernel->afh_next_channel_map, AFH_BYTES_PER_CHANNEL_MAP);

                /*
                 * Swap the mapping tables (fast pointer swap!)
                 */
                {
                    u_int8 *p_mapping_table_temp;

                    p_mapping_table_temp = 
                        p_afh_kernel->p_current_mapping_table;
                    p_afh_kernel->p_current_mapping_table =
                        p_afh_kernel->p_next_mapping_table;
                    p_afh_kernel->p_next_mapping_table =
                        p_mapping_table_temp;
                }

                /*
                 * The broadcast kernel should just toggle immediately to
                 * enabled (1)  or disabled (0), as requested.
                 *
                 * if (the kernel is the MASTER piconet) then
                 *   Enable broadcast AFH if current link is not DISABLED
                 *   Disable broadcast link next mode  
                 * endif
                 */
                if (p_afh_link->p_afh_kernel == &_LSLCafh_AFH_hse_kernel[0])
                {
                    t_afh_link *p_bc_afh_link = _LSLCafh_AFH_link + LOCAL_DEVICE_INDEX;

                    p_bc_afh_link->afh_current_mode = 
                        (p_afh_link->afh_current_mode != 0);
                    p_bc_afh_link->afh_next_mode = AFH_MODE_EX_INACTIVE;
                }
            }
        }
    }

    /*
     * First determine the assigned frequency to this slot
     */
    
    /*
     *  If AFH_Disabled Then
     *      Determine new non-AFH frequency
     *  Else
     *      If bit in channel map is used or mode is AFH(79) Then
     *          Return normal frequency
     *      Else
     *          Determine new AFH frequency
     *      Endif
     *  Endif
     */
    if ((p_afh_link->afh_current_mode == AFH_MODE_DISABLED)
		|| (LSLChop_Get_rfSelection() == SINGLE_FREQ))
    {
        afh_freq = LSLChop_Get_Frequency_Channel_Connection(
                        &p_afh_kernel->hs_kernel, clk);
    }
    else
    {
        if (clk & BIT1)
        {
            /*
             * Master Receive => Use previous Master Tx Frequency
             */
            afh_freq = p_afh_kernel->afh_current_master_tx_freq;
        }
        else
        {
            afh_freq = LSLChop_Get_Frequency_Channel_Connection(
                            &p_afh_kernel->hs_kernel, clk);
            /*
             * Master Transmit => Calculate/Store
             */
            if ( (p_afh_link->afh_current_mode != AFH_MODE_EX_79) &&
                 (p_afh_kernel->afh_current_channel_map[afh_freq>>3] &
                 (1<<(afh_freq & 0x07))) == AFH_CHANNEL_UNUSED)
            {
                /*
                 * Determine the AFH 
                 */
                u_int8  k_prime;
                t_hs_kernel *p_hs_kernel = &p_afh_kernel->hs_kernel;

#if ((PRH_BS_CFG_TABASCO_VERSION < PRH_BS_HW_TABASCO_VERSION_T1_0_0) || (BUILD_TYPE==UNIT_TEST_BUILD))
                /*
                 * V1.1 hardware has no support for AFH
                 */
                u_int32 hs_kernel_summation;
                
                if (LSLChop_Get_HSE_Engine() == LSLChop_HSE_SOFTWARE)
                {
                    /*
                     * Recalculate mod on previous stored calculation
                     */
                    hs_kernel_summation = p_hs_kernel->sum_before_mod;
                }
                else
                {
                    /*
                     * Must execute software Kernel function
                     */
                    hs_kernel_summation =  LSLChop_Execute_79_Hop_Kernel_Connection_Summation(
                        clk, p_hs_kernel);
                }
                /*
                 * Index into table based on k_prime = summation%num_of_used_channels
                 */
                k_prime = hs_kernel_summation % p_afh_kernel->afh_current_num_used_chns;
#else /*PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0*/
                /*
                 * The V1.2 hardware makes the sum_before_mod available
                 * Index into table based on k_prime = summation%num_of_used_channels
                 */
                k_prime =  p_hs_kernel->sum_before_mod % 
                    p_afh_kernel->afh_current_num_used_chns;
#endif
                afh_freq = p_afh_kernel->p_current_mapping_table[k_prime];
            }
            p_afh_kernel->afh_current_master_tx_freq = afh_freq;
        }
    }

    return afh_freq;
}

/****************************************************************************
 * LMafh_LM_Read_AFH_Channel_Map
 *
 * Description
 *  Invoked by the higher layers on the LMP to read the AFH
 *  Channel Map for the connection handle.
 *
 * Parameters
 *   device_index           The channel map for this device index.
 *   p_afh_mode             Where to store the afh mode for this device.
 *
 * Returns
 *   Pointer to the channel map.
 *
 * From V1.2 Draft Spec:
 *  This command will return the values for the AFH_Mode and
 *  AFH_Channel_Map for the specified Connection Handle. The
 *  Connection_Handle must be a Connection_Handle for an ACL connection.
 *  The returned values indicate the state of the hop sequence specified by the
 *  most recent LMP_Set_AFH message for the specified Connection_Handle,
 *  regardless of whether the master has received the baseband ACK or whether
 *  the HSSI has passed.
 *  This command shall be supported by a device that declares support for the
 *  AFH_capable feature.
 *
 *  Note: The above doesn't make sense.  The request always should return the 
 *        active and not the most recent LMP_Set_AFH message!
 *
 ****************************************************************************/ 
const u_int8* LSLCafh_LM_Read_AFH_Channel_Map(
    t_deviceIndex device_index, u_int8 *p_afh_mode)
{
    t_afh_link *p_afh_link = _LSLCafh_AFH_link + device_index;
    t_afh_kernel *p_afh_kernel = p_afh_link->p_afh_kernel;
    const u_int8 *p_afh_channel_map;

    /*
     * If not active Then
     *     Report DISABLED and all frequencies
     * If there is no pending change Then
     *     Report the current mode and channel map
     * Else
     *     Report the next mode and channel map
     * Endif
     */
    SYSdebug_Assert(p_afh_kernel!=NULL);

    if (p_afh_link->afh_next_mode == AFH_MODE_EX_INACTIVE)
    {
        *p_afh_mode = p_afh_link->afh_current_mode;
        if (*p_afh_mode == AFH_MODE_DISABLED)
        {
            p_afh_channel_map = mLSLCafh_Get_79_Bit_Vector_Set_Ref();
        }
        else
        {
            p_afh_channel_map = p_afh_kernel->afh_current_channel_map;
        }
    }
    else
    {
        *p_afh_mode = p_afh_link->afh_next_mode;
        p_afh_channel_map = p_afh_kernel->afh_next_channel_map;
    }

    if (*p_afh_mode == AFH_MODE_DISABLED)
    {
        /*
         * Always report the full bit vector when disabled
         * The specification reserves these bits without indication
         */
        p_afh_channel_map = mLSLCafh_Get_79_Bit_Vector_Set_Ref();
    }
    else 
    {
        /*
         * Always reduce to ENABLED since not DISABLED.
         */
        *p_afh_mode = AFH_MODE_ENABLED;
    }

    return p_afh_channel_map;
}

/****************************************************************************
 * LSLCafh_Count_1_Bits
 *
 * Description
 * Counts the number of bits which are 1 in a map/classification array
 *
 * Parameters
 *   bit_array              The array of 10 bytes containing 79 bits
 *
 * Returns
 *   The number of bits which are 1 in the bit_array arrgument
 *
 ****************************************************************************/ 
u_int8 LSLCafh_Count_1_Bits(const u_int8 bit_array[])
{
    u_int8 i;

    u_int8 num_bits_1 = 0;

    for (i = 0; i < AFH_N_MAX_USED_CHANNELS; i++)
    {
        u_int8 channel_byte = bit_array[i>>3];
        if (channel_byte & (1<<(i&0x07)))
        {
            num_bits_1++;
        }
    }
    return num_bits_1;
}

#else
/*
 * Completely empty source files are illegal in ANSI C
 */
void LSLCafh_Dummy(void);

#endif /* (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1) */
