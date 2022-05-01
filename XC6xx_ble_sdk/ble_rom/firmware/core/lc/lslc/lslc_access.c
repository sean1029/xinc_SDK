/*************************************************************************
 * MODULE NAME:    lslc_access.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LSLC_Access
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  1 Jun 1999
 *
 * SOURCE CONTROL: $Id: lslc_access.c,v 1.456 2013/10/25 16:45:27 tomk Exp $
 * 
 * LICENSE:
 *     This source code is copyright (c) 1999-2004 Ceva Inc.
 *     All rights reserved.
 *
 * ISSUES:
 *    The direct bit setting approach is much too inefficient, both in terms
 *    of memory and processor execution.
 *    1. Suggest a set of 32 bit registers where the control word is
 *       established in a local register and written once.
 *       Reimplement as macros.
 *    2. Map register to fixed area of memory.
 *    3. ******** MAJOR  jn 12/3/2000 **********
 *       Must not write COM_CTRL_REGS until all relevant info is used
 *       for receive.  Hence, now cached.
 *
 *
 * NOTES TO USERS:
 *    V2.01    17 Mar 2000   Complete rewrite of lslc_access to support
 *                           lslc slot handler, fast access to hardware
 *                           via cached registers, and shadowed common
 *                           control registers.
 *    V2.02    23 Mar 2000   Added tx and rx ctrl to shadow registers
 *    V2.03    19 Jun 2000   Extracted freq setting to new file lslc_freq.c
 *
 ************************************************************************/

#include "sys_config.h"
#include "lc_types.h"

#include "dl_dev.h"

#include "tra_queue.h"

#include "hw_lc.h"
#include "hw_memcpy.h"
#include "hw_register.h"
#include "hw_leds.h"
#include "hw_radio.h"

#include "uslc_chan_ctrl.h"
#include "uslc_testmode.h"
#include "uslc_inquiry.h"
#include "uslc_scheduler.h"


#include "lslc_access.h"
#include "lslc_hop.h"
#include "lslc_stat.h"
#include "lslc_clk.h"
#include "lslc_freq.h"
#include "lslc_class.h"

#include "lc_interface.h"               /* Use for LC Indirect Callbacks */

#if (PRH_BS_CFG_SYS_CHANNEL_ASSESSMENT_SCHEME_SUPPORTED==1)
#include "lmp_afh.h"
#endif
#include "lslc_pkt.h"
#include "bt_test.h"

#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)

static void _LSLCacc_Loopback_Testmode_Payload(t_devicelink *p_dev_link, 
            t_packet packetType, t_length packet_length);
#endif

#include "bt_pump.h"                    /* Optional internal data pump  */
#include "bt_tester.h"                  /* Optional internal testmode tester */
#include "bt_timer.h"
#include "hc_flow_control.h"

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
#include "bt_mini_sched.h"
#endif

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
#include "lmp_sco_container.h"
#endif

#if (PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
#include "lmp_link_policy.h"
#endif

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
#include "hw_pta.h"
#include "lmp_acl_container.h"
#endif

#include "sys_debug_config.h"

extern t_queue BTQueues[];

/*
 * Private functions
 */
#ifndef BLUETOOTH_MODE_LE_ONLY
static boolean _is_end_eSCO_window(t_sco_info *p_sco_link, t_devicelink* pDL, 
                                           t_clock  current_clk);
static t_TXRXstatus
   _LSLCacc_Extract_Payload(t_devicelink *p_dev_link, t_packet packetType);
static t_TXRXstatus
   _LSLCacc_Process_Rx_CRC(t_devicelink *p_dev_link, t_packet packetType);
static void
   _LSLCacc_Prepare_Tx_Complete_If_Same_Device(t_devicelink *p_dev_link);
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
static void
   _LSLCacc_Handle_Erroneous_SCO_Packet(t_devicelink* p_dev_link);
#endif
#endif//BLUETOOTH_MODE_LE_ONLY

/*
 * Select function for copying to/from ACL buffers
 */
#if (PRH_BS_DEV_BIG_ENDIAN_AND_DIRECT_BASEBAND_MOVE==1)
#define _LSLCacc_memcpy_ACL     hw_memcpy32_transparent
#else
#if (PRH_BS_CFG_SYS_ACL_BUFFER_ALIGNMENT==16)
    /*
     * Optimised for slower boards, move 16 bytes at a time
     * Note loop overhead is similar to a move/store (hence unroll).
     */ 
#define _LSLCacc_memcpy_ACL     hw_memcpy128
#else
#define _LSLCacc_memcpy_ACL     hw_memcpy32
#endif
#endif
#if ((LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED==1) || (LC_DEFERRED_LOOPBACK_PAYLOAD_EXTRACTION_SUPPORTED==1))
u_int16 G_rx_loopback_len;
u_int8  G_msg_type;
#endif
#if 0 // debug only
static u_int8 test_nak_next_esco=1;
#endif
/*
 *  Select function for copying synchronous data (SCO/eSCO) to/from ACL buffers
 *  Prior to version T1 of the Tabasco hardware, only 16 bits out of every 32 bits of
 *  the ACL buffer were used for SCO. 
 *
 *  From T1 version every 32 bits out of 32 are used.
 *  Segmented SCO or eSCO is allowed in the current implementation.
 *  However, _LSLCacc_memcpy() routines have to be able to cope
 *  with copying from an 8 bit aligned start to a 32 bit aligned
 *  destination. However, not vice versa as no re-assembly takes place
 *  in the SCO queueing system. Each new SCO buffer in the queue is 32-bit
 *  aligned.
 */
#define _LSLCacc_memcpy_SCO_to_ACL         hw_memcpy_byte_to_word32
#if     (__DEBUG_PCM_VOICE__ == 1)
#define _LSLCacc_memcpy_SCO_from_ACL     hw_memcpy32
#else
#define _LSLCacc_memcpy_SCO_from_ACL     hw_memcpy8  //hw_memcpy32 // TK 07/01/2013 fix for BQB
#endif

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
void _LSLCacc_Non_EDR_Pkt_Tx_Rx(void);
void _LSLCacc_Set_PTT(const t_devicelink *p_dev_link);
#endif

u_int8 LSLCacc_last_rx_LMP_msg;

/*
 * Lookup table for t_packet size in slots used by Packet_Slots() macro.
 */
const u_int8 LSLCacc_packet_slots[] = {1,1,1,1,1,1,1,1,1,1,3,3,3,3,5,5};

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/* 
 * SCO CFG values are stored here to allow LC Interface call from LM to 
 * synchronously set the values.
 * No point storing in Codec Driver since each codec is HAL specific 
 * and this is common code
 */
static volatile u_int16 lslc_acc_shadow_sco_cfg_; /* 12 bit SCO cfg values */
#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
static volatile u_int8 lslc_acc_shadow_sco_route_; /* sco_route value for each
                                                      of the 3 possible SCOs */ 
#endif
#endif

#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
static boolean lslc_acc_recent_rssi_available_;
#endif

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
u_int8 edr_tx_edr_delay, edr_rx_edr_delay;
u_int8 edr_sync_error=0;
#endif
#if ((LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED==1) || (LC_DEFERRED_LOOPBACK_PAYLOAD_EXTRACTION_SUPPORTED==1))
#ifndef BLUETOOTH_MODE_LE_ONLY
/*
 * Code to defer copying Rx'd data from ISR into mainloop
 */
volatile static boolean deferred_extraction_pending = FALSE;
volatile static boolean deferred_extraction_enabled = TRUE;

static t_devicelink *p_deferred_dl;
/*static*/ t_packet deferred_pkt_type;

extern t_queue BTQueues[];

void LSLCacc_Enable_Deferred_Extraction(boolean status)
{
    deferred_extraction_enabled = status;
}

__INLINE__ void LSLCacc_Setup_Deferred_Extraction_(t_devicelink *p_dev_link, t_packet pkt_type)
{
    p_deferred_dl = p_dev_link;
    deferred_pkt_type = pkt_type;
}

__INLINE__ boolean LSLCacc_Is_Deferred_Extraction_Enabled_(void)
{
    return deferred_extraction_enabled;
}

__INLINE__ void LSLCacc_Set_Deferred_Extraction_Pending_(boolean status)
{
    if (LSLCacc_Is_Deferred_Extraction_Enabled_() == TRUE)
    {
        deferred_extraction_pending = status;
    }
}

__INLINE__ boolean LSLCacc_Is_Deferred_Extraction_Pending_(void)
{
    return deferred_extraction_pending;
}

void LSLCacc_Perform_Deferred_Payload_Extraction_If_Reqd(void)
{
    if (LSLCacc_Is_Deferred_Extraction_Pending_())
    {
        t_TXRXstatus status;
        mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG);
        mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);

        status = _LSLCacc_Extract_Payload(p_deferred_dl, deferred_pkt_type);

        /*
         * can check based on context if packet type is still valid
         */
        if (RX_OK != status)
        {
            deferred_pkt_type = INVALIDpkt;
        }

        /*
         * FLOW = GO (1) if local queue is not full or
         *  if the local queue was full and now is empty
         */
        if (DL_Is_Local_Buffer_Full(p_deferred_dl))
        {
            if (BTq_Is_Queue_Not_Full(L2CAP_IN_Q, DL_Get_Device_Index(p_deferred_dl)))
            {
                DL_Set_Local_Rxbuf_Full(p_deferred_dl, FALSE);
            }
        }
        else                                       /* GO 1 */
        {
             DL_Set_Local_Rxbuf_Full(p_deferred_dl, TRUE);
        }

        LSLCacc_Set_Deferred_Extraction_Pending_(FALSE);
    }
}
#endif//BLUETOOTH_MODE_LE_ONLY
#endif

/*
 * Define a shadow set of Jalapeno Control registers to allow advanced
 * preparation of transmit packets or preparation to receive.
 * The hardware CC registers cannot be set until the current packet is tx/rx.
 */
typedef struct s_ctrl_reg_shadow
{
   u_int32 sync_low;
   u_int32 sync_high;
   u_int32 gp1;
   u_int32 gp2;
   u_int32 tx_ctrl;
   u_int32 esco_ctrl;
   u_int32 rx_ctrl;
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
   u_int32 edr_ctrl;
#endif
   const u_int8  *p_encryption_key;
   u_int8  encryption_key_length;
} t_ctrl_reg_shadow;

static t_ctrl_reg_shadow _ctrl_reg_shadow;
static boolean _was_previous_rx_slot_a_scan;

/*****************************************************************************
 * LSLCacc_Set_Am_Addr
 *
 * Set up the AMADDR for both Tx/Rx in Common Control Register (shadow)
 ****************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
void LSLCacc_Set_Am_Addr(t_am_addr am_addr)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG,JAL_AM_ADDR, am_addr );
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);
}


/*****************************************************************************
 * LSLCacc_Prepare_Tx_ID
 *
 * Prepare Transmit Hardware to transmit ID packet in 1st half of slot
 * Build all shadow control registers
 *
 * Parameters:
 * context         Inquiry | Page | Page_Scan | SlavePageResponse
 *
 ****************************************************************************/
void LSLCacc_Prepare_Tx_ID(t_state context)
{
    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP2_REG);
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
    u_int tx_buf;

    /*
     * Load all registers from hardware LC
     */
    mHWreg_Load_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP2_REG);

    /*
     * Transmit Control Register:  Clear all except TX_BUF, and set TX_MODE only
     */
    tx_buf = mHWreg_Get_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);
    mHWreg_Clear_Cache_Register(JAL_TX_CTRL_REG);
    if (tx_buf)
        mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);

    if (context==Page_Scan || context==SlavePageResponse
        || context==Master_Slave_Switch || context==Connection)
    {
        /*
         * Based on email LKO to CM 21Mar00  TX_MODE=1 not 2!
         */
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE,
                              TXm_NORMAL);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
    }
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    else if (context == Connection_2ndHalf_Slot_Tx)
    {
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE,
                              TXm_2ND_HALF_SLOT);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
    }
#endif
    else
    {
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE,
                              TXm_SLAVE_1ST_HALF__MASTER_BOTH);

        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
    }

    mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    /*
     * GP1 Common Control Register:  Define Only Page, Whiten and Encrypt Bits
     *                               Slave bit defined only above.
     *                               Clear AM_ADDR for readability.
     */

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_CRC_INIT, 0);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_ENCRYPT, 0);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_AM_ADDR, 0);

    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);

    /*
     * GP2 Common Control Register:  Clear All DWH registers
     * This code is not necessary but improves readability of registers
     */
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH_ALL_REG, 0);

    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP2_REG, p_reg_shadow->gp2);

    LSLCstat_Inc_Tx_Packet_Cntr(IDpkt);
}
#endif//BLUETOOTH_MODE_LE_ONLY
/*
 * Adjustment necessary for setting X input Bit[5] to 1 during FHS Tx/Rx
 */
#define LSLC_DWH_ADJUST_FHS    0x20
/*****************************************************************************
 * LSLCacc_Prepare_Tx_Rx_FHS
 *
 * Prepare Transmit/Receive Hardware to transmit or receive FHS packet.
 * Also used to prepapare EIR packets, which are treated largely similar.
 *
 * Parameters:
 * context         Transmit: MasterPageResponse | InquiryResponse
 *                       Receive:  Inquiry | SlavePageResponse
 * crc_hec_init          Initialisation of CRC/HEC
 ****************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
static u_int _last_fhs_X_input = 0;

void LSLCacc_Prepare_Tx_Rx_FHS(t_state context, u_int crc_hec_init)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP2_REG);

    /*
     * GP Common Control Register:
     *  Page           0
     *  Slave          1  Rx (InquiryResponse | SlavePageResponse)
     *                 0  Tx (Inquiry | MasterPageResponse)
     *  Encrypt        0
     *  Whitening      From system register
     *  AM_ADDR        0 FHS(Page), 0 FHS(Inquiry Scan)
     *                 Setup in LSLCacc_Set_Am_Addr
     *  CRC_INIT       InquiryResponse: DCI, MPR: UAP of Slave
     *  DWH_INIT
     */
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_ENCRYPT, 0);

    if (context==MasterPageResponse || context==Inquiry || context==MasterExtendedInquiryResponse)
    {
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
    }
    else
    {
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
    }
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_CRC_INIT, crc_hec_init);
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, _ctrl_reg_shadow.gp1);


    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP2_REG);

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP2_REG, JAL_WHITEN);

    if (context!=MasterExtendedInquiryResponse)
    {
        _last_fhs_X_input = LSLChop_Get_X();
    }

    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH_INIT, _last_fhs_X_input
        | LSLC_DWH_ADJUST_FHS );
    /*
     * Set second half
     * Data whitening and frequency setting should be combined!
     * Not complete for RF23
     *
     * if ( clkn15_12 - clkn4_2__0 == 1) then X mid delta = 1 else = 17 endif
     * i.e.
     * if ( X_start has (-1) mod 16 then X_mid will have 0 mod 16 hence delta
     * requires + 16 adjustment before incrementing!
     * All mod 32!
     * Note register in GP1
     */

    if ( context==Inquiry || context==MasterExtendedInquiryResponse)
    {
        t_clock clkn = (HW_get_native_clk() >> 2 ) << 2;
        u_int8  X_mid = _last_fhs_X_input + 1;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
        /* EIR use native_clk-4 as X input relates to FHS from prev frame */
        if (context==MasterExtendedInquiryResponse)
            clkn -= 4;
#endif

        /*
         * Adjust if ( clkn15_12 - clkn4_2__0 == 1 at start of Rx slot)
         * Note, no need for clock change to Rx slot 2nd half
         *       since CLKN[1] not used and CLKN[0] is 1 in 2nd half slot
         */
        if ( ((clkn>>12)&0x0F) == ( ((clkn>>1)&0xE) | 1 ) )
        {
            X_mid ^= 0x10;              /* Add 16 Toggle Bit 4        */
        }
        mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH2_INIT, X_mid
        | LSLC_DWH_ADJUST_FHS );
    }
    else
    {
        /*
         * Shouldn't be required in Slave Page Response 2nd half receive
         */
        mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH2_INIT,
         LSLChop_Get_X() | LSLC_DWH_ADJUST_FHS );
    }

    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP2_REG, _ctrl_reg_shadow.gp2);
}
#endif//BLUETOOTH_MODE_LE_ONLY
/*****************************************************************************
 * LSLCacc_Prepare_Tx_Rx_Encryption
 *
 * Prepare The Cache with the Encryption Key Address and Length
 ****************************************************************************/
void LSLCacc_Prepare_Tx_Rx_Encryption(
     const u_int8 *p_encryption_key, u_int8 encryption_key_length)
{
    _ctrl_reg_shadow.p_encryption_key = p_encryption_key;
    _ctrl_reg_shadow.encryption_key_length = encryption_key_length;
}

/*****************************************************************************
 * LSLCacc_Prepare_Tx_Rx_Connection
 *
 * Prepare Transmit/Receive Hardware to transmit or receive
 * ACL, SCO, NULL, POLL packets.   The device state is Connection.
 *
 * Parameters:
 * role_of_peer         MASTER | SLAVE
 * crc_hec_init          Initialisation of CRC/HEC
 * encrypt_mode          ENCRYPT_(NONE|NON_BROADCAST|BROADCAST|ALL)
 * frame_position       TX_START | RX_START
 ****************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
void LSLCacc_Prepare_Tx_Rx_Connection(const t_devicelink *p_dev_link,
                                      u_int crc_hec_init,
                                      t_encrypt_mode encrypt_mode,
                                      t_state context)
{
    t_role role_of_peer = DL_Get_Role_Peer(p_dev_link);
#ifdef SYS_DEBUG_INSERT_HEC_ON_ESCO_TX
	extern u_int8 eSCO_Active;
#endif
    /*
     * GP Common Control Register:
     *  Page           0
     *  Slave          role_of_peer == MASTER: 1
     *  Encrypt        encrypt
     *  Whitening      From system register
     *  AM_ADDR        Rx  Set by call to LSLCacc_Set_Am_Addr() in invoker.
     *                 Tx  Set in LSLCacc_Setup_Packet_Header()
     *  CRC_INIT       as selected.
     *  DWH_INIT       Automatically loaded in hardware.
     */
    t_clock dwh_init_clk;

    /* Start of Scope for GP1 */
    {
        mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
        mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP1_REG);
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE);
        mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_ENCRYPT, encrypt_mode);

        /*
         * set SLAVE bit for actual piconet slave, or for both peers
         * during MSS to enable TDD Switch
         */
        if ((context == Master_Slave_Switch) || (role_of_peer == MASTER))
        {
            mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
        }
        else
        {
            mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
        }

        /*
         * read the last clk used in frequency calculation for the 
         * input to data whitening 
         */
        dwh_init_clk = LSLCfreq_Get_Last_Frequency_Clock() >> 1;

        mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_CRC_INIT, crc_hec_init);

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
        /*
         * Always setup the SCO as defined in the device link
         * thereby eliminating the need for separate SCO setup
         */
        {
            t_sco_fifo sco_fifo = p_dev_link->active_sco_fifo;

	    /*
	     * Software always uses hardware SCO_FIFO 0.
	     * Software responsible for multiplexing various logical SCO_FIFOs and related
	     * SCO_CFG, SCO_ROUTE (and any VCI_CLK_SEL and VCI_CLK_SEL_MAP) through
	     * hardware SCO_FIFO 0.
	     */
            mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_SCO_FIFO, 0);
            mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_VCI_CLK_SEL, LC_Get_Vci_Clk_Sel());

            
#if defined(SYS_HAL_VCI_CLK_OVERRIDE_SUPPORTED)
            if(!HWcodec_Is_VCI_CLK_Override_Enabled() || LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(sco_fifo))
#endif
            {
#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
                if (1 == LC_Get_SCO_Repeater_Bit())
                {
                    /* routing via ACL. Turn off transcoding */
                    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_SCO_CFG0, 8 /* transparent */);
                    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_SCO_CFG1, 8 /* transparent */);
                }
                else
#endif
                {
                    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_SCO_CFG0, ( (lslc_acc_shadow_sco_cfg_ >> (sco_fifo<<2)) & 0xF) );
                }
            }
 
            /* VCI_CLK_SEL_MAP is unfortunately in GP2 - and is setup below */
        } 

#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
        {
             mHWreg_Create_Cache_Register(JAL_ESCO_CTRL_REG);
             mHWreg_Assign_Cache_Register(JAL_ESCO_CTRL_REG, 0);

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)         
            mHWreg_Assign_Cache_Field(JAL_ESCO_CTRL_REG, JAL_ESCO_TX_LEN, DL_Get_eSCO_Tx_Length(p_dev_link));
            mHWreg_Assign_Cache_Field(JAL_ESCO_CTRL_REG, JAL_ESCO_RX_LEN, DL_Get_eSCO_Rx_Length(p_dev_link));
            mHWreg_Assign_Cache_Field(JAL_ESCO_CTRL_REG, JAL_ESCO_LT_ADDR, DL_Get_eSCO_LT_Address(p_dev_link));
#endif
        /*
         *  T1 series hardware: set JAL_SCO_ROUTE to support SCO via HCI if required.
         */
        mHWreg_Assign_Cache_Field(JAL_ESCO_CTRL_REG, JAL_SCO_ROUTE, 
                 LSLCacc_Get_SCO_ROUTE(p_dev_link->active_sco_fifo));

        mHWreg_Move_Cache_Register(JAL_ESCO_CTRL_REG, _ctrl_reg_shadow.esco_ctrl);
        }
#endif /*if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) */
#endif /*if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */

        mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, _ctrl_reg_shadow.gp1);
    } /* End of Scope for GP1 */


    /*
     * Now complete the setup of whitening in COM_CTRL_GP2_REG (0x24)
     * Data whitening is bt_clk[6:1] of Tx/Rx slot
     * Both JAL_DWH_INIT and JAL_DWH_INIT2 are required for Return to Piconet
     */
    /* Start of Scope for GP2 */
    {
        mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP2_REG);
        mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP2_REG);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP2_REG, JAL_WHITEN);
#ifdef SYS_DEBUG_INSERT_HEC_ON_ESCO_TX
		if((role_of_peer == MASTER) && (eSCO_Active))
		{
			mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_TEST_HEC, 1);
		}
		else
		{
			mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_TEST_HEC, 0);
		}
#endif
        mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH_INIT, 
            dwh_init_clk);
        mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH2_INIT,
            dwh_init_clk);
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
        mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_VCI_CLK_SEL_MAP, LC_Get_Vci_Clk_Sel_Map());
#endif

        mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP2_REG, _ctrl_reg_shadow.gp2);
    }
    /* End of Scope for GP2 */
}
#endif//BLUETOOTH_MODE_LE_ONLY

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/*****************************************************************************
 * LSLCacc_Sync_Setup_SCO_CFG_Ex
 *
 * Synchronously setup the SCO CFG register (in cache)
 * Extended to support multiple SCO connections (cannot modify pDL)
 *
 * Next slot hardware setup will have these SCO configurations.
 *
 * sco_fifo         Valid range 0..2, otherwise no change
 * sco_cfg          As supported by Tabasco (currently 0..2)
 ****************************************************************************/
void LSLCacc_Sync_Setup_SCO_CFG_Ex(t_sco_fifo sco_fifo, t_sco_cfg sco_cfg)
{
    lslc_acc_shadow_sco_cfg_ &= ~(0xf << (sco_fifo << 2));
    lslc_acc_shadow_sco_cfg_ |= (sco_cfg&0x0F) << (sco_fifo << 2);
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
    lslc_acc_shadow_sco_route_ &= ~(0x1 << sco_fifo );
    lslc_acc_shadow_sco_route_ |= (((sco_cfg & SCO_ROUTE_VIA_ACL) >> 7) << sco_fifo);
#endif
#endif
}

/*****************************************************************************
 * LSLCacc_Get_SCO_CFG
 *
 * Get the SCO CFG register (in cache) for selected sco fifo
 *
 * sco_fifo         Valid range 0..2, otherwise no change
 ****************************************************************************/
#if (PRAGMA_INLINE==1)
#pragma inline(LSLCacc_Get_SCO_CFG)
#endif
__INLINE__ t_sco_cfg LSLCacc_Get_SCO_CFG(t_sco_fifo sco_fifo)
{
    return (lslc_acc_shadow_sco_cfg_ >> (sco_fifo << 2)) & 0x0F;
}

#endif

/*****************************************************************************
 * LSLCacc_Is_SCO_CFG_Via_ACL_Buffer
 *
 * Indicate if the SCO_CFG or SCO_ROUTE(T series HW) is configured to tx/rx 
 * via the ACL buffer. 
 *
 * Input: sco_fifo         Valid range 0..2,
 * Output: 0  SCO via VCI configured
 *         1  SCO via ACL buffer configured
 ****************************************************************************/
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
boolean LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(t_sco_fifo sco_fifo)
{
#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
    return LSLCacc_Get_SCO_ROUTE(sco_fifo);
#else
    return (LSLCacc_Get_SCO_CFG(sco_fifo) == SCO_CFG_VIA_ACL);
#endif
}
#endif

/*****************************************************************************
 * LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode
 *
 * Indicate if the SCO_CFG is related to a sixteen bit or an eight
 * bit input sample. This, in turn, informs the host controller
 * how much data to copy from the hardware ACL buffers.
 * Input: sco_fifo         Valid range 0..2,
 * Output: 0  Not an eight bit input mode.
 *         1  An eight bit input mode.
 ****************************************************************************/
boolean LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(t_sco_fifo sco_fifo)
{
    boolean is_eight_bit = 0;
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    u_int8 sco_cfg = LSLCacc_Get_SCO_CFG(sco_fifo);

    if((sco_cfg < 3) || (sco_cfg == 8) || (sco_cfg == 9) || (sco_cfg == 10)
        || (sco_cfg > 12))
    {
        is_eight_bit = 1;
    }
#endif

    return is_eight_bit;
}


#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
/*****************************************************************************
 * LSLCacc_Sync_Setup_SCO_ROUTE_Ex
 *
 * Synchronously setup the SCO ROUTE  register (in cache)
 *
 * Next slot hardware setup will have this SCO_ROUTE configuration.
 *
 * sco_fifo         Valid range 0..2, otherwise no change
 * sco_route        As supported by Tabasco (ie 0 or 1)
 ****************************************************************************/
void LSLCacc_Sync_Setup_SCO_ROUTE_Ex(t_sco_fifo sco_fifo, boolean sco_route)
{
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    lslc_acc_shadow_sco_route_ &= ~(0x1 << sco_fifo );
    lslc_acc_shadow_sco_route_ |= (sco_route << sco_fifo);
#endif
}
#if 0
/*****************************************************************************
 * LSLCacc_Get_SCO_ROUTE
 *
 * Get the SCO ROUTE register (in cache) for selected sco fifo
 *
 * sco_fifo         Valid range 0..2, otherwise no change
 ****************************************************************************/
__INLINE__ boolean LSLCacc_Get_SCO_ROUTE(t_sco_fifo sco_fifo)
{
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    return (lslc_acc_shadow_sco_route_ >> sco_fifo ) & 0x01;
#endif
}
#endif
#endif


/*****************************************************************************
 * LSLCacc_Prepare_Rx_ID
 *
 * Prepare Receiver Hardware to receive ID packet
 *
 * Parameters:
 * context         InquiryScan | InquiryResponse |
 *                       Page | Page_Scan | SlavePageResponse
 *
 * All registers stored in software shadow registers
 ****************************************************************************/
void LSLCacc_Prepare_Rx_ID(t_state context)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP2_REG);

    /*
     * GP1 Common Control Register:  Set Only Page, Slave and Encrypt Bits
     *                              Clear AM_ADDR only for readability
     */
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_CRC_INIT, 0);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_ENCRYPT, 0);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_AM_ADDR, 0);

    if (context==Page || context==MasterPageResponse 
            || context==Connection_Park_Access_Window_Rx )
    {
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
    }
    else /* context is InquiryScan|InquiryResponse|PageScan|Master_Slave_Switch */
    {
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE);
    }

    /*
     * Store in shadow register to be set when rx enabled
     */
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, _ctrl_reg_shadow.gp1);

    /*
     * GP2 Common Control Register
     */
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP2_REG);
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH_ALL_REG, 0);
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP2_REG, _ctrl_reg_shadow.gp2);

    /*
     * Receive Control Register:  Set RX_MODE only
     */
    {
        t_Receiver_Mode rx_mode;

        mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);
        mHWreg_Load_Cache_Register(JAL_RX_CTRL_REG);

        switch (context)
        {
        case Inquiry_Scan:
        case Page_Scan:
        case InquiryResponse:
            rx_mode = RXm_FULL_WIN;
            break;

        case Master_Slave_Switch:
        case MasterPageResponse:
            rx_mode = RXm_SINGLE_WIN;
            break;

        default:
        /* case Page: */
        /* case Connection_Park_Access_Window_Rx:  */
            rx_mode = RXm_DOUBLE_WIN;
            break;

        }

        mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE, rx_mode);
        mHWreg_Move_Cache_Register(JAL_RX_CTRL_REG, _ctrl_reg_shadow.rx_ctrl);
    }

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    _LSLCacc_Non_EDR_Pkt_Tx_Rx();
#endif
}

/*****************************************************************************
 * LSLCacc_Prepare_Rx_FHS
 *
 * Prepare Receiver Hardware to receive FHS packet
 *
 * Parameters:
 * context         Inquiry | SlavePageResponse
 *
 * Just need to set RX_MODE and the rest is handled in general Tx/Rx function
 ****************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
void LSLCacc_Prepare_Rx_FHS(t_state context, u_int crc_hec_init)
{
    mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_RX_CTRL_REG);

    mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE, RXm_DOUBLE_WIN);
    mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_WIN_EXT, SYSconfig_Get_Win_Ext());

    mHWreg_Move_Cache_Register(JAL_RX_CTRL_REG, _ctrl_reg_shadow.rx_ctrl);

    LSLCacc_Prepare_Tx_Rx_FHS(context, crc_hec_init);

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    _LSLCacc_Non_EDR_Pkt_Tx_Rx();
#endif
}
#endif//BLUETOOTH_MODE_LE_ONLY

#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
/*****************************************************************************
 * LSLCacc_Prepare_Rx_Return_To_Piconet
 *
 * Prepare Receiver Hardware to receive a packet on the Piconet connection
 * when returning from hold mode
 *
 * Just need to set RX_MODE and the rest is handled in general Tx/Rx function
 ****************************************************************************/
void LSLCacc_Prepare_Rx_Return_To_Piconet(const t_devicelink *p_dev_link)
{
    mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);

    mHWreg_Assign_Cache_Register(JAL_RX_CTRL_REG, _ctrl_reg_shadow.rx_ctrl);
    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP1_REG, _ctrl_reg_shadow.gp1);

    mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE, RXm_FULL_WIN);

    /*
     * HW doesn't seem to realign IOR_WIN on non-FHS pkt
     * if the page bit is not set and the pkt is outside the +/-
     * 10 us window -- win-ext does not matter (only for correlation,
     * not for alignment)
     *
     * problem with setting the page bit is that any pkt is
     * treated as an IDpkt
     * 
     * will have huge implications for slave on a connection!!!
     */
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE);

    mHWreg_Move_Cache_Register(JAL_RX_CTRL_REG, _ctrl_reg_shadow.rx_ctrl);
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, _ctrl_reg_shadow.gp1);

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    _LSLCacc_Set_PTT(p_dev_link);
#endif

}

#endif

/*****************************************************************************
 * LSLCacc_Prepare_Rx_Connection
 *
 * Prepare Receiver Hardware to receive a packet on the Piconet connection
 *
 * Just need to set RX_MODE and the rest is handled in general Tx/Rx function
 ****************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
void LSLCacc_Prepare_Rx_Connection(const t_devicelink *p_dev_link)
{
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    t_USLC_Frame_Activity cfa = USLCsched_Get_Current_Frame_Activity();
    t_USLC_Frame_Activity nfa = USLCsched_Get_Next_Frame_Activity();
#endif

    mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);
    mHWreg_Assign_Cache_Register(JAL_RX_CTRL_REG, _ctrl_reg_shadow.rx_ctrl);

    mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE, RXm_SINGLE_WIN);

#if 0 // GF 15 March
 //   mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_WIN_EXT, SYSconfig_Get_Win_Ext());
#else
   mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_WIN_EXT, SYSconfig_Get_Win_Ext());
#endif

    mHWreg_Move_Cache_Register(JAL_RX_CTRL_REG, _ctrl_reg_shadow.rx_ctrl);

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    // Must ensure PTT=0 when transmit/receive HV1/HV2/HV3/DV.
	if (((SCO_MASTER==cfa) || (SCO_SLAVE==nfa)) && is_SCO_Packet(p_dev_link->active_sco_pkt))
	{
        _LSLCacc_Non_EDR_Pkt_Tx_Rx();
    }
	else
	{
        _LSLCacc_Set_PTT(p_dev_link);
    }
#endif
}

/*****************************************************************************
 * LSLCacc_Build_AccessCode
 * Build the Channel/Device/Inquiry Access Code
 *
 * Length 72 bits       Preamble  Sync_Word [Trailer]
 *                      Note LSB of preamble and trailer are leftmost.
 *                      The preamble and trailer are generated by hardware.
 * Preamble[4]:         Fixed zero-one pattern
 *                      if (SyncWord lsb is 0) then 0101 else 1010 endif
 * Syncword[64]:        Master syncword
 * Trailer[4]:          Only if packet header follows
 *                      if (SyncWord msb is 0) then 1010 else 0101 endif
 *
 * Syncword to be stored in common control register shadow.
 *****************************************************************************/
void LSLCacc_Build_AccessCode(const t_syncword *syncword)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    p_reg_shadow->sync_low  = syncword->low;
    p_reg_shadow->sync_high = syncword->high;
}

/*****************************************************************************
 * LSLCacc_Build_PacketHeader
 *
 * Length 54 bits       (AM_ADDR, Type, Flow, ARQN, SEQN, HEC)[18] x 1/3 FEC
 * AM_ADDR[3]:  LC_DeviceLink
 * Type[4]:     Interface: PacketType
 * Flow[1]:     0/1: For each new transmitted packet that contains data
 *              with CRC, invert SEQN.
 * ARQN[1]:     ACK/NAK: Informs Source that the payload data with CRC was
 *              transferred successfully.
 * SEQN[1]:     FLOW/STOP: if receive buffer can/cannot be emptied
 *              (i.e. after checking the transport queue availability).
 * HEC[8]:      Hardware generated.
 *
 * Initialisation of SEQN/ARQN bits must be done in the DeviceLink structure,
 * so that 1st packet transmitted from master and from slave have SEQN=1/ARQN=NAK.
 *
 *****************************************************************************/
void LSLCacc_Build_PacketHeader(const t_devicelink *p_dev_link, t_packet packetType)
{
	extern u_int8 EIR_Packet;
    u_int tx_buf;
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
	mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);

	/*
     * Merge new fields with Shadowed Register for TX_CTRL_REG
     * I need to know the ACTUAL tx_buf bit therefore read register!!!!!
     */
	mHWreg_Load_Cache_Register(JAL_TX_CTRL_REG);

    /*
     * Clear all except tx_buf bit     LC:   SEQN=0, NAK, STOP, AM_ADDR = 0
     */
	/*
	 *
	 * CM: 13 July 2007
	 * Is TX_BUF relevant for the EDR FPGA ?
	 */
    tx_buf = mHWreg_Get_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);
    mHWreg_Clear_Cache_Register(JAL_TX_CTRL_REG);
    if (tx_buf)
    {
        mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);
    }

	if (packetType == EV3)
	{
       mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE, HV3 /* ie 0x7 on BB */);
	}
    else
	{
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE, packetType);
	}
 
	if ( (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) && 
						!is_ACL_CRC_Packet(packetType) )
    {
        mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_ARQN);
    }


    /*
     *  On slave leave at NAK until explicitly set
     *  Not necessary but a sanity check in case of spurious packets
     */
    if (DL_Get_Role_Peer(p_dev_link) == SLAVE)   /* Local device is master */
    {
        if ( DL_Get_Tx_LT_Previous_Arqn(p_dev_link) )
        {
            mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_ARQN);
        }
    }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
     if (is_eSCO_LT(p_dev_link->tx_lt_index))
     {  
        /*
         * FLOW must always be set to go on eSCO logical transport.
         */
        mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_FLOW);
     }
     else
#endif
    /*
     * FLOW = GO (1) if local queue is not full or
     *  if the local queue was full and has now got space
     */
	 if (EIR_Packet !=1)
	 {
		 if ( DL_Is_Local_Buffer_Full(p_dev_link) 
#if (LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED==1)
			 || (LSLCacc_Is_Deferred_Extraction_Pending_())
#endif
			 )
		 {
			 if (BTq_Is_Queue_Not_Full(L2CAP_IN_Q, DL_Get_Device_Index(p_dev_link)))
			 {
				 mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_FLOW);
				 DL_Set_Local_Rxbuf_Full((t_devicelink *)p_dev_link, FALSE);
			 }
		 }
		 else                                       /* GO 1 */
		 {
			 mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_FLOW);
		 }
	 }
    /*
     * SEQN:  if (Device Link Tx_Seqn = 1) Then Set to 1 Else Leave as set above (0) 
     */
    if (DL_Get_Tx_LT_Seqn(p_dev_link) )
    {
        mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_SEQN);
    }

    /*
     * Any header packet must have transmit mode normal
     */
    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE, TXm_NORMAL);

    mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    /*
     * AM_ADDR    (setup in JAL_COM_CTRL_GP1_REG by LSLCacc_Set_Am_Addr())
     *
     * May only set AM_ADDR if (packetType!=FHSpkt) ||
     *                    DL_Get_Ctrl_State(p_dev_link) == CONNECTED_TO_MASTER)
     * 2nd predicate is to support Master/Slave Switch FHS
     */
    LSLCacc_Set_Am_Addr( DL_Get_LT_Address(p_dev_link) );

    LSLCstat_Inc_Tx_Packet_Cntr(packetType); /* Note: IDs not counted here */

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	{
        mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
        mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);

		mHWreg_Clear_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ACL);
        mHWreg_Clear_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO);

        if (is_EDR_ACL_packet(packetType))
        {
             mHWreg_Assert_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ACL);
        }
	    else if (is_EDR_ESCO_packet(packetType))
	    {
            mHWreg_Assert_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO);
        }
        mHWreg_Move_Cache_Register(JAL_EDR_CTRL_REG, p_reg_shadow->edr_ctrl);
    }
#endif 
}

/*****************************************************************************
 * void _LSLCacc_Prepare_Tx_Complete_If_Same_Device
 *
 * It is only when a packet has been completely received that the CRC check
 * can be checked.  This means that the prepared next outgoing
 * ARQN/FLOW may have to be over-ridden based on the latest received packet.
 *
 * In addition, this function will prevent an L2CAP message from being
 * transmitted if the DL_Get_Peer_L2CAP_Flow == STOP to ensure abrupt
 * stopping of data when the previous payload header indicated it.
 * Required during start/stop of encryption.
 *****************************************************************************/
void _LSLCacc_Prepare_Tx_Complete_If_Same_Device(t_devicelink *p_dev_link)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    t_packet tx_packet_type = DL_Get_Tx_Packet_Type(p_dev_link);
    /*
     * For SLAVE, (TX) AM_ADDR is in COM_CTRL_REG
     */

    mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);

    /*
     * If I'm a slave I should always set-up the latest ARQN since
     * I've just received the packet.
     * If I'm a master I should only set-up if the device I'm sending
     * to is the same as the device I've received from.
     */
    if ( mHWreg_Get_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE) ||
        ( !mHWreg_Get_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_SLAVE) &&
         ( mHWreg_Get_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_AM_ADDR)
           == DL_Get_Tx_LT_Am_Addr(p_dev_link) ) ) )
    {
         /*
          * Merge latest ARQN fields with Shadowed Register for TX_CTRL_REG
          */
         if ( DL_Get_Tx_LT_Previous_Arqn(p_dev_link) )
         {
             mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_ARQN);
         }
         else
         {
             mHWreg_Clear_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_ARQN);
         }

         /*
          * Set Flow to STOP (=0), only if ACL FLOW == STOP,
          */
         if ( DL_Get_Local_Rxbuf_Full(p_dev_link) )
         {
             mHWreg_Clear_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_FLOW);
         }

         /*
          * L2CAP Flow Requirement:
          * Encryption start/stop during connection requires abrupt stop
          * of data.  Hence if L2CAP_Flow == STOP, as result of
          * previously received packet then replace ACL packet with NULL
          */

         if (   DL_Get_Local_Tx_L2CAP_Flow(p_dev_link)==STOP &&
                mLSLCacc_Is_ACL_Packet(tx_packet_type) &&
                mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_L_CH)!=LMP_msg &&
                mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_LEN)!=0 )
         {
                 mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE, NULLpkt);
                 /*
                  * Ensure that we cancel the Ack Pending
                  * ACKs in headers may be persistent (rf ARQ Scheme Part B)
                  * Since we have an explicit request from previous packet,
                  * we know we have no outstanding NAK, therefore it is
                  * safe to request a new descriptor later
                  *
                  * Bug 1244, this is required
                  * If an L2CAP packet is being overruled the pending
                  * ACK must be cancelled, since we are now sending a
                  * default packet instead and the peer device can
                  * persist the ACK bit which without the code below
                  * would cause a false ACK to the unsent L2CAP packet
                  */
                  DL_Set_Tx_LT_Tx_Ack_Pending(p_dev_link, FALSE);
         }

         mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
    }
}

/************************************************************************
 * LSLCacc_Build_FHS_Payload
 * Must update the clock each time FHS packet is sent
 * Returns updated clock into the FHS structure (Design decision)
 * In 0.9, actual FHS is 144 bits, but structure is 160bits, no issues!
 *
 * LSLCacc_Build_FHS_Payload_Ex
 * Extended function to explicitly take a native_clk value
 * Used for FHS in Master Slave Switch
 ************************************************************************/
void LSLCacc_Build_FHS_Payload(t_devicelink *p_dev_link)
{
    t_clock  native_clk = HW_get_native_clk();

    LSLCacc_Build_FHS_Payload_Ex(p_dev_link, native_clk + 4);
}

void LSLCacc_Build_FHS_Payload_Ex(t_devicelink *p_dev_link, t_clock native_clk)
{
    t_length pkt_contents_length = FHS_PACKET_LENGTH;
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;


    /*
     * FHS carries native clock at instance when being carried.
     * No need to take care of clock wrap since FHS structure carries
     * only 26 bits.  Last clock sent stored in Local Device FHS
     */
    FHS_Set_CLK(LC_Get_Device_FHS_Ref(), native_clk);
    hw_memcpy((void *)(HW_get_tx_acl_buf_addr()), LC_Get_Device_FHS_Ref(),
                   pkt_contents_length);

    /*
     * Update Length field and Toggle Buffer field in cached TX control register
     */
    {
    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_LEN, pkt_contents_length);
    mHWreg_Toggle_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);

    mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
    }
}
#endif//BLUETOOTH_MODE_LE_ONLY
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
/************************************************************************
 * LSLCacc_Build_EIR_Payload
 ************************************************************************/
void LSLCacc_Build_EIR_Payload(void)
{
    t_length pkt_contents_length = LC_Get_Local_EIR_Length();
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    hw_memcpy((void *)(HW_get_tx_acl_buf_addr()), LC_Get_Local_EIR_Payload(),
                   pkt_contents_length);

    /*
     * Update Length field and Toggle Buffer field in cached TX control register
     */
    {
    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_L_CH, LCH_start);
    // BQB :- In EIR packets the Try P-Flow should be set to 0 
    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_P_FLOW, 0);

    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_LEN, pkt_contents_length);
    mHWreg_Toggle_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);

    mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
    }
}
#endif
#if     0
/************************************************************************
 * LSLCacc_Build_ACL_Payload
 * Generate Payload Header: Single/Multi Slot [1/2bytes]
 *
 * L_CH[2] b1b0:  00 N/A, 01 L2CAP cont, 10 L2CAP start, 11 LMP
 * FLOW[1] b2:    Controls flow at the L2CAP level.
 * LENGTH         Single[5] b7:b3,      Multi[9]
 *                Length *excludes* the Payload Header and CRC
 * PADDING        Single[0]             Multi[4]
 *
 * Length of packet is the length of payload excluding header
 * Precondition  Packets must not exceed packet length (no local check)
 ************************************************************************/
void LSLCacc_Build_ACL_Payload(t_devicelink *p_dev_link)
{
    t_length pkt_contents_length = DL_Get_Tx_Length(p_dev_link);
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
    t_packet packet_type;

    /*
     * Copy the data to the ACL registers
     */
	_LSLCacc_memcpy_ACL((void*) HW_get_tx_acl_buf_addr(), 
                     (void*)DL_Get_Tx_Qpdu(p_dev_link), pkt_contents_length);
    /*
     * Now setup the tx control register
     */ 
    {
    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);

	mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
    /*
     * Determine the packet type
     */
    packet_type = DL_Get_Tx_Packet_Type(p_dev_link);

    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_L_CH,
               DL_Get_Tx_Message_Type(p_dev_link));

    /*
     * PAYLOAD FLOW bit is always GO (1) for LMP
     */

    if (DL_Get_Tx_Message_Type(p_dev_link) == LMP_msg )
    {
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_P_FLOW, 1);
    }
    else
    {
         mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_P_FLOW,
             DL_Get_Local_Rx_L2CAP_Flow(p_dev_link) );
    }
    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_LEN, pkt_contents_length);

	if(packet_type!=DV)
    {
        mHWreg_Toggle_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);
    }

    mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    /*
     * Count number of Tx Data (ACL) Packets
     */
    DL_Inc_Tx_ACL_Data_Packets(p_dev_link);

    /*
     * If Pump Verification Enabled Then Verify the Tx Data Register
     */
    BTpump_Verify_Data_Pump_Before_Tx(DL_Get_Device_Index(p_dev_link),
           (u_int8 *) HW_get_tx_acl_buf_addr(), 
           pkt_contents_length, 
           (t_LCHmessage) mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_L_CH) );

    /*
     * If Test Mode Tester Enabled Then Verify the Rx Data Is Ok
     *
     * Callback to function to perform payload verification
     */
    BTtmt_Verify_Tester_Before_Tx(DL_Get_Device_Index(p_dev_link),
           (u_int8 *) HW_get_tx_acl_buf_addr(), 
           pkt_contents_length, 
           (t_LCHmessage) mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_L_CH));

    }
}
#endif
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/************************************************************************
 * LSLCacc_Build_SCO_Payload
 *
 * if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
 * {
 *    If SCO through ACL Memory Buffer (SCO_CFG==8) Then
 *       HV1 is in ACL bytes Voice 10  [00,01,02,03,04,05,06,07,08,09]
 *       HV2 is in ACL bytes Voice 20  [00,01,02,03,04,05,06,07,08,09,
 *                                     0A,0B,0C,0D,0E,0F,10,11,12,13]
 *       HV3 is in ACL bytes Voice 30  [00,01,02,03,04,05,06,07,08,09,
 *                                     0A,0B,0C,0D,0E,0F,10,11,12,13]
 *                                     14,15,16,17,18,19,1A,1B,1C,1D]
 *       DV is in ACL bytes  Voice 10  [00,01,04,05,08,09,0C,0D,10,11]
 *                           Data   9   14,15,16,17,18,19,1A,1B,1C]
 *     Else If SCO through Code (SCO_CFG != 8)Then
 *       HV1/2/3    Voice Bytes are to/from Codec without processor intervention
 *       DV         Voice Bytes are to/from Codec without processor intervention
 *                  Data  is as normal ACL [00,01,02,03,04,05,06,07,08,09]
 * }
 * else 
 * {
 * If SCO through ACL Memory Buffer (SCO_CFG==9) Then
 *    HV1 is in ACL bytes Voice 10  [00,01,04,05,08,09,0C,0D,10,11]
 *    HV2 is in ACL bytes Voice 20  [00,01,04,05,08,09,0C,0D,10,11,
 *                                   14,15,18,19,1C,1D,20,21,24,25]
 *    HV3 is in ACL bytes Voice 30  [00,01,04,05,08,09, C, D,10,11,
 *                                   14,15,18,19,1C,1D,20,21,24,25
 *                                   28,29,2C,2D,30,31,34,35,38,39]
 *    DV is in ACL bytes  Voice 10  [00,01,04,05,08,09,0C,0D,10,11]
 *                        Data   9   14,15,16,17,18,19,1A,1B,1C]
 * Else If SCO through Code (SCO_CFG < 9)Then
 *    HV1/2/3    Voice Bytes are to/from Codec without processor intervention
 *    DV         Voice Bytes are to/from Codec without processor intervention
 *               Data  is as normal ACL [00,01,02,03,04,05,06,07,08,09]
 * Endif
 *
 * Notes:
 *  The TX_LEN is only appropriate for DV packets.
 *  For SCO via HCI:
 *      1.  The ACL buffer must be toggled if HV1/2/3 
 *      2.  The ACL data of DVs will be setup first rf LSLCpkt_Generate_DV().
 *          and will have to be moved 20 bytes to allow for SCO data
 ************************************************************************/
void LSLCacc_Build_SCO_Payload(const t_devicelink *p_dev_link)
{
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    t_packet packet_type;
    t_length pkt_contents_length;
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
    boolean packet_payload_available = FALSE;

    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    
    mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    packet_type = DL_Get_Tx_SCO_Packet_Type(p_dev_link);

    if (LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(p_dev_link->active_sco_fifo))
    {
        /*
         * Move the payload since the configuration is a FIFO based one
         */       
        if (packet_type==DV)
        {
            pkt_contents_length = MAX_DVSCO_USER_PDU;

            /*
             * If Hardware is older than T-series hardware:
             * 
             * If SCO through ACL Memory Buffer (SCO_CFG==9) Then
             *    DV is in ACL bytes  Voice 10  [00,01,04,05,08,09,0C,0D,10,11]
             *                        Data   9   14,15,16,17,18,19,1A,1B,1C]
             * Endif
             * Hence stored ACL Data (max 9) must be moved up by 20 bytes
             * to allow for SCO data.  
             * The ACL data is re-read from the queue as a move operation
             * was unstable on a development environment (B1852).
             *
             * Else (if hardware is T-series or newer:
             *
             * If SCO through ACL Memory Buffer (sco route 1)
             * and conversion from 8-bit baseband interface.
             *    DV is in ACL bytes Voice 10  [00,01,02,03,04,05,06,07,08,09]
             *                       Data   9  [0c,0d,0e,0f,10,11,12,13,14]
             * else if conversion from >8-bit baseband interface.
             *    DV is in ACL bytes Voice 20  [00,01...13]
             *                       Data   9  [14,15...1C]
             * Endif
             * Hence stored ACL Data (max 9) must be moved up by 12 bytes
             * to allow for SCO data.
             *
             * Endif T-Series Hardware.
             * 
             */
#if(PRH_BS_CFG_TABASCO_VERSION < PRH_BS_HW_TABASCO_VERSION_T1_0_0)
            _LSLCacc_memcpy_ACL((void*) (HW_get_tx_acl_buf_addr() + 0x14), 
                (void*)DL_Get_Tx_Qpdu(p_dev_link), 12);
#else
            if(LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(p_dev_link->active_sco_fifo))
            {
                _LSLCacc_memcpy_ACL((void*) (HW_get_tx_acl_buf_addr() + 0x0c), 
                    (void*)DL_Get_Tx_Qpdu(p_dev_link), 12);
            }
            else
            {
                _LSLCacc_memcpy_ACL((void*) (HW_get_tx_acl_buf_addr() + 0x14),
                    (void*)DL_Get_Tx_Qpdu(p_dev_link), 12);
            }
#endif
        }
        else
        {
#if(PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
            if (is_eSCO_LT(p_dev_link->tx_lt_index))
            {
                pkt_contents_length = DL_Get_eSCO_Tx_Length(p_dev_link);
            }
            else
#endif
            {
                pkt_contents_length = SYSconst_Get_Packet_Length(packet_type);
            }

            /*
             * If sample size is greater than 8 bit, then
             * use 16 bits for each sample - hence copy
             * twice as much data from the queue to hardware
             * - hardware converts to 8 bit samples and
             * sends the correct amount of data to the peer
             */
            if (!LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(p_dev_link->active_sco_fifo) )
            {
                pkt_contents_length <<= 1;
            }
        } /* (packet_type==DV) */

#if(PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        if (is_ACL_LT(p_dev_link->tx_lt_index))
#endif
        {
            /*
             * The SCO data is currently just top of SCO queue.
             */
            u_int8 active_sco_fifo;
#if(0==PRH_BS_DBG_SOURCE_SINK_SYN_TEST_DATA)
            t_q_descr *p_qD;
#endif
               
            active_sco_fifo = p_dev_link->active_sco_fifo;

#if(0==PRH_BS_DBG_SOURCE_SINK_SYN_TEST_DATA)
            p_qD = BTq_BB_Dequeue_Next(SCO_OUT_Q, active_sco_fifo, 
                    (t_packetTypes)(1<< DL_Get_Tx_SCO_Packet_Type(p_dev_link)));

            if(p_qD && p_qD->data)
            {
                _LSLCacc_memcpy_SCO_to_ACL((void *)HW_get_tx_sco_buf_addr(),
                (void *)p_qD->data, pkt_contents_length);
                BTq_Ack_Last_Dequeued(
                SCO_OUT_Q, active_sco_fifo, pkt_contents_length);

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
                if(HCfc_Read_SYN_Flow_Control_Enable())
                {
                    BTms_OS_Post(BTMS_OS_EVENT_HOST_TO_HC_FLOW_CONTROL);
                }
#endif
                packet_payload_available = TRUE;
            } 
#else
            {
            u_int8 temp_buffer[540] = { 0xAA };
            _LSLCacc_memcpy_SCO_to_ACL((void *)HW_get_tx_sco_buf_addr(), temp_buffer, pkt_contents_length);
            packet_payload_available = TRUE;
            }
#endif
        } /* SCO packet ? */

#if(PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        else /* Setup eSCO payload*/
        {
            /*
             * #2233
             */ 
            if(DL_Get_eSCO_Tx_Data(p_dev_link))
            {
                /*
                 * Copy the eSCO data to the ACL registers
                 */
                _LSLCacc_memcpy_SCO_to_ACL((void*) HW_get_tx_sco_buf_addr(), 
                    (void*)DL_Get_eSCO_Tx_Data(p_dev_link), pkt_contents_length);
                packet_payload_available = TRUE;
            }
        }
#endif

        if (!packet_payload_available)
        {
            if (BTtst_Get_DUT_Mode() >= DUT_ACTIVE_TXTEST)
            {
                /*
                 * Tx a SCO in testmode must use pre-setup packet payload
                 * The payload (normally PRBS) is defined by DL_Set_Tx_Qpdu()
                 */
                _LSLCacc_memcpy_SCO_to_ACL((void *)HW_get_tx_sco_buf_addr(),
                    (void *)DL_Get_Tx_Qpdu(p_dev_link), pkt_contents_length);
            }
            else
            {
                /*
                 * No SCO packet on queue to send
                 * Alternatives:
                 * 0. Resend same SCO data as last sent   
                 * 1. Preset SCO data to fixed pattern 0
                 * 2. Preset SCO data to fixed pattern 0101
                 * 3. Substitute NULL if HV1-3  [Current default]
                 *    Substitute DM1 if DV
                 *
                 * Could check extension bits on Voice Settings
                 * switch ((LC_Get_Voice_Setting()&0x3000)>>12)
                 * {
                 *    case 0:
                 *       break;
                 *    case 1:
                 *        break;     
                 *    case 2:
                 *        break;     
                 *    case 3:   Substitute NULL
                 *        break;     
                 * }
                 *
                 */
                 
                /*
                 * Substitute a DM1 if DV, otherwise a NULLpkt
                 */
                t_packet new_packet_type = 
                    (t_packet) ((packet_type==DV) ? DM1 : NULLpkt);
                mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE, new_packet_type);
//                mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
                LSLCstat_Dec_Tx_Packet_Cntr(packet_type);
                LSLCstat_Inc_Tx_Packet_Cntr(new_packet_type);
            } 
        } /* packet_payload_available */
    } /* LSLCacc_Is_SCO_CFG_Via_ACL_Buffer*/

    /*
     * Ensure that buffer is toggled when enabling transmitter
     */
    mHWreg_Toggle_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);
    mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    /*
     * Note do not set length of SCO packets in JAL_TX_LEN register.
     * Packet Type set in Build_Packet_Header()
     */
#endif
}
#endif

/************************************************************************
 * _LSLCacc_Move_CC_Shadow_to_Jalapeno
 *
 *      Moves the locally cached Common Control Registers to Hardware
 *      Only over-ride values here e.g. WHITEN Off
 ************************************************************************/
static void _LSLCacc_Move_CC_Shadow_to_Hardware(void)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP2_REG);
    mHWreg_Create_Cache_Register(JAL_ESCO_CTRL_REG);
    /*
     * Move shadow contents to local caches and adjust if necessary
     * by merging adjusting fields before writing
     */
    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);
    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP2_REG, p_reg_shadow->gp2);
    mHWreg_Assign_Cache_Register(JAL_ESCO_CTRL_REG, p_reg_shadow->esco_ctrl);

    /*
     * The syncword is in both the Jalapeno LC and also in the
     * Habanero PHY
     *
     * The PHY uses it to correlate on incoming packets,
     * the LC uses it to put the access code in packet for transmission
     */
    {
        t_syncword syncword;
        syncword.low  = p_reg_shadow->sync_low;
        syncword.high = p_reg_shadow->sync_high;
        HWradio_Set_Syncword(syncword);
    }

    /* also do it in Jalapeno for transmission */
    mHWreg_Assign_Register(JAL_COM_CTRL_SYNC_LO_REG, p_reg_shadow->sync_low);
    mHWreg_Assign_Register(JAL_COM_CTRL_SYNC_HI_REG, p_reg_shadow->sync_high);

	*((volatile unsigned *) (0x4002c24c)) = p_reg_shadow->sync_low;
	*((volatile unsigned *) (0x4002c250)) = p_reg_shadow->sync_high;

    /*
     * Note encryption can be uni-directional from Master (rf LMP)
     * The actual activation of Kc' generation from Kc is on writing
     * the ENC_KEY_LEN.  Hence, the ordering of Key, then Key Length must
     * be maintained.
     */
    if (mHWreg_Get_Cache_Field(JAL_COM_CTRL_GP1_REG, JAL_ENCRYPT) != 0)
    {
        /*
         * Optimised for slower boards, move 16 bytes at a time
         * Note loop overhead has been same as move.
         */
        hw_memcpy128((void*) JAL_ENC_KEY_ADDR, 
            (void*) p_reg_shadow->p_encryption_key, 16);
        mHWreg_Assign_Register(JAL_ENC_KEY_LENGTH_REG,
                 p_reg_shadow->encryption_key_length );
    }

    /*
     *  Allow Override any common control register settings here!
     */
    {
        /*
         *  DWH can be over-riden via system configuration.
         */
        if (SYSconfig_Get_Data_Whitening_Enable()==0)
        {
            mHWreg_Assign_Cache_Field(JAL_COM_CTRL_GP2_REG, JAL_DWH_ALL_REG, 0);
        }
    }

    /*
     * Now store the General Purpose cache registers in hardware
     */
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_GP2_REG);
    mHWreg_Store_Cache_Register(JAL_ESCO_CTRL_REG);
}

/************************************************************************
 * _is_Priority_ACL
 *      identifies if next ACL access is a high priority access
 ************************************************************************/
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
boolean _is_Priority_ACL(t_devicelink *p_dev_link)
{
    t_lmp_link *p_link = LMaclctr_Find_Device_Index(p_dev_link->device_index);
    if (p_link)
    {
        return (!BTq_Is_Queue_Empty(LMP_OUT_Q, p_dev_link->device_index) || ((p_link->flush_timeout != 0)
            && !BTq_Is_Queue_Empty(L2CAP_OUT_Q, p_dev_link->device_index))
            || (p_link->default_pkt_type == POLLpkt) || (LMpol_Get_Interval_To_Next_Wakeup()==0));
    }
    return FALSE; // no associated p_link - e.g. inquiry procedure
}
#endif

/************************************************************************
 * _is_Priority_CHAC
 *      identifies if next access is a high priority CHAC access
 ************************************************************************/
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
boolean _is_Priority_CHAC(void)
{
    t_ulc_procedure active_procedure = USLCchac_Get_Active_Procedure();

    return (0 != ((1<<active_procedure) & (
    		(1<<R2P_PROCEDURE) |
    		(1<<MSS_PROCEDURE) |
    		(1<<MSS_PROCEDURE_SLAVE_CONTEXT) |
    		(1<<MSS_PROCEDURE_MASTER_CONTEXT) |
    		(1<<PAGE_PROCEDURE) ) ));
}
#endif
#if     0
/************************************************************************
 * LSLCacc_Enable_Transmitter
 *      Enables transmit of generated packet
 *      (later may need to check if all ok)
 *      Moves the locally cached Common Control Registers to Hardware
 *
 * Transmitter is automatically disable (TX_MODE -> 0) after Transmission
 ************************************************************************/
void LSLCacc_Enable_Transmitter(void)
{
#if ( (PRH_BS_DEV_TOGGLE_TX_SEQN_ON_TX_CRC_PACKET ==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1))
   t_devicelink *p_dev_link =
               LSLCpkt_Get_Active_Tx_Device_Link_Ref();
   t_packet packetType;
#endif
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Assign_Cache_Register(JAL_EDR_CTRL_REG, p_reg_shadow->edr_ctrl);
#endif

    mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);

    /*
     * If something to transmit  TX_MODE != 0
     */
    if ( mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE) != 0 )
    {
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
        t_USLC_Frame_Activity cfa = USLCsched_Get_Current_Frame_Activity();
		t_USLC_Frame_Activity nfa = USLCsched_Get_Next_Frame_Activity();
		
        HWpta_Tx_Access_Request(	(boolean)((0 != (nfa & (SCO_MASTER))) || (0 != (cfa & (SCO_SLAVE)))) /*sco priority*/
			    || (((0 != (nfa & ACL_MASTER)) || (0 != (cfa & ACL_SLAVE))) && _is_Priority_ACL(p_dev_link)) /* acl priority */
					|| _is_Priority_CHAC() /* chac priority */,
		     						(boolean)(0 != (cfa & (CHAN_MASTER|CHAN_SLAVE))) /*freq_overlap*/,
			    					(DL_Get_Role_Local(p_dev_link)) /*role */);
#endif

#if (PRH_BS_DEV_TOGGLE_TX_SEQN_ON_TX_CRC_PACKET ==1)
    /*
     * Requirement (B2131):
     * During transmission of packets without a CRC the SEQN bit shall
     * remain the same as it was in the previous packet.[BT1.2, section 7.6.2.5]
     *
     * The tx_seqn stored in the "device_link" represents the tx_seqn to use
     * in the transmission of the next "CRC packet". This tx_seqn should only
     * be used for "non-CRC" packets when the the tx_seqn has actually been used
     * in the transmission of a "CRC" packet. A design decision has been taken to 
     * only toggle the tx_seqn  on the reception of an ACK of the last 
     * transmitted "CRC" packet. Therefore, a mechanism is required to ensure correct
     * use of the tx_seqn for transmission of "non-CRC" packets between the transmission
     * of "CRC" packets. "tx_seqn_active" will be used to determine if the tx_seqn
     * stored in the device_link has been "actually" used for the transimission of a 
     * "CRC" packet. Only when a "CRC" packet has been transmitted will the "tx_seqn_active"
     * be set to TRUE.
     *
     * Take for example the following scenario:
     * Tx: DM(tx_seqn=1) device_link.tx_seqn=1
     * Rx: NULL(ACK) device_link.tx_seqn=0 (ie 1-->0)
     * Tx: DM(tx_seqn=0) device_link.tx_seqn=0
     * Rx: NULL(ACK) device_link.tx_seqn=1 (ie 0-->1)
     * Tx: NULL(tx_seqn=0) device_link.tx_seqn=1 [NOTE 1]
     * Rx: NULL
     * Tx: DM(tx_seqn=1) device_link.tx_seqn=1
     * Rx: NULL(ACK) device_link.tx_seqn=0 (ie 1-->0)
     * Tx: NULL(tx_seqn=1) device_link.tx_seqn=0
     * Rx: NULL
     *
     * NOTE1:
     * Even though device_link.tx_seqn=1, the NULL must be transmitted with
     * tx_seqn=0 as the last "CRC" packet transmitted by the device used a 
     * tx_seqn=1. Therefore, a the tx_seqn currently stored in the device_link
     * has not been used for a "CRC" packet (ie is inactive), the tx_seqn used
     * for the transmission NULL packet must be the inverse of that stored in the
     * device_link.
     * 
     */
      packetType = (t_packet) mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG,    JAL_TX_TYPE);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
        if (packetType > DM1)
        {
            /*
             * Potentially an EDR packet.  Need to pre-pend the PTT_ACL | PTT_ESCO bit
             */
            packetType = ( ( mHWreg_Get_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ACL) |
		                     mHWreg_Get_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO)) << 4) |
                             packetType;
         }
#endif 

    if ( !is_ACL_CRC_Packet(packetType)
         && (!DL_Is_Tx_Seqn_Active(p_dev_link))
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
        && is_ACL_LT(p_dev_link->tx_lt_index)
#endif
    )
    {
        /*
         * tx seqn is not active, therefore must use a "toggled"
         * tx_seqn for these non-CRC packets.
         */
        if (DL_Get_Tx_LT_Seqn(p_dev_link) )
        {
            mHWreg_Clear_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_SEQN);
        }
        else
        {
            mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_SEQN);
        }

    }
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
    if (is_ACL_LT(DL_Get_Active_Tx_LT(p_dev_link)) 
        /* And not in a Testmode Active state*/
        && ( (BTtst_Get_DUT_Mode() == DUT_DISABLED) ||
             (BTtst_Get_DUT_Mode() == DUT_ENABLED) ))
    {
        /*
         * In LSLCacc_Prepare_Tx_Rx_Connection() the JAL_ESCO_CTRL_REG registers
         * are set by default (i.e. even when the next slot is not an eSCO slot).
         * As the function LSLCacc_Prepare_Tx_Rx_Connection() is common to both
         * Tx and Rx, it is not possible to determine within the function if 
         * the Tabasco register preparation is for a Tx or Rx slot.
         * However, it is important that the eSCO_LT_ADDR is always set to "0"
         * for a "non-eSCO" slot.
         * Therefore, on enabling the transmitter override the eSCO_AM_ADDR component
         * of JAL_ESCO_CTRL_REG (that was just previously setup in 
         * LSLCacc_Prepare_Tx_Rx_Connection()) in the case when the next slot is 
         * is "non-eSCO" slot.
         *
         * EXCEPTION:
         * If the device is in Testmode, then always set the eSCO_LT_ADDR. Ie 
         * do not modify the eSCO_LT_ADDR previously set in 
         * LSLCacc_Prepare_Tx_Rx_Connection().
         */
         mHWreg_Create_Cache_Register(JAL_ESCO_CTRL_REG);
          mHWreg_Assign_Cache_Register(JAL_ESCO_CTRL_REG, _ctrl_reg_shadow.esco_ctrl);

         mHWreg_Assign_Cache_Field(JAL_ESCO_CTRL_REG, JAL_ESCO_LT_ADDR, 0);

         mHWreg_Move_Cache_Register(JAL_ESCO_CTRL_REG, _ctrl_reg_shadow.esco_ctrl);
    }
#endif

        _LSLCacc_Move_CC_Shadow_to_Hardware();
        mHWreg_Store_Cache_Register(JAL_TX_CTRL_REG);

        if (mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_ARQN))
        {
            LSLCstat_Inc_Tx_Ack_Sent_Cntr();
        }
        
        /*
         * Clear TX_MODE in cache to prevent 2nd sending
         */
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE, TXm_TRANSMITTER_DISABLED);

        mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

#if (PRH_BS_DEV_TOGGLE_TX_SEQN_ON_TX_CRC_PACKET ==1)
    if (is_ACL_CRC_Packet(packetType))
    {
        /*
         * The tx_seqn is considered "active" once it has been used
         * in the transmission of a CRC packet.
         * Rules for tx_seqn used for non-RC packets is as follows:
         * if (tx_seqn_active)
         *   then use tx_seqn in packet transmission.
         * else 
         *     then use inverted tx_seqn (inverted as stored in the device_link)
         */
        DL_Set_Tx_Seqn_Active(p_dev_link);
    }
#endif
    }
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    {
        mHWreg_Assert_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_ENABLE);
    	mHWreg_Assign_Cache_Field(JAL_EDR_CTRL_REG, JAL_EDR_TX_EDR_DELAY, edr_tx_edr_delay);
        mHWreg_Assign_Cache_Field(JAL_EDR_CTRL_REG, JAL_EDR_RX_EDR_DELAY, edr_rx_edr_delay);
	//	mHWreg_Assign_Cache_Field(JAL_EDR_CTRL_REG,JAL_EDR_SYNC_ERROR, edr_sync_error);
        mHWreg_Store_Cache_Register(JAL_EDR_CTRL_REG);
    }
#endif

    /*
     *  Override any transmit control register settings here!
     */


}
#endif
/************************************************************************
 * LSLCacc_Disable_Transmitter
 *      Aborts pending transmission if any by clearing TX_MODE
 *      Used to override lookahead packet if not needed.
  ************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
void LSLCacc_Disable_Transmitter(void)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
    mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE, TXm_TRANSMITTER_DISABLED);
    mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    HWpta_Tx_Access_Complete();
#endif

    /* put device back into standby */

    if ((Standby == USLCchac_get_device_state()) && !HW_get_tx_mode())
    {
        /* opportunity to put radio in standby */
    }
    else
    {
        /*
         * jn Target for removal.
         */
        if(!HW_get_page())
        {

            extern t_freq _LSLCfreq_channel[4];
            _LSLCfreq_channel[TX_START] = _LSLCfreq_channel[TX_MID] 
                = HW_RADIO_SETTING_INVALID;
        }
    }
}
#endif//BLUETOOTH_MODE_LE_ONLY

/************************************************************************
 * LSLCacc_Enable_Receiver
 *      Enables receiver for generated packet
 *      (later may need to check if all ok)
 *      Moves the locally cached Common Control Registers to Hardware
 * Restrictions
 *      Must be called only once per required enable.
 ************************************************************************/
void LSLCacc_Enable_Receiver(void)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
    t_devicelink *p_dev_link = LSLCpkt_Get_Active_Rx_Device_Link_Ref();

    mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);

    mHWreg_Assign_Cache_Register(JAL_RX_CTRL_REG, p_reg_shadow->rx_ctrl);
    
    /*
     * If something to receive  RX_MODE != 0
     */
    if ( mHWreg_Get_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE)
               != RXm_RECEIVER_DISABLED )
    {
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
		t_USLC_Frame_Activity cfa = USLCsched_Get_Current_Frame_Activity();
		t_USLC_Frame_Activity nfa = USLCsched_Get_Next_Frame_Activity();

        HWpta_Rx_Access_Request((boolean)((0 != (nfa & (SCO_SLAVE))) || (0 != (cfa & (SCO_MASTER)))) /*priority*/
			    || (((0 != (nfa & ACL_SLAVE)) || (0 != (cfa & ACL_MASTER))) && _is_Priority_ACL(p_dev_link)) /* acl priority */
					|| _is_Priority_CHAC() /* chac priority */,
		     						(boolean)(0 != (cfa & (CHAN_MASTER|CHAN_SLAVE))) /*freq_overlap*/,
			    					(DL_Get_Role_Local(p_dev_link)) /*role */);
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
        if (is_ACL_LT(DL_Get_Active_Rx_LT(p_dev_link))
            /* And not in a Testmode Active state*/
            && ( (BTtst_Get_DUT_Mode() == DUT_DISABLED) ||
                 (BTtst_Get_DUT_Mode() == DUT_ENABLED)) )
        {
            /*
             * In LSLCacc_Prepare_Tx_Rx_Connection() the JAL_ESCO_CTRL_REG registers
             * are set by default (i.e. even when the next slot is not an eSCO slot).
             * As the function LSLCacc_Prepare_Tx_Rx_Connection() is common to both
             * Tx and Rx, it is not possible to determine within the function if 
             * the Tabasco register preparation is for a Tx or Rx slot.
             * However, it is important that the eSCO_LT_ADDR is always set to "0"
             * for a "non-eSCO" slot.
             * Therefore, on enabling the receiver override the eSCO_AM_ADDR component
             * of JAL_ESCO_CTRL_REG (that was just previously setup in 
              * LSLCacc_Prepare_Tx_Rx_Connection()) in the case when the next slot is 
             * is "non-eSCO" slot.
              * EXCEPTION:
             * If the device is in Testmode, then always set the eSCO_LT_ADDR. Ie 
             * do not modify the eSCO_LT_ADDR previously set in 
             * LSLCacc_Prepare_Tx_Rx_Connection().
             */

            mHWreg_Create_Cache_Register(JAL_ESCO_CTRL_REG);
            mHWreg_Assign_Cache_Register(JAL_ESCO_CTRL_REG, _ctrl_reg_shadow.esco_ctrl);

            mHWreg_Assign_Cache_Field(JAL_ESCO_CTRL_REG, JAL_ESCO_LT_ADDR, 0);

            mHWreg_Move_Cache_Register(JAL_ESCO_CTRL_REG, _ctrl_reg_shadow.esco_ctrl);
        }
#endif
        /*
         * Only update the hardware registers if either of the following conditions
         * are satisfied:
         * 1. Requested RX_MODE is not FULL_WIN, or
         * 2. Requested RX_MODE is FULL_WIN and the device was not already
         *    scanning in the previous frame.
         */
        if ( (RXm_FULL_WIN != mHWreg_Get_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE)) || 
             ( (RXm_FULL_WIN == mHWreg_Get_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE)) &&
               (_was_previous_rx_slot_a_scan ==0)))
        {
            _LSLCacc_Move_CC_Shadow_to_Hardware();
            mHWreg_Store_Cache_Register(JAL_RX_CTRL_REG);
        }

#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
        if (mHWreg_Get_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE)
            != RXm_FULL_WIN)
        {
            t_devicelink *p_dev_link =
               LSLCpkt_Get_Active_Rx_Device_Link_Ref();
            HWradio_Update_Rx_Gain_If_Reqd(DL_Get_RSSI(p_dev_link)); 
        }
#endif

        if (RXm_FULL_WIN == mHWreg_Get_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE))
        {
            _was_previous_rx_slot_a_scan=1;
        }
        else
        {
            _was_previous_rx_slot_a_scan=0;
        }

        /*
         * Clear RX_MODE in cache to prevent subsequent receiving
         */
        mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE,
          RXm_RECEIVER_DISABLED);
        mHWreg_Move_Cache_Register(JAL_RX_CTRL_REG, p_reg_shadow->rx_ctrl);
    }
    else
    {
        /*
         * Disable the Received only in the case where the device is 
         * not currently scanning.
         */
        if (0 == _was_previous_rx_slot_a_scan)
        {
            HW_set_rx_mode(0);
        }
    }

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	{
        mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
        mHWreg_Assign_Cache_Register(JAL_EDR_CTRL_REG, _ctrl_reg_shadow.edr_ctrl);
        mHWreg_Assert_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_ENABLE);
        mHWreg_Store_Cache_Register(JAL_EDR_CTRL_REG);
	}
#endif
    /*
     *  Override any receiver control register settings here!
     */
}


/************************************************************************
 * LSLCacc_Discard_SCO_Payload
 *
 * Discards one SCO payload from SCO_OUT_Q. This procedure is required
 * where LMP prioritises over transmission of enqueued SCO, as otherwise
 * a latency creep is introduced on each LMP transmission.
 *
 ************************************************************************/
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
void LSLCacc_Discard_SCO_Payload(const t_devicelink *p_dev_link)
{
    u_int8 active_sco_fifo = p_dev_link->active_sco_fifo;

    if ((SCO_CFG_VIA_ACL == LSLCacc_Get_SCO_CFG(active_sco_fifo)) || 
		(BTtst_Loopback_SCO_Enabled()))
    {
        t_packet sco_packet_type = DL_Get_Tx_SCO_Packet_Type(p_dev_link);

        t_q_descr *p_qD = BTq_BB_Dequeue_Next(SCO_OUT_Q, active_sco_fifo, 
            (t_packetTypes)(1<< sco_packet_type));

        if (p_qD && p_qD->data)
        {
            BTq_Ack_Last_Dequeued(
                SCO_OUT_Q, active_sco_fifo, SYSconst_Get_Packet_Length(sco_packet_type));

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
            if (HCfc_Read_SYN_Flow_Control_Enable())
            {
                BTms_OS_Post(BTMS_OS_EVENT_HOST_TO_HC_FLOW_CONTROL);
            }
#endif
        }
    }
}
#endif
/************************************************************************
 * LSLCacc_Disable_Receiver
 *      Disables receiver for currently generated packet
 *      (later may need to check if all ok)
  ************************************************************************/
void LSLCacc_Disable_Receiver(void)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);
    mHWreg_Assign_Cache_Register(JAL_RX_CTRL_REG, p_reg_shadow->rx_ctrl);
    mHWreg_Assign_Cache_Field(JAL_RX_CTRL_REG, JAL_RX_MODE, RXm_RECEIVER_DISABLED);
    mHWreg_Move_Cache_Register(JAL_RX_CTRL_REG, p_reg_shadow->rx_ctrl);

    /*
     * Immediate disable the receiver
     */
    mHWreg_Store_Cache_Register(JAL_RX_CTRL_REG);

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    HWpta_Rx_Access_Complete();
#endif

    _was_previous_rx_slot_a_scan = 0;
}

/************************************************************************
 * LSLCacc_Process_No_Rx_Packet
 *
 * Defaults are
 *     if no packet is received or the received header is in error then
 *          FLOW = GO is assumed  (Remote Buffer is not full)
 *          ARQN = NAK is assumed
 ************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
void LSLCacc_Process_No_Rx_Packet(t_devicelink *p_dev_link)
{
    DL_Set_Remote_Rxbuf_Full(p_dev_link, FALSE);
    DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
    DL_Set_Rx_Packet_Type(p_dev_link, INVALIDpkt);
    DL_Set_Rx_Status(p_dev_link, RX_NO_PACKET);

    LSLCass_Record_No_Rx_Packet(p_dev_link);

    /*
     *
     * CM: 22 SEP 03
     * Need to determine if this is required for eSCO
     * 
     */ 
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
    if(SYSconfig_Get_Erroneous_Data_Reporting())
	{
		if((USLCsched_Get_Current_Frame_Activity()==SCO_SLAVE)
			|| (USLCsched_Get_Current_Frame_Activity()==SCO_MASTER))
		{
			_LSLCacc_Handle_Erroneous_SCO_Packet(p_dev_link);
		}
	}
#endif
    /*
     *  We must update previous ARQN in prepared outgoing packet
     *  if its on this device_link
     */
    _LSLCacc_Prepare_Tx_Complete_If_Same_Device(p_dev_link);

    LSLCstat_Inc_Rx_No_Packet_Cntr();
}

/************************************************************************
 * LSLCacc_Extract_Packet
 *
 *    Check packet for validity and update LSLC monitors accordingly
 *    If payload move to appropriate queue
 ************************************************************************/
void LSLCacc_Extract_Packet(t_devicelink *p_dev_link)
{
    t_TXRXstatus status;
    t_packet     packetType;

    mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG);
    mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);

    /*
     * Current status of packet being received.
     */
    status = DL_Get_Rx_Status(p_dev_link);

    /*
     * If Valid Header Then
     *     Process CRC and Payload
     * Else If Unexpected Header Status Then
     *     Process as No Packet Received
     * Endif
     */

    if (status == RX_NORMAL_HEADER || status == RX_BROADCAST_HEADER)
    {
        /*
         * Valid non-ID Packet,   Packet Header processed on Rx_Hdr_Intr
         */
        packetType = DL_Get_Rx_Packet_Type(p_dev_link);


#ifdef LC_RESTRICTED
/****************************************************************************
 *  Allow the restriction of packets into the LC
 *  Treat as an error packet if excluded by corresponding LC_RESTRICTED bit
 ****************************************************************************/
        if ( (1u<<packetType) & LC_RESTRICTED )
        {
             DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
             LSLCstat_Inc_Rx_Restricted_Packet_Cntr();
             status = RX_RESTRICTED_PACKET;
        }
        else
#endif

        {
            status = _LSLCacc_Process_Rx_CRC(p_dev_link, packetType);
        }

#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
        if((RX_CRC_ERROR == status) && (BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER))
        {
            t_LCHmessage msg_type = (t_LCHmessage)HW_get_rx_l_ch();

            if (msg_type != LMP_msg)
            {
                /*
                 * Evaluate BER in payload with test mode tester loopback tests.
                 */
                HW_toggle_rx_buf();
                BTtmt_Verify_Tester_After_Rx(DL_Get_Device_Index(p_dev_link),
                    (const u_int8 *)HW_get_rx_acl_buf_addr(), 
                    (t_length)HW_get_rx_len(), LCH_start);
            }
        }
#endif

        if ((RX_OK == status) ||  
			((RX_CRC_ERROR == status) && (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) &&
			 (LMP_msg != (t_LCHmessage)HW_get_rx_l_ch()) ))

        {
            /*
             * Valid reception of an AUX1, DM*, DH*, DV, SCO, FHS, NULL or POLL packet
             */
#if ((LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED==1) || (LC_DEFERRED_LOOPBACK_PAYLOAD_EXTRACTION_SUPPORTED==1))


            if ((LSLCacc_Is_Deferred_Extraction_Enabled_()==TRUE))
            {
				u_int8 msg_type;

			    if (mLSLCacc_Is_ACL_Packet(packetType))
					G_rx_loopback_len = mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_LEN);
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
			    else if (is_SYN_Packet(packetType))
				    G_rx_loopback_len = BTtst_Get_Loopback_Packet_Length();
#endif				
				else 
					G_rx_loopback_len = 0;

                // Ensure that the packet length is not greater than the Maximum Loopback 
                // Length.. 
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
				if (G_rx_loopback_len > BTtst_Get_Loopback_Packet_Length())
					G_rx_loopback_len = BTtst_Get_Loopback_Packet_Length();
#endif

                G_msg_type = (u_int8)mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH);

			    msg_type = G_msg_type; 

				if ((BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) &&
                    (msg_type != LMP_msg ) && (G_rx_loopback_len !=0))
				{

#if 1 // GF Added 22 Jan 2010 - Removed as caused problems on 3-DH1 and other tests
					HW_toggle_rx_buf();
#endif
					G_msg_type = (u_int8)mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH);

					msg_type = G_msg_type;

                    LSLCacc_Setup_Deferred_Extraction_(p_dev_link, packetType);
                    LSLCacc_Set_Deferred_Extraction_Pending_(TRUE);

				}
				else
				{
		            _LSLCacc_Extract_Payload(p_dev_link, packetType);
                    LSLCacc_Set_Deferred_Extraction_Pending_(FALSE);


               /*
                *
                * In the case of deferred payload extraction, need to
                * force update of peer L2CAP flow before extracting
                * payload, yet turn on baseband flow control.
                *
                * TODO: does calling this twice have any side effects?
                *       should this move here from Extract_ACL_Packet
                *       altogether?
                *
                * The Peer L2CAP Flow Status may need to be checked before
                * sending next Tx packet (rf.
                *_LSLCacc_Prepare_Tx_Complete_ARQN_If_Same_Device() )
                */

				}

                status = RX_OK;
            }
            else /* Control or Non-DV SCO */
            {
                status = _LSLCacc_Extract_Payload(p_dev_link, packetType);
                LSLCacc_Set_Deferred_Extraction_Pending_(FALSE);
            }
#else
            status = _LSLCacc_Extract_Payload(p_dev_link, packetType);
#endif
        }

        /*
         * Can check based on context if packet-type is still valid.
         */

		// If we have a CRC on a SCO packet then we handle it specially if we 
		// have Erroneous_Sync_Data supported
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
    if(SYSconfig_Get_Erroneous_Data_Reporting())
	{
		if ((RX_CRC_ERROR == status) && (is_eSCO_Packet(packetType)))
		{
			t_q_descr *p_qD = 0;
			t_sco_info *p_sco_link;
			t_sco_fifo hw_active_sco_fifo;
			t_length pkt_contents_length;
		    boolean extract_payload_contents = TRUE;

			hw_active_sco_fifo = p_dev_link->active_sco_fifo;
			p_sco_link = LMscoctr_Find_SCO_By_SCO_Index(hw_active_sco_fifo);

					/*
					* Now do not want to extract a packet if the device is in the 
					* SCO_MASTER_DEACTIVATION_PENDING state.
					*/
			if(p_sco_link && (p_sco_link->state == SCO_MASTER_DEACTIVATION_PENDING))
				extract_payload_contents = FALSE;
			
			// We have a CRC on a Sync packet. 
			// 1/ We need to check are we in an eSCO interval.
			// 2/ If this is the First Pkt in the interval we need to enqueue it BUT not commit.
			//    The descriptor is left uncommited in case there are subsequent good packets.
			// 3/ If this is not the First Pkt then we overwrite the data previosuly written to the Queue.
			// 4/ If this is the end of the eSCO interval then we commit any data previously written to the Queue.
			
			// How do I know when I am at the end of a window. - Next to Check next Master/Slave Activity.
			if((DL_Get_Rx_eSCO_Pending(p_dev_link)) /*&& 	(DL_Get_Rx_SYNC_Erroneous_Data_Status(p_dev_link) > SYNC_DATA_WITH_ERRORS) */ &&
				(extract_payload_contents == TRUE) && (LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(hw_active_sco_fifo)))
			{
				u_int32 current_clk;

				pkt_contents_length = DL_Get_eSCO_Rx_Length(p_dev_link);

				/*
				* If converting to a sample size > 8 AND the air mode is
				* not transparent, the hardware will supply
				* twice as many bits to the host controller for delivery
				* to the host
				*/
				if(!LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(hw_active_sco_fifo) )
				{
					pkt_contents_length <<=1;
				}
				
				if(DL_Get_Rx_SYNC_Erroneous_Data_Status(p_dev_link) > SYNC_DATA_WITH_ERRORS)
				{
					// Have I previously Rx'ed eSCO_Erroneous_Data
					// If I have then I already have a qD stored in ????.
					p_qD = DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link);
					
					if (!p_qD)
					{
						// Dont have a descriptor. So must get one. 
						if (p_dev_link->sco_commit_pending == FALSE)
						{
							u_int16 packet_length = pkt_contents_length;
#if 1 // TK 15 MAY 2012: HANDLE MULTIPLE ERRONEOUS BUFFERS PER PACKET
							while (packet_length > PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH)
							{
							    p_qD = BTq_Enqueue(SCO_IN_Q, hw_active_sco_fifo , PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH);
								if (p_qD)
								{ 
									if (!DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link)) // first buffer
										DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link,p_qD); // set first buffer

									BTq_Commit(SCO_IN_Q, hw_active_sco_fifo); // note: data will be stored later
									BTQueues[SCO_IN_Q+hw_active_sco_fifo].num_entries--; // protect
								}
								packet_length -= PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
							}
#endif
							p_qD  = BTq_Enqueue(SCO_IN_Q, hw_active_sco_fifo , packet_length);
							// GF 28 Jan 2010 Conf_10
							if (p_qD)
								p_dev_link->sco_commit_pending = TRUE;
						}
						if (p_qD)
						{
							if (!DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link))
							    DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link,p_qD);
						}
					}
					if (p_qD)
					{
						u_int8* rx_data_ptr = (u_int8*)HW_get_rx_sco_buf_addr();
						
						DL_Set_Rx_SYNC_Erroneous_Data_Status(p_dev_link,SYNC_DATA_WITH_ERRORS);
						
						// Copy the Errored CRC data onto the Queue. But dont commit yet.

#if 1 // TK 15 MAY 2012: HANDLE MULTIPLE ERRONEOUS BUFFERS PER PACKET
                        while (p_qD)
                        {
                            if (p_qD->next)
                            {
                                _LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data, (void*)rx_data_ptr, PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH);
                                rx_data_ptr += PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
                                pkt_contents_length -= PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
                            }
                            else
                                _LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data, (void*)rx_data_ptr, pkt_contents_length);

							p_qD->message_type = SYNC_DATA_WITH_ERRORS;
							p_qD = p_qD->next;
                        }

						p_qD = DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link); // restore p_qD from above iterations
#else
						p_qD->message_type = SYNC_DATA_WITH_ERRORS;
						_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data, (void*)rx_data_ptr, pkt_contents_length);
#endif
					} // end if p_qD
					
				} // End > SYNC_DATA_WITH_ERRORS
				
				
				// May have to play with the current clock a little (i.e add to it) to get the correct value to 
				// find end of the eSCO ReTx window.
				
				if (DL_Get_Rx_SYNC_Erroneous_Data_Status(p_dev_link) >  SYNC_DATA_GOOD) 
				{
					current_clk = (LC_Get_Piconet_Clock_Ex((p_sco_link->device_index),TRUE) + 2) & 0x0FFFFFFE;
					
					/* Determine if end of eSCO window. If it is then commit.
					*/
					if (_is_end_eSCO_window(p_sco_link, p_dev_link,current_clk) &&
						p_dev_link->sco_commit_pending)
					{
						BTq_Commit(SCO_IN_Q, hw_active_sco_fifo );
#if 1 // TK 15 MAY 2012: HANDLE MULTIPLE ERRONEOUS BUFFERS PER PACKET
						while (p_qD && p_qD->next)
						{
							BTQueues[SCO_IN_Q+hw_active_sco_fifo].num_entries++; // un-protect
							p_qD = p_qD->next;
						}
#endif
						DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link,0);
						DL_Set_Rx_SYNC_Erroneous_Data_Status(p_dev_link,0xFF);
						p_dev_link->esco_end_retransmission_window_timer = 
							p_sco_link->esco_end_retransmission_window_timer;
						p_dev_link->sco_commit_pending = FALSE;
						
					}
				}
			}  // End of DL_Get_Rx_eSCO_Pending(p_dev_link)
		} // End ((RX_CRC_ERROR == status)
		} // end SYSconfig_Get_Erroneous_Data_Reporting
#endif

        if (RX_OK != status)
        {
            packetType = INVALIDpkt;
#if (LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED==1)
            LSLCacc_Set_Deferred_Extraction_Pending_(FALSE);
#endif
        }

        /*
         *  We must update previous ARQN in prepared outgoing packet
         *  if its on this device_link
         */
        _LSLCacc_Prepare_Tx_Complete_If_Same_Device(p_dev_link);
    }
    else
    {
        if (status != RX_HEC_ERROR && status != RX_AMADDR_ERROR)
        {
            /*
             * PKD without PKD_HDR occurs when AMADDR Error. Must
             * not process RSSI or update channel assessment data
             * on this occurance.
             *
             * Process as No Packet Received otherwise.
             */

#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
            lslc_acc_recent_rssi_available_ = FALSE;
#endif

            if(HW_get_rx_am_addr() != DL_Get_Am_Addr(p_dev_link))
            {
                if (DL_Get_Role_Peer(p_dev_link) == SLAVE)
                {
                    LSLCstat_Inc_AMADDR_Error_Cntr();
                }

                DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
                status = RX_AMADDR_ERROR;
            }
            else
            {
                LSLCacc_Process_No_Rx_Packet(p_dev_link);
                status = RX_MISSED_HEADER;
            }
        }
        else
        {
             /*
              * status as recorded during RX_HDR processing
              */

#if ((LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED==1) || (LC_DEFERRED_LOOPBACK_PAYLOAD_EXTRACTION_SUPPORTED==1))


            if ((LSLCacc_Is_Deferred_Extraction_Enabled_()==TRUE))
            {
				u_int8 msg_type = 0;
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
				G_rx_loopback_len = BTtst_Get_Loopback_Packet_Length();
#endif
                G_msg_type = LCH_start;
				if (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) 
				{
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
					packetType =  BTtst_Get_Loopback_Packet();
#endif
					msg_type = G_msg_type; 
				}

				if ((BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) &&
                    (msg_type != LMP_msg ) && (G_rx_loopback_len !=0))
				{
                  //  LSLCacc_Setup_Deferred_Extraction_(p_dev_link, packetType);
                 //   LSLCacc_Set_Deferred_Extraction_Pending_(TRUE);
				}
				else
				{
#if 0 // GF 28 Jan 2010 Conf_4
      // Below Code should do nothing when not in testmode
      // Removed for now 
		            _LSLCacc_Extract_Payload(p_dev_link, packetType);
#endif
                    LSLCacc_Set_Deferred_Extraction_Pending_(FALSE);
				}

               /*
                *
                * In the case of deferred payload extraction, need to
                * force update of peer L2CAP flow before extracting
                * payload, yet turn on baseband flow control.
                *
                * TODO: does calling this twice have any side effects?
                *       should this move here from Extract_ACL_Packet
                *       altogether?
                *
                * The Peer L2CAP Flow Status may need to be checked before
                * sending next Tx packet (rf.
                *_LSLCacc_Prepare_Tx_Complete_ARQN_If_Same_Device() )
                */
#if 1 // GF Post BQB - The msg_type was not initialised in some cases
				msg_type = G_msg_type;
#endif
               if ((msg_type != LMP_msg) &&
                   (BTtst_Get_DUT_Mode() != DUT_ACTIVE_LOOPBACK))
               {

                   DL_Set_Local_Tx_L2CAP_Flow(p_dev_link,
                       (t_flow) mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG,
                                    JAL_RX_P_FLOW));
               }

                status = RX_OK;
            }
            else /* Control or Non-DV SCO */
            {
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
				if (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) 
					packetType =  BTtst_Get_Loopback_Packet();
#endif
#if 0 // GF 28 Jan 2010 Conf_5 consider turning below code off
                status = _LSLCacc_Extract_Payload(p_dev_link, packetType);
                LSLCacc_Set_Deferred_Extraction_Pending_(FALSE);
#endif
            }
#endif

        }
        packetType = INVALIDpkt;
    }

    DL_Set_Rx_Packet_Type(p_dev_link, packetType);

    LSLCstat_Inc_Rx_Packet_Cntr(packetType);

    /*
     * Packet initial information logged on packet header
     * Log the final status and register (CRC/Payload header may have changed!)
     */
#ifndef BLUETOOTH_MODE_LE_ONLY
    LSLCstat_Log_Rx_Final_Status(status);
#endif//BLUETOOTH_MODE_LE_ONLY

    if (status == RX_OK)
    {
        if (DL_Get_Rx_Status(p_dev_link) == RX_BROADCAST_HEADER)
        {
            LSLCstat_Inc_Rx_Broadcast_Packet_Cntr();
            status = RX_BROADCAST_PACKET;
        }
        else
        {
            LSLCstat_Inc_Valid_Packet_Cntr();
        }
    }
    else
    {
       LSLCstat_Inc_Invalid_Packet_Cntr();
    }
    /*
     * Record status of last received packet
     */
    DL_Set_Rx_Status(p_dev_link, status);


}

/************************************************************************
 * _LSLCacc_Process_Rx_CRC
 *
 * Processes the Rx Payload CRC and update Expected Rx SEQN/Rx ARQN
 *
 * Notes:
 * 1. POLL/NULL/SCO have no CRC check,  AUX1 no CRC/SEQN/ARQ check
 * 2. For duplicates, the status = RX_DUPLICATE and payload is ignored.
 * 3. Broadcast LMP/L2CAP Start messages will not be filtered out.
 ************************************************************************/
static t_TXRXstatus
   _LSLCacc_Process_Rx_CRC(t_devicelink *p_dev_link, t_packet packetType)
{
    t_TXRXstatus status = RX_OK;
    mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG);
    mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);

    if (packetType==FHSpkt)         /* No impact on ARQN and SEQN */
    {
        if (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_CRC_ERR) )
        {
            LSLCstat_Inc_CRC_Error_Cntr();
            status = RX_CRC_ERROR;
        }
    }

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
    else if ( is_ACL_CRC_Packet(packetType) || is_eSCO_Packet(packetType))

#else

    else if ( is_ACL_CRC_Packet(packetType) )

#endif
    {
        /*
         * This code cannot be processed at Rx_Packet_Header since
         * LCH field, used for Broadcast, is in packet header.
         * The expected sequence number must match the received except
         * for a Broadcast LMP/L2CAP message.
         *
         * Increment Rx Number of CRC packets
         * if Packet is not a Broadcast Message then
         *     if sequence number mismatch and not in Testmode Loopback and
         *         not an eSCO packet then
         *         status = RX_DUPLICATE, ACK packet
         *     else if CRC error or L_CH == LCH_null
         *     {    
         *         if (not eSCO) || 
         *            ( is eSCO && no eSCO payload received in this eSCO window)
         *               status = RX_CRC_ERROR, NAK packet
         *         else 
         *             status = RX_OK, ACK packet,
         *     }
         *     else
         *         status = RX_OK, ACK packet, toggle sequence
         *     endif
         *
         * else ! Packet is a Broadcast Message
         *     if L2CAP continue AND broadcast sequence number mismatch then
         *         status = RX_BROADCAST_DUPLICATE
         *         ! expected broadcast seqn number does not change
         *     else if CRC error
         *         status = RX_CRC_ERROR
         *     else
         *         expected broadcast seqn number is a toggle of rx seqn
         *     endif
         * else
         *
         * endif
         *
         * NOTE eSCO:
         * eSCO does not obey the standard SEQN rules. See figure 7.14 (BT1_2_D6)
         * Accept any eCO payload in an eSCO window if no valid payload has already
         * been received in the current eSCO window (ie accept irrespective of the 
         * SEQN). Once a valid eSCO payload has been received in an eSCO window, 
         * ignore all subsequent payloads in the eSCO window and always set
         * ARQN = ACK on the eSCO LT_ADDR.
         */

        if(is_ACL_CRC_Packet(packetType))
        {
            DL_Inc_Rx_ACL_Data_Packets(p_dev_link);
        }

        if (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR)!=0)
        {
            /*
             * Not a broadcast, Filter any duplicates
             * If in Testmode loopback or Standalone Rx then Process all duplicates
             */
#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
            if (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK)
            {
                /*
                 * Some testers (eg Anritsu) may not enforce SEQN correctly
                 * based on ACKs from the slave in testmode loopback, so force
                 * re-alignment of rx_expected_seqn.
                 */
                DL_Set_Rx_LT_Expected_Seqn(p_dev_link, mHWreg_Get_Cache_Bit(
                    JAL_RX_STATUS_GP_REG, JAL_RX_SEQN));
            }
#endif
            if ( mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_SEQN) !=
                    DL_Get_Rx_LT_Expected_Seqn(p_dev_link)
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
                && (!is_eSCO_LT(p_dev_link->rx_lt_index))
#endif

#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
                && (BTtst_Get_DUT_Mode() != DUT_STANDALONE_RXTEST)
                && (BTtst_Get_DUT_Mode() != DUT_TESTMODE_TESTER)
#endif
                )
            {
                /*
                 * V1.1 Specification, Duplicates with CRC error must be ACKed
                 * However, many competitors NAK and test house may expect it.
                 */
                t_arqn rx_arqn = ACK;

                LSLCstat_Inc_Rx_Duplicate_Cntr();

#if (PRH_BS_CFG_SYS_NAK_DUPLICATES_IF_CRC_SUPPORTED==1)
                /*
                 * NAK if CRC error and feature is currently enabled
                 */
                if (SYSconfig_Is_Feature_Supported(
                      PRH_BS_CFG_SYS_NAK_DUPLICATES_IF_CRC_FEATURE) && 
                      mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_CRC_ERR) )
                {
                    rx_arqn = NAK;
                }
#endif
                DL_Set_Rx_LT_Previous_Arqn(p_dev_link, rx_arqn);
                status = RX_DUPLICATE;
            }

            /*
             * Filter any CRC errors or messages with incorrect message type
             */
            else if ( mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_CRC_ERR) || 

                ((mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH) == LCH_null) && 
                (mLSLCacc_Is_ACL_Packet(packetType))) 
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
                || (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_EDR_SYNC_ERR) &&
                     (is_EDR_Packet(packetType)))
#endif                
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
					 || HW_get_pta_grant_rx_denied()
                //|| (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_PTA_GRANT_RX_DENIED))
#endif
                )
            {
#if 0 // Debug Only -- Mechanism to record EDR_Sync Errors
      // These will show up on data tester as Broadcast Dublicates.

				if(mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_EDR_SYNC_ERR) &&
                     (is_EDR_Packet(packetType)))
				{

					LSLCstat_Inc_Broadcast_Duplicate_Cntr();
				}
				else
#endif
				{
                    LSLCstat_Inc_CRC_Error_Cntr();
                    LSLCass_Record_Rx_CRC_Event_Channel(p_dev_link);
                }
                DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
                if(is_ACL_CRC_Packet(packetType))
                {
                    DL_Inc_Rx_ACL_Data_Packets_Nacked(p_dev_link);
                }

                status = RX_CRC_ERROR;

#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
                /* 
                 * Bad CRC packets (and duplicates ?) are looped back with ARQN=NAK 
                 */
                if ((BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) &&
                  (LMP_msg != (u_int8)mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH)))
                {
                    status = RX_OK;
                }
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
            /* 
             *NOTE eSCO:
             * eSCO does not obey the standard SEQN rules. See figure 7.14 (BT1_2_D6)
             * Accept any eSCO payload in an eSCO window if no valid payload has already
             * been received in the current eSCO window (ie accept irrespective of the 
             * SEQN). Once a valid eSCO payload has been received in an eSCO window, 
             * ignore all subsequent payloads in the eSCO window and always set
             * ARQN = ACK on the eSCO LT_ADDR.
             */
            if (is_eSCO_LT(p_dev_link->rx_lt_index) && (!DL_Get_Rx_eSCO_Pending(p_dev_link)))
            {
                DL_Set_Rx_LT_Previous_Arqn(p_dev_link, ACK);
                status = RX_OK;
            }
#endif
            }

#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
            /* 
             * If I am a testmode tester and an L2CAP packet Then ACK the packet
             */
            else if (BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER && ( 
                mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH) != LMP_msg ))
            {
                DL_Set_Rx_LT_Previous_Arqn(p_dev_link, ACK);
            }
#endif

            /* 
             * If L2CAP flow control is Stopped AND its not a zero-length L2CAP packet
             *  AND
             *  I don't have an LMP packet
             * Then 
             *     NAK the packet
             * Else
             *    Process Valid Packet
             * Endif
             */

            else if ( ((DL_Get_Local_Rx_L2CAP_Flow(p_dev_link) == STOP)
               && (mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_LEN)!=0) )
               && /* Its an L2CAP Packet */
               ( mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH) != LMP_msg ) )
            {
                DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
                status = RX_RESTRICTED_PACKET;
            }

            /*
             * Valid packet received.
             */
            else
            {
                /*
                 * For eSCO links the Toggling of the Tx_Seqn 
                 * will be handled by the Link Policy
                 * object at the start of each eSCO window.
                 */ 
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
                if (is_ACL_LT(p_dev_link->rx_lt_index))
#endif
                {
                    DL_Toggle_Rx_LT_Expected_Seqn(p_dev_link);
                }
                DL_Set_Rx_LT_Previous_Arqn(p_dev_link, ACK);
                status = RX_OK;
            }
         }
         else
         {
            /*
             * Broadcast packet received.
             */
			 /* To protect the Queue for Connection Oriented Data we reject all Broadcast duplicates of LCH_Continue and any duplicate of LCH_Start which occurs 
			  * when the Queue is NOT empty.
			  */
             if ( ((mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH) == LCH_continue) ||
			       ((mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH) == LCH_start ) &&
				   !BTq_Is_Queue_Empty(L2CAP_IN_Q,0)) ) &&
                   (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_SEQN)!=
                   DL_Get_Rx_Expected_Broadcast_Seqn(p_dev_link) )
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
                    && !USLCinq_Is_Extended_Inquiry_Response_Frame()
#endif
                   )
             {
                 LSLCstat_Inc_Broadcast_Duplicate_Cntr();
                 status = RX_BROADCAST_DUPLICATE;
             }
             else if ( mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_CRC_ERR) ||
              (mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH) == LCH_null) )
             {
                 LSLCstat_Inc_CRC_Error_Cntr();
                 DL_Inc_Rx_ACL_Data_Packets_Nacked(p_dev_link);
                 status = RX_CRC_ERROR;
             }
#if 0 // to be evaluated (previously used on SHOGA) -
             /*
              * Avoid broadcast LMP flooding the LMP_IN_Q by discarding potential
              * duplicates when LMP_IN_Q is not empty.
              */
             else if ( (mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH)
                           == LMP_msg) && !BTq_Is_Queue_Empty(LMP_IN_Q,0) &&
                       (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_SEQN)!=
                        DL_Get_Rx_Expected_Broadcast_Seqn(p_dev_link) ) )
             {
                 LSLCstat_Inc_Broadcast_Duplicate_Cntr();
                 status = RX_BROADCAST_DUPLICATE;
             }
#endif
             else
             {
                 DL_Set_Rx_Expected_Broadcast_Seqn(p_dev_link,
                  !mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_SEQN) );
                 status = RX_OK;
             }
         }
    }
#if (PRH_BS_DEV_NAK_NON_CRC_PACKETS_WITH_INCORRECT_SEQN ==1)
    else if ( !is_ACL_CRC_Packet(packetType) 
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
        && is_ACL_LT(p_dev_link->rx_lt_index)
#endif
        )
    {
        if (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_SEQN) ==
                    DL_Get_Rx_LT_Expected_Seqn(p_dev_link))
        {
            /*
             * If the device receives a packet other than DH, DM, DV or EV with the
             * SEQN bit inverted from that in the last header succesfully received on the
             * same LT_ADDR, it shall set the ARQN bit to NAK until a DH, DM, DV or EV
             * packet is successfully received. [BT1.2, section 7.6.2.2]
             * 
             * if POLL/NULL/SCO then actual seqn should be !rx_expected_seqn
             */

			// GF 24 Jan 2010 - Ignore a sequence mismatch in TestMode
			if (BTtst_Get_DUT_Mode() != DUT_ACTIVE_LOOPBACK)
				DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
        }
        else
        {
            /*
             * If the device receives a packet other than DH, DM, DV or EV with the
             * SEQN bit equal to that in the last header succesfully received on the
             * same LT_ADDR, it shall set the ARQN bit to the previous ARQN.
             * [BT1.2, section 7.6.1, Figure 7.13]
             */
            
            /*
             * Do nothing !
             * Previous ARQN will be used for the next transmission.
             */
			// Change :- Conf_1
			// GF 25 Jan 2010 -- 2:40pm
			// Some Risk Here !! Run through full ATS
			if (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) 
				DL_Set_Rx_LT_Previous_Arqn(p_dev_link, ACK);
        }
    }
		      

#endif


    return status;
}

/************************************************************************
 * LSLCacc_Process_Rx_Header
 *
 * Processes the packet header
 *
 * Notes:
 * 1. Initialisation of SEQN/NAK at receive end via device link.
 *      For 1st packet master to slave  (POLL)      SEQN==1, ARQN==NAK
 *      For 1st packet slave  to master (NULL|ANY?) SEQN==1, ARQN==NAK
 * 2. At this stage if a Rx ACK, the last Tx packet can be acked.
 ************************************************************************/

void LSLCacc_Process_Rx_Header(t_devicelink *p_dev_link)
 {
	 t_packet packetType;
	 t_TXRXstatus status;
	 t_deviceIndex device_index = DL_Get_Device_Index(p_dev_link);
	 mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	 mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
	 mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
#endif
	 mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);
	
#if 1 // GF 7 Jan 2011

    if (HW_get_slave()==1)
    {        
        /*
         * This block is responsible for recording this device's
         *      current receive bt clock
         *      current intra-slot offset 
         *      current bt clock offset from native clock
         */
        
        /* last frequency clock is current receive bt clock */
        t_clock bt_clock = LSLCfreq_Get_Last_Frequency_Clock() & 0x0ffffffc;                
        
        /* intraslot transitioning in hardware so avoid race on read */
        u_int32 new_intraslot = HW_Get_Intraslot_Avoid_Race();
        
        /* use new intraslot to determine if native will transition in rx slot */
        t_clock current_frame_nt_clk = HW_Get_Native_Clk_Avoid_Race();
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
        if(new_intraslot >= 625 && !(current_frame_nt_clk&2))
        { /* native already transitioned => adjust back */
            current_frame_nt_clk -= 4;
		//	nt_clk_transition = 0x1;
        }
#endif

        
        DL_Set_Piconet_Clock_Last_Access(p_dev_link, bt_clock);        
        DL_Set_Local_Slot_Offset(p_dev_link, (u_int16)new_intraslot);        
        {
            t_clock clk_offset_adjusted;

#if 1 //(PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
            clk_offset_adjusted = BTtimer_Clock_Difference((current_frame_nt_clk & 0x0ffffffc),bt_clock);
#else
            clk_offset_adjusted = (bt_clock - (current_frame_nt_clk & 0x0ffffffc));
#endif
            DL_Set_Clock_Offset(p_dev_link, clk_offset_adjusted);
        }
    }
#endif

	 if ( mHWreg_If_Cache_Bit_is_0(JAL_RX_STATUS_GP_REG, JAL_HEC_ERR) &&
         mHWreg_If_Cache_Bit_is_0(JAL_RX_STATUS_GP_REG, JAL_AM_ADDR_ABORT))
	 {
	 /*
	 * Header is ok.
	 * if (am_addr is 0) Then
	 *    packetType = Rx Packet Type
	 *    if (packetType == FHSpkt)
	 *        status = RX_NORMAL_HEADER
	 *    else
	 *        status = RX_BROADCAST_HEADER
	 *    endif
	 * else if ( not Expected am_addr)
	 *    status = RX_AMADDR_ERROR,     packetType = INVALIDpkt
	 *    if (Device is Master)
	 *        Increment AMADDR Error Counter
	 *    endif
	 * else
	 *    process header information ARQN/FLOW
	 * endif
	 *
         */
		packetType = (t_packet) mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_TYPE);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
		if ( is_ACL_CRC_Packet(packetType) && (DM1 != packetType))
		{
			/*
			 * Potentially an EDR packet.  Need to pre-pend the PTT_ACL  bit
             */
			 packetType = ( mHWreg_Get_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ACL) << 4) |
				 packetType;
			//- TODO: LC notice demod rx packet type.(EDR_2MBITS_PACKET_MASK/EDR_3MBITS_PACKET_MASK)
			
		 }
		 else if ( is_SYN_Packet(packetType) ) 
		 {
			/*
			 * Potentially an EDR packet.  Need to pre-pend the PTT_ESCO  bit
             */
			 packetType = ( mHWreg_Get_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO) << 4) |
				 packetType;
             //- TODO: LC notice demodem rx packet type.(EDR_2MBITS_PACKET_MASK/EDR_3MBITS_PACKET_MASK)
             
             
#if 0  //  debug only - Temp force NAK of every second eSCO.
			 if (test_nak_next_esco==1)
			 {
				 DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
				 LSLCass_Record_Rx_HEC_Event_Channel(p_dev_link);
				 BTtmt_Record_Rx_Packet_Error();
				 LSLCstat_Inc_HEC_Error_Cntr();
				 status = RX_HEC_ERROR;
				 packetType = INVALIDpkt;
				 test_nak_next_esco = 0;
				// HW_set_hec_err(1);
				 mSetHWEntry(JAL_HEC_ERR, 1); 
				 goto eSCONak;

			 }
			 else
			 {
				 test_nak_next_esco = 1;
			 }
#endif
			

		 }
#endif 
		 if ( mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR)==0)
		 {            
			if (packetType == FHSpkt)
			 {
				 status = RX_NORMAL_HEADER;
			 }
			 else
			 {
				 status = RX_BROADCAST_HEADER;
			 }
		 }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
		 else if ( DL_Decode_Valid_LT_address(p_dev_link, (u_int8)(mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR))) == FALSE )
#else
		 else if ( mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR) != DL_Get_Am_Addr(p_dev_link))
#endif
		 {                                   /* No impact on ARQN           */
			 if (DL_Get_Role_Peer(p_dev_link) == SLAVE)
			 {
				 LSLCstat_Inc_AMADDR_Error_Cntr();
			 }
			 status = RX_AMADDR_ERROR;
			 packetType = INVALIDpkt;
		 }
		 else
		 {      /* Valid Non-Broadcast Header   */
				/*
				 * Report that this device is active for e.g.
				 * Supervision Timeout monitoring
				 * Only report the device activity when the device is in the
				 * connected state.
				 */
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
  			 p_dev_link->rx_lt_index = ((mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR)
                 == DL_Get_Am_Addr(p_dev_link))?(ACL_SCO_LT):(ESCO_LT)); // TK 28/3/2012
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
			 if ( NOT_CONNECTED != DL_Get_Ctrl_State(p_dev_link))
			 {
				 LC_Report_Device_Activity(device_index);
			 }
#endif//BLUETOOTH_MODE_LE_ONLY
			 /*
             * Process FLOW/ARQN information valid since header is valid
             */
			 DL_Set_Remote_Rxbuf_Full(p_dev_link,
				 mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_FLOW) == STOP );
			 
			 if (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_FLOW) == STOP)
			 {
			 /*
			 * Flow==Stop is an implicit NACK 
			 * Must retransmit original packet (before STOP) on GO indication
			 * Descriptors (and Ack_Pending) are now persistent to ensure packet 
			 * changing due to channel quality does not affect data integrity
			 *    Do not set DL_Set_Tx_Ack_Pending(p_dev_link, FALSE);
                 */
			 }
			 

			 /*
             * If Previous Tx Packet is ACKed Then Confirm to Queue, Toggle SEQN
             */
			 else if (  (mHWreg_Get_Cache_Bit(JAL_RX_STATUS_GP_REG, JAL_RX_ARQN)
				         && DL_Get_Rx_LT_Tx_Ack_Pending(p_dev_link))
				 ||
				 // GF 13 December 2009
				 ((BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) && DL_Get_Rx_LT_Tx_Ack_Pending(p_dev_link))
				 )
				 
				 
			 {
				 DL_Set_Rx_LT_Tx_Ack_Pending(p_dev_link, FALSE);
				 /*
                 *
                 * CM: 22 SEP 03
                 * For eSCO links the Toggling of the Tx_Seqn 
                 * will be handled by the Link Policy
                 * object at the start of each eSCO window.
                 * DL_Toggle_Rx_LT_Seqn() has been moved to ACL
                 * code below.
                 */ 
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
				 if (is_ACL_LT(p_dev_link->rx_lt_index))
#endif
				 {
                     DL_Toggle_Rx_LT_Seqn(p_dev_link);
					 
#if (PRH_BS_DEV_TOGGLE_TX_SEQN_ON_TX_CRC_PACKET ==1)
                     DL_Set_Tx_Seqn_Not_Active(p_dev_link);
#endif
					 
#if (PRH_BS_DEV_ADOPT_ERRATUM_E2088==0)
					 /*
                     * Erratum E2088 is concerned with Testmode and SEQN
                     * When not active must force a SEQN change on Rx ACK
                     * To support this when E2088 not enforced, the Ack Pending
                     * flag is set (but 
                     * Test modes should only have LMP packets on queues
                     * Only ACK L2CAP when not DUT_ACTIVE, always ACK LMP
                     */
					 if ((BTtst_Get_DUT_Mode() <= DUT_ENABLED)
						 || (LMP_msg == DL_Get_Tx_Message_Type(p_dev_link)) ||
						 (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK))
#endif
					 { 
						 
						DL_Inc_Tx_ACL_Data_Packets_Acked(p_dev_link);
						LC_Ack_Last_Tx_Packet(device_index);
						LSLCstat_Inc_Rx_Ack_Processed_Cntr();
						 
					 }
				 }
			 }
			 
			 
#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
			 else if ((BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER)
				 && DL_Get_Rx_LT_Tx_Ack_Pending(p_dev_link))
			 {
			 /*
			 * => ARQN=NAK, DUT_TESTMODE_TESTER and pending ACK
			 * If a Tx data packet Then toggle SEQN
                 */
				 if ( (LMP_msg != DL_Get_Tx_Message_Type(p_dev_link)) )
				 {
                     DL_Set_Rx_LT_Tx_Ack_Pending(p_dev_link, FALSE);
                     DL_Toggle_Rx_LT_Seqn(p_dev_link);
                     LSLCstat_Inc_Rx_Ack_Processed_Cntr();
				 }
			 }
#endif
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
			 //            if (is_eSCO_LT(p_dev_link->rx_lt_index) && (packetType==HV3))
			 /*
             * If the packetType is percieved to be HV3, check
             * a) is it on the eSCO LT_ADDR.
             * OR
             * b) is the local device in testmode AND is the
             *    testmode scenario an EV3 scenario.
             *
             * If so, change the packetType to EV3.
             */
			 {
				 extern t_tm_control volatile testmode_ctrl;
				 
				 if( (packetType == HV3) && 
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
					 /* & not EDR eSCO */
					 (!mHWreg_Get_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO)) &&
#endif
					 (   
					 is_eSCO_LT(p_dev_link->rx_lt_index) || 
					 ((BTtst_Get_DUT_Mode() >= DUT_ENABLED) && 
					 (testmode_ctrl.packetType == EV3))  ||
					 ((DUT_TESTMODE_TESTER == BTtst_Get_DUT_Mode()) && 
					 (HW_get_am_addr() == HW_get_esco_lt_addr()))
                     )
					 )
				 {
					 packetType = EV3;
				 }
			 }
			 
#endif
			 status = RX_NORMAL_HEADER;
        }
    }
    else if (mHWreg_If_Cache_Bit_is_1(JAL_RX_STATUS_GP_REG, JAL_AM_ADDR_ABORT))
    {
        DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
        LSLCstat_Inc_AMADDR_Error_Cntr();
        status = RX_AMADDR_ERROR;
        packetType = INVALIDpkt;
    }
    else /* JAL_HEC_ERR ==1  */
    {
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
		if(SYSconfig_Get_Erroneous_Data_Reporting())
		{
			// GF 29 November 2008
			if((USLCsched_Get_Current_Frame_Activity()==SCO_SLAVE)
				|| (USLCsched_Get_Current_Frame_Activity()==SCO_MASTER))
			{
				_LSLCacc_Handle_Erroneous_SCO_Packet(p_dev_link);
			}
		}
#endif

        DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
        LSLCass_Record_Rx_HEC_Event_Channel(p_dev_link);
        BTtmt_Record_Rx_Packet_Error();
        LSLCstat_Inc_HEC_Error_Cntr();
        status = RX_HEC_ERROR;
        packetType = INVALIDpkt;
    }

    DL_Set_Rx_Packet_Type(p_dev_link, packetType);
    DL_Set_Rx_Status(p_dev_link, status);

#if 0 // GF 7 Jan 2011

    if (HW_get_slave()==1)
    {        
        /*
         * This block is responsible for recording this device's
         *      current receive bt clock
         *      current intra-slot offset 
         *      current bt clock offset from native clock
         */
        
        /* last frequency clock is current receive bt clock */
        t_clock bt_clock = LSLCfreq_Get_Last_Frequency_Clock() & 0x0ffffffc;                
        
        /* intraslot transitioning in hardware so avoid race on read */
        u_int32 new_intraslot = HW_Get_Intraslot_Avoid_Race();
        
        /* use new intraslot to determine if native will transition in rx slot */
        t_clock current_frame_nt_clk = HW_Get_Native_Clk_Avoid_Race();
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
        if(new_intraslot >= 625 && !(current_frame_nt_clk&2))
        { /* native already transitioned => adjust back */
            current_frame_nt_clk -= 4;
        }
#endif
        
        DL_Set_Piconet_Clock_Last_Access(p_dev_link, bt_clock);        
        DL_Set_Local_Slot_Offset(p_dev_link, (u_int16)new_intraslot);        
        {
            t_clock clk_offset_adjusted;

#if 1 //(PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
            clk_offset_adjusted = BTtimer_Clock_Difference((current_frame_nt_clk & 0x0ffffffc),bt_clock);
#else
            clk_offset_adjusted = (bt_clock - (current_frame_nt_clk & 0x0ffffffc));
#endif
            DL_Set_Clock_Offset(p_dev_link, clk_offset_adjusted);
        }
    }
#endif
}

/************************************************************************
 * LSLCacc_Extract_FHS_Payload
 *
 * Extract the FHS packet payload and deliver to fhs buffer
 *************************************************************************/
void LSLCacc_Extract_FHS_Payload(t_FHSpacket *p_fhs_packet)
{
    hw_memcpy(p_fhs_packet, (u_int8*) HW_get_rx_acl_buf_addr(),
                FHS_PACKET_LENGTH);                   
}
#endif//BLUETOOTH_MODE_LE_ONLY
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
/************************************************************************
 * LSLCacc_Extract_EIR_Payload
 *
 * Extract the EIR packet payload and deliver to eir buffer
 *************************************************************************/
void LSLCacc_Extract_EIR_Payload(u_int8 *p_eir_packet, u_int8 rx_length)
{
    hw_memcpy(p_eir_packet, (u_int8*) HW_get_rx_acl_buf_addr(),
                rx_length /* EIR_PACKET_LENGTH */);
	/*
	 * If a 32 memcpy is used (& this is likely to be the case) and the rx_length is not a 
	 * multiple of 4 bytes then need to ensure the "un-used" bytes in the last 32bit word are
	 * set to zero.  To cover all scenarios we shall set the 3 bytes following the rx_length to
	 * 0 (ie there may be 1, 2, or 3 irrelevant bytes following the last relevant byte).
	 * 
	 * Example: rx_len = 17 bytes.
	 *   A hw_memcpy32 will copy 5 u_int32 locations (the 17th byte is located in the 5th u_int32 location).
	 *   The 5th byte will only contain 1 relevent byte
	 *   with the remainign 3 bytes containing whatever happened to be in the RX_ACL_BUFFERS at the 
	 *   time.  For the purposes of the EIR need to ensure that these 3 unused bytes are set to "0".
	 */
	p_eir_packet[rx_length]=0x00;
	p_eir_packet[rx_length+1]=0x00;
	p_eir_packet[rx_length+2]=0x00;
}
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
 /************************************************************************
 * _LSLCacc_Extract_ACL_Payload
 *
 * Extract the ACL packet payload and deliver to queues.
 *************************************************************************/
void _LSLCacc_Extract_ACL_Payload(t_devicelink *p_dev_link, t_packet packetType)
{
    t_deviceIndex device_index = DL_Get_Device_Index(p_dev_link);
	
    t_length     rx_length;
    t_q_descr    *rx_qd = 0;
    u_int8       msg_type;
    u_int8       q_type;
	
    boolean      rx_queue_full= FALSE;
	
    mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
#endif
	
    mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);
	
#if ((LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED==1) || (LC_DEFERRED_LOOPBACK_PAYLOAD_EXTRACTION_SUPPORTED==1))
	if ((BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) &&
		(LSLCacc_Is_Deferred_Extraction_Pending_()))
	{
		// In Testmode Loopback DV packets are handled completely in the 
		// SCO queues.
		rx_length = G_rx_loopback_len;
		msg_type = G_msg_type;
	}
	else
#endif
	{
		rx_length = mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_LEN);
		HW_toggle_rx_buf();
		msg_type = (u_int8)mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH);
	}
	
    if (msg_type == LMP_msg) 
    {
        q_type = LMP_IN_Q;
		
        /*
		* Load LMP opcode from tabasco to support both endian-ness
		*/
        LSLCacc_last_rx_LMP_msg = 
            (u_int8)((*(u_int32 *)HW_get_rx_acl_buf_addr())>>1);
    }
    else
    {
        q_type = L2CAP_IN_Q;
    }
	
#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
    /*
	* Check if Testmode loopback: DVs ACL/SCO data are handled in SCO extraction
	* If active loopback, and not an LMP or DV, then immediately loopback/exit
	*/
	
	if ( (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK) && (msg_type != LMP_msg) )
	{ 
#if (LC_DEFERRED_LOOPBACK_PAYLOAD_EXTRACTION_SUPPORTED==1) 
		if (LSLCacc_Is_Deferred_Extraction_Pending_() == TRUE)
		{
			q_type = L2CAP_OUT_Q;
			
			if(rx_length > 0)
			{
				
				if(mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR))
				{
					
					// As there can only be one entry on the Queue in loopback 
					// I need to first check if there is a pre-existing entry
					// on the Queue. If there is it has to be removed.
					// GF 11 dec
					// GF 11 dec end change
					rx_qd = BTq_Enqueue(L2CAP_OUT_Q, device_index, rx_length);
					rx_queue_full = !rx_qd;
				} 
			}
			
			
			/*
			* If message type is L2CAP Then handle both baseband and L2CAP flow control
			*/
			if (msg_type != LMP_msg && 
			/*
			* B1518, ignore flow if a broadcast L2CAP message
			*        This ignores the case where a Master defaults to Stop
			*/
			mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR) )
			{
			/*
			* Baseband Flow control STOP only appropriate if no queue for an L2CAP message
				*/
				DL_Set_Local_Rxbuf_Full(p_dev_link, rx_queue_full);
				/*
				* The Peer L2CAP Flow Status may need to be checked before sending next
				* Tx packet (rf. _LSLCacc_Prepare_Tx_Complete_ARQN_If_Same_Device() )
				*/
				DL_Set_Local_Tx_L2CAP_Flow(p_dev_link,
					(t_flow) mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_P_FLOW) );
			}
			
			
			if(rx_qd)
			{
			/*
			*  Update previous ARQN/FLOW in prepared outgoing packet
			*  if its on this device_link and then send pending tx packet.
			*  This defines the critical receive window processing
				*/
				_LSLCacc_Prepare_Tx_Complete_If_Same_Device(p_dev_link);

				LSLCacc_Enable_Transmitter();
				
				/*
				* Place entry on queue by filling in the receive descriptor and commit.
				*/
				rx_qd->message_type = msg_type;
				
				{
					u_int8 *rx_acl_buf_addr = (u_int8*)(HW_get_rx_acl_buf_addr());
					
					/*
					* When DV packet received with SCO to be delivered to HCI,
					* it is offset by either 20 bytes or 12 bytes to allow 
					* for SCO payload. The same ACL component offset also applies 
					* when SCO CFG inactive
					*/
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
					if (packetType==DV && 
						( (LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(p_dev_link->active_sco_fifo) ||
						(SCO_CFG_INACTIVE == LSLCacc_Get_SCO_CFG(p_dev_link->active_sco_fifo)))))
					{
#if (PRH_BS_CFG_TABASCO_VERSION < PRH_BS_HW_TABASCO_VERSION_T1_0_0)
						rx_acl_buf_addr += 20;
#else
						if(LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(p_dev_link->active_sco_fifo))
						{
							rx_acl_buf_addr += 12;
						}
						else
						{
							rx_acl_buf_addr += 20;
						}
#endif
					}
#endif
					/*
					* ACL data is otherwise always aligned with start of rx acl buffer
					*/
#if (PRH_BS_CFG_SYS_ACL_BUFFER_ALIGNMENT==16) && (PRH_BS_DEV_BIG_ENDIAN_AND_DIRECT_BASEBAND_MOVE==0)
					/*
					* LMP_msg queued using variable length chunk system, so no buffer alignment. Must
					* not use hw_memcpy128 in this case, aas could cause memory corruption of LMP_IN_Q!!
					*/
					if (LMP_msg == msg_type)
					{
						hw_memcpy32(rx_qd->data, (void*)rx_acl_buf_addr, rx_length);
					}
					else
#endif
					{
						_LSLCacc_memcpy_ACL(rx_qd->data, (void*)rx_acl_buf_addr, rx_length);
					}
				}
				
				/*
				* This line (below) will only be executed if something was
				* put on the queue, i.e. rx_qd != 0x0
				*/
				BTq_Commit(q_type, device_index);
				
			} /* else if(rx_qd) */
			
		 }
		 else
#endif
			 if (packetType != DV)
			 {
				 _LSLCacc_Loopback_Testmode_Payload(p_dev_link, packetType, rx_length);
			 }
			 return;
     }
#endif
	 
#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
	 /*
     * If in Testmode Tester In Standalone Rx Test then Ignore payload
     */
     else if (BTtst_Get_DUT_Mode() == DUT_STANDALONE_RXTEST)
     {
		 return;
     }
     else if ( (BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER) && (msg_type != LMP_msg) )
     {
	 /*
	 * If Test Mode Tester Enabled Then Verify the Rx Data Is Ok
	 *
	 * Callback to function to perform payload verification
         */
		 BTtmt_Verify_Tester_After_Rx(DL_Get_Device_Index(p_dev_link),
			 (const u_int8 *)HW_get_rx_acl_buf_addr(), 
			 rx_length, ( t_LCHmessage) LCH_start);
		 return;
     }
#endif
	 
#if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)
	 /*
     * If Pump Verification Enabled Then Verify the Rx Data Is Ok
     *
     * Internal Data Pump:  Check that packet is ok after before being
     * extracted from rx register by recalculating the software CRC
     */
	 BTpump_Verify_Data_Pump_After_Rx(DL_Get_Device_Index(p_dev_link),
		 (const u_int8 *)HW_get_rx_acl_buf_addr(), 
		 rx_length, ( t_LCHmessage) msg_type);
#endif
	 
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
		 /*
		 * If in extended inquiry response frame, the packet is handled by
		 * the USLCinq SM for event generation, and so is not enqueued here.
     */
	 if(USLCinq_Is_Extended_Inquiry_Response_Frame())
	 {
		 return;
	 }
#endif
	 
	 /*
     * If it's not a zero length packet, put it on the queue
     */
	 if(rx_length > 0)
	 {
	 /*
	 * Check if it was a broadcast packet and enqueue on device's queue
	 * If device queue space then put on queue else ignore endif
         */
		 
		 if(!(mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR)))
		 {
			 rx_qd = BTq_Enqueue(q_type, device_index, rx_length); 
			 
			 if(rx_qd)
			 {
			 /*
			 * For the moment, set broadcast flags to received
			 * active broadcast (0x1)
                 */
				 
				 rx_qd->broadcast_flags = 0x1;
				 /* 
                 * NOTE: When park mode is implemented, the rx_qd->broadcast_flags
                 * should be set to either 0x1 (received in active or piconet broadcast)
                 * or 0x2 (received in park mode)
                 */
			 }
			 /*
             * The buffer's for broadcast should never be indicated full
             */
			 rx_queue_full = 0;                
		 }
		 else
		 {
			 rx_qd = BTq_Enqueue(q_type, device_index, rx_length);
			 rx_queue_full = !rx_qd;
		 } 
	 }
	 else
	 {
	 /*
	 * 0 length packet: don't put on queue/ensure flow is GO
         */
		 rx_qd = 0x0;  
		 rx_queue_full = 0x0;   
	 }
	 
	 /*
     * If message type is L2CAP Then handle both baseband and L2CAP flow control
     */
	 if (msg_type != LMP_msg && 
	 /*
	 * B1518, ignore flow if a broadcast L2CAP message
	 *        This ignores the case where a Master defaults to Stop
	 */
	 mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_AM_ADDR) )
	 {
	 /*
	 * Baseband Flow control STOP only appropriate if no queue for an L2CAP message
         */
		 DL_Set_Local_Rxbuf_Full(p_dev_link, rx_queue_full);
		 /*
         * The Peer L2CAP Flow Status may need to be checked before sending next
         * Tx packet (rf. _LSLCacc_Prepare_Tx_Complete_ARQN_If_Same_Device() )
         */
		 DL_Set_Local_Tx_L2CAP_Flow(p_dev_link,
			 (t_flow) mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_P_FLOW) );
	 }
	 
	 if (rx_queue_full)
	 {
	 /*
	 * Queue is full therefore override CRC packet (non-broadcast only)
		 */
		 DL_Toggle_Rx_LT_Expected_Seqn(p_dev_link);
		 DL_Set_Rx_LT_Previous_Arqn(p_dev_link, NAK);
	 }
	 else if(rx_qd)
	 {
	 /*
	 *  Update previous ARQN/FLOW in prepared outgoing packet
	 *  if its on this device_link and then send pending tx packet.
	 *  This defines the critical receive window processing
         */
		 _LSLCacc_Prepare_Tx_Complete_If_Same_Device(p_dev_link);
		 LSLCacc_Enable_Transmitter();
		 
		 /*
         * Place entry on queue by filling in the receive descriptor and commit.
         */
		 rx_qd->message_type = msg_type;
		 
		 {
			 u_int8 *rx_acl_buf_addr = (u_int8*)(HW_get_rx_acl_buf_addr());
			 
			 /*
			 * When DV packet received with SCO to be delivered to HCI,
			 * it is offset by either 20 bytes or 12 bytes to allow 
			 * for SCO payload. The same ACL component offset also applies 
			 * when SCO CFG inactive
			 */
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
			 if (packetType==DV && 
				 ( (LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(p_dev_link->active_sco_fifo) ||
				 (SCO_CFG_INACTIVE == LSLCacc_Get_SCO_CFG(p_dev_link->active_sco_fifo)))))
			 {
#if (PRH_BS_CFG_TABASCO_VERSION < PRH_BS_HW_TABASCO_VERSION_T1_0_0)
				 rx_acl_buf_addr += 20;
#else
				 if(LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(p_dev_link->active_sco_fifo))
				 {
					 rx_acl_buf_addr += 12;
				 }
				 else
				 {
					 rx_acl_buf_addr += 20;
				 }
#endif
			 }
#endif
			 /*
			 * ACL data is otherwise always aligned with start of rx acl buffer
			 */
#if (PRH_BS_CFG_SYS_ACL_BUFFER_ALIGNMENT==16) && (PRH_BS_DEV_BIG_ENDIAN_AND_DIRECT_BASEBAND_MOVE==0)
			 /*
			 * LMP_msg queued using variable length chunk system, so no buffer alignment. Must
			 * not use hw_memcpy128 in this case, aas could cause memory corruption of LMP_IN_Q!!
			 */
			 if (LMP_msg == msg_type)
			 {
				 hw_memcpy32(rx_qd->data, (void*)rx_acl_buf_addr, rx_length);
			 }
			 else
#endif
            {
                _LSLCacc_memcpy_ACL(rx_qd->data, (void*)rx_acl_buf_addr, rx_length);
            }
        }

        /*
         * This line (below) will only be executed if something was
         * put on the queue, i.e. rx_qd != 0x0
         */
        BTq_Commit(q_type, device_index);

    } /* else if(rx_qd) */
}

/************************************************************************
 * _LSLCacc_Extract_SCO_Payload
 *
 * Extract the SCO packet payload and deliver to queues.
 *************************************************************************/

void _LSLCacc_Extract_SCO_Payload(t_devicelink *p_dev_link, t_packet packetType)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_length pkt_contents_length;
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    t_sco_fifo hw_active_sco_fifo = p_dev_link->active_sco_fifo;
    t_q_descr *p_qD = 0;
	t_sco_info *p_sco_link;
	boolean extract_payload_contents = TRUE;
#endif

#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
            u_int8 sco_fifo_rep;
#endif

#ifdef SYS_DEBUG_STOP_SENDING_SCO_TO_HOST
      return;
#endif
#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
    /*
     * Check if Testmode loopback:
     */
    if (BTtst_Get_DUT_Mode() == DUT_ACTIVE_LOOPBACK)
    {
        /*
         *  DVs ACL/SCO data are handled here
         * Just loopback the maximum SCO packet length
         */
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
#if PRH_BS_CFG_TABASCO_VERSION < PRH_BS_HW_TABASCO_VERSION_T1_0_0
        t_length rx_length = BTtst_Get_Loopback_Packet_Length() * 2; /* 30 * 2 for interleaving */
#else
        t_length rx_length = BTtst_Get_Loopback_Packet_Length();
		if (packetType == DV)
			rx_length = 10;
#endif
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
        if (is_eSCO_Packet(packetType))
        {
            /*
             * eSCO data is handled here.
             * Loopback the received length.
             */
            rx_length = DL_Get_eSCO_Rx_Length(p_dev_link);
        }
#endif
#if ((LC_DEFERRED_PAYLOAD_EXTRACTION_SUPPORTED !=1) && (LC_DEFERRED_LOOPBACK_PAYLOAD_EXTRACTION_SUPPORTED !=1))

        _LSLCacc_Loopback_Testmode_Payload(p_dev_link, packetType, rx_length);
#else

#ifndef SYS_DEBUG_STOP_SENDING_SCO_TO_HOST
#if 1 // 13 Jan Added by GF to support large EDR eSCO packets
      // Was not turned on during conformance in March so left off
	  // until completely verified.
		if (rx_length > PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH)
		{

		   u_int8* p_ACL_Buffer = (void *)HW_get_rx_sco_buf_addr();

			while ( rx_length > PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH)
			{
				p_qD = BTq_Enqueue(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/, PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH);
				if (p_qD && p_qD->data)
				{
					_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data,
							(void *)p_ACL_Buffer, PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH);

					BTq_Commit(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/);
				}
				rx_length -= PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH;
				p_ACL_Buffer += PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH;

			}

			// The Last Segment
			p_qD = BTq_Enqueue(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/, rx_length);
			if (p_qD && p_qD->data)
			{
				_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data,
					(void *)p_ACL_Buffer, rx_length);

				BTq_Commit(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/);
			}

		}
		else
		{
			p_qD = BTq_Enqueue(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/, rx_length);
			if (p_qD && p_qD->data)
			{
				_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data,
					(void *)HW_get_rx_sco_buf_addr(), rx_length);
				
				BTq_Commit(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/);
			}
		}

#else

		p_qD = BTq_Enqueue(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/, rx_length);
		if (p_qD && p_qD->data)
		{
			_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data,
				(void *)HW_get_rx_sco_buf_addr(), rx_length);

			BTq_Commit(SCO_OUT_Q, 0x00 /*hw_active_sco_fifo*/);
		}
#endif

#endif

#endif
        return;
     }
#endif /*(PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)*/

#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
    /*
     * If in Testmode Tester In Standalone Rx Test then Ignore payload
     */
     else if (BTtst_Get_DUT_Mode() == DUT_STANDALONE_RXTEST)
     {
          return;
     }
     else if ( (BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER))
     {
        /*
         * If Test Mode Tester Enabled Then Verify the Rx Data Is Ok
         *
         * Callback to function to perform payload verification
         */
        t_length     rx_length;

        if (packetType==DV)
        {
            rx_length = MAX_DVSCO_USER_PDU; /* Below gets DV ACL length! */
        }
        else
        {
            rx_length = SYSconst_Get_Packet_Length(packetType);  
        }

        BTtmt_Verify_Tester_After_Rx(DL_Get_Device_Index(p_dev_link),
            (const u_int8 *)HW_get_rx_sco_buf_addr(), 
            rx_length, LCH_null);
        return;
     }
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
    if (is_eSCO_Packet(packetType))
    {
        hw_active_sco_fifo = p_dev_link->active_sco_fifo;
        /*
         * #2233   
         */ 
        pkt_contents_length = DL_Get_eSCO_Rx_Length(p_dev_link);

        /*
         * If a valid eSCO payload has alrady been received in the 
         * eSCO window, then ignore this payload.
         *
         * Note: At the start of each eSCO window, the rx_esco_pending
         * is reset. This ensures that only 1 valid eSCO payload is stored
         * on the queues for each eSCO window. 
         */
        if (DL_Get_Rx_eSCO_Pending(p_dev_link))
        {
            /*
             * A valid eSCO paylaod has now been received in the eSCO window.
             * Therefore, the device is not expecting subseqent eSCO payloads
             * in this window. Any suquent eSCO payloads in this eSCO window
             * will be ignored.
             */
            DL_Set_Rx_eSCO_Pending(p_dev_link, FALSE);    
			
			// If eSCO packet - I need to Read the Rx State.
			{
				t_sco_info *p_sco_link;
				
				p_sco_link = LMscoctr_Find_SCO_By_SCO_Index(p_dev_link->active_sco_fifo);
				
				if (p_sco_link)
					LMscoctr_Read_End_Rx_Transcode_State(p_sco_link);
			}
        } 
        else
        {
            /*
             * A valid eSCO paylaod has already been received in the eSCO window.
             * Therefore, the device is not expecting eSCO payloads
             * in this window. This eSCO payload in this eSCO window will be ignored.
             */
            extract_payload_contents = FALSE;
        }
    }
    else
#endif
    {
        /*
         * SCO packet.
         */
        if (packetType==DV)
        {
            pkt_contents_length = 10;  /* Below gets DV ACL length!      */
        }
        else
        {
             pkt_contents_length = SYSconst_Get_Packet_Length(packetType);  
        }
    }

    /*
     * If converting to a sample size > 8 AND the air mode is
     * not transparent, the hardware will supply
     * twice as many bits to the host controller for delivery
     * to the host
     */
    if(!LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(hw_active_sco_fifo) )
    {
        pkt_contents_length <<=1;
    }

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
   /*
    * #2775
	* Previously there was no checking of the p_sco_link_state, i.e.
    * SCO packet extraction was performed for all states.
    *
    * Now do not want to extract a packet if the device is in the 
    * SCO_MASTER_DEACTIVATION_PENDING state.
    */
	p_sco_link = LMscoctr_Find_SCO_By_SCO_Index(hw_active_sco_fifo);
    
    if(p_sco_link && ((p_sco_link->state == SCO_MASTER_DEACTIVATION_PENDING) ||
		(p_sco_link->state==SCO_UNUSED)|| (p_sco_link->state==SCO_ACTIVATION_PENDING) ||
		(p_sco_link->state==SCO_IDLE)))
	    extract_payload_contents = FALSE;


    if (extract_payload_contents && 
            LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(hw_active_sco_fifo))
    {
#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
        if (1== LC_Get_SCO_Repeater_Bit())
        {
            if (LMscoctr_Get_Number_SCOs_Active() == 2)
            {
                sco_fifo_rep = ( (hw_active_sco_fifo==0) ? 1 : 0) ;

                /*
                 * Enqueue directly onto the SCO fifo output queue of the other SCO link.
                 * The queue is the one associated with the other SCO link. 
                 */
                p_qD = BTq_Enqueue(SCO_OUT_Q, sco_fifo_rep, pkt_contents_length);
            }
            else
            {
                /*
                 * Discard all SCO data until the 2nd SCO link is established.
                 */
                p_qD = 0;
            }
        }    
        else
#endif
            /*
             * Enqueue directly onto the SCO fifo input queue [0..2] 
             * The queue is the one associated with the current h/w sco_fifo
             * and may not be the same as p_dev_link->active_sco_fifo due
             * to next frame lookahead.
             */
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
		p_qD = DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link);

		if ((!is_eSCO_Packet(packetType)) || (p_qD == 0))
#endif
		{
#if (PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1)
			if (p_dev_link->sco_commit_pending == FALSE)
#endif
			{

				if (pkt_contents_length > PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH)
				{
					
					u_int8* p_ACL_Buffer = (void *)HW_get_rx_sco_buf_addr();
					
					while ( pkt_contents_length > PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH)
					{
						p_qD = BTq_Enqueue(SCO_IN_Q, hw_active_sco_fifo, PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH);
						if (p_qD && p_qD->data)
						{
							_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data,
								(void *)p_ACL_Buffer, PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH);
							
							BTq_Commit(SCO_IN_Q, hw_active_sco_fifo );
						}
						pkt_contents_length -= PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
						p_ACL_Buffer += PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
						
					}
					
					// The Last Segment
					p_qD = BTq_Enqueue(SCO_IN_Q, hw_active_sco_fifo, pkt_contents_length);
					if (p_qD && p_qD->data)
					{
						_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data,
							(void *)p_ACL_Buffer, pkt_contents_length);
						
						BTq_Commit(SCO_IN_Q, hw_active_sco_fifo);
						p_dev_link->sco_commit_pending = FALSE;
						p_qD = 0;
					}
					
				}
				else
				{				
					p_qD = BTq_Enqueue(SCO_IN_Q, hw_active_sco_fifo, pkt_contents_length);
				}

#if (PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1)
				if (p_qD)
					p_dev_link->sco_commit_pending = TRUE;
#endif
			}


		}

		if (p_qD)
		{
			u_int8* rx_data_ptr = (u_int8*)HW_get_rx_sco_buf_addr();

			DL_Set_Rx_SYNC_Erroneous_Data_Status(p_dev_link,SYNC_DATA_GOOD);
			
#if 1 // TK 15 MAY 2012: HANDLE MULTIPLE ERRONEOUS BUFFERS PER PACKET
			if (p_qD->next)
			{
				while (p_qD)
				{
					if (p_qD->next)
					{
						_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data, (void *)rx_data_ptr, PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH);
						rx_data_ptr += PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
						pkt_contents_length -= PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
					}
					else
						_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data, (void *)rx_data_ptr, pkt_contents_length);

					p_qD->message_type = SYNC_DATA_GOOD;
					p_qD = p_qD->next;
				}

				p_qD = DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link); // restore p_qD from above iterations
			}
			else
#endif
			{
				p_qD->message_type = SYNC_DATA_GOOD;
				_LSLCacc_memcpy_SCO_from_ACL((void *)p_qD->data, (void *)rx_data_ptr, pkt_contents_length);
			}

#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
			if ( (1== LC_Get_SCO_Repeater_Bit()) && (LMscoctr_Get_Number_SYN_Connections() == 2) )
			{
				BTq_Commit(SCO_OUT_Q, sco_fifo_rep);
			}
			else
#endif
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
			{
			u_int32 current_clk;
			boolean end_eSCO_window;

			current_clk = (LC_Get_Piconet_Clock_Ex((p_sco_link->device_index),TRUE) + 2) & 0x0FFFFFFE;

			/* Determine if end of eSCO window. If it is then commit.
			 */
				end_eSCO_window = _is_end_eSCO_window(p_sco_link, p_dev_link,current_clk);
			    if (end_eSCO_window)
				    p_dev_link->esco_end_retransmission_window_timer = p_sco_link->esco_end_retransmission_window_timer;


				if (p_dev_link->sco_commit_pending && (end_eSCO_window ||
					(DL_Get_Rx_SYNC_Erroneous_Data_Status(p_dev_link) == SYNC_DATA_GOOD)))
				{
					BTq_Commit(SCO_IN_Q, hw_active_sco_fifo);
#if 1 // TK 15 MAY 2012: HANDLE MULTIPLE ERRONEOUS BUFFERS PER PACKET
					while (p_qD && p_qD->next)
					{
						BTQueues[SCO_IN_Q+hw_active_sco_fifo].num_entries++; // un-protect
						p_qD = p_qD->next;
					}
#endif
					DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link,0);
					DL_Set_Rx_SYNC_Erroneous_Data_Status(p_dev_link,0xFF);
					p_dev_link->sco_commit_pending = FALSE;
				}
			}

#else
			{
				BTq_Commit(SCO_IN_Q, hw_active_sco_fifo);
			}
#endif
		} /* p_qD */
	} /* SCO_CFG_VIA_ACL */
#endif
#endif//BLUETOOTH_MODE_LE_ONLY

}
#endif//BLUETOOTH_MODE_LE_ONLY
#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
/************************************************************************
 * _LSLCacc_Loopback_Testmode_Payload
 *
 * Extract the Testmode packet payload and loopback if necessary.
 * The full packet will have been processed which means ARQN is valid.
 *
 * p_dev_link          Pointer to the active LC device link
 * packet_type         The packet type received
 * packet_length       The length of the ACL/SCO buffer to loopback
 *
 *                     The actual length will be extracted from
 *                     rx_status register.
 *
 * Scenarios with D**  loopback (including DM1):
 *          Tester(M)              IUT(S)
 * 1. Normal    |-----D**_LC2AP---->|
 *              |<----D**_L2CAP-----|
 *              |                   |
 * 2. Tester    |-----DM1_LMP------>|
 *    sends LMP |<---------NULL-----|
 *              |                   |
 * 3. IUT       |-----D**_LC2AP---->| <- Enqueue LMP message
 *    sends LMP |<----DM1_LMP-------| 
 *              |                   |
 *************************************************************************/
static void _LSLCacc_Loopback_Testmode_Payload(t_devicelink *p_dev_link, 
    t_packet packet_type, t_length packet_length)
{  
	u_int8 msg_type;
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;
    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);

    mHWreg_Assign_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);

    /*
     * First check if pending LMP message which will have been setup
     * on packet header.  Send LMP if ready to transmit.  
     * Detect LMP (Awaiting ACK AND L_CH will be LMP)
     */
    if ( (DL_Get_Tx_Ack_Pending(p_dev_link)==FALSE) ||
         (mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_L_CH) != LMP_msg) )
    {
        /*
         * Loopback received packet
         */
        mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
		mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
		mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
#endif
        mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);        

        msg_type = mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_L_CH);
        DL_Use_Local_Tx_Descriptor(p_dev_link);
        DL_Set_Tx_Packet_Type(p_dev_link, packet_type);
        DL_Set_Tx_Length(p_dev_link, packet_length);

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        if(is_eSCO_Packet(packet_type))
        {
            DL_Set_Active_Tx_LT(p_dev_link, ESCO_LT);
            DL_Set_eSCO_LT_Address(p_dev_link, ESCO_LT, DL_Get_Am_Addr(p_dev_link));
            DL_Set_eSCO_Tx_Length(p_dev_link, (u_int8)packet_length);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
            LC_Set_PTT_ESCO(p_dev_link->device_index, (boolean)is_EDR_ESCO_packet(packet_type));
#endif
        }
#endif

        /* 
         * L2CAP Flows must always be 1 to ensure no over-rides
         */
        DL_Set_Local_Tx_L2CAP_Flow(p_dev_link, GO);
        DL_Set_Local_Rx_L2CAP_Flow(p_dev_link, GO);

        DL_Set_Tx_Message_Type(p_dev_link, (t_LCHmessage) msg_type);

        /* 
         * All packet types are copied here for both little/big endian
         *
         * Direct copy of payload and length, and setup of cache register
         * Build a NULL, and change to appropriate packet type 
         *
         * It is much easier to modify a NULL than to try to use
         * LSLCpkt_Generate_SCO()/LSLCpkt_Generate_ACL() which deal with 
         * endianess
         */
        DL_Set_Tx_Qpdu(p_dev_link, (u_int8*) HW_get_rx_acl_buf_addr() );
        LSLCpkt_Generate_NULL(p_dev_link);
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
		        /*
				 * If an EDR packet is to be transmitted and EPC is supported in the peer then
				 * need to override the tx_power_level set in _LSLCpkt_Prepare_Piconet_Channel_Access()
				 */
				if (DL_Is_EPC_Enabled(p_dev_link) && is_EDR_ACL_packet(packet_type))
					HWradio_Set_Tx_Power_Level(DL_Get_EPC_Power_Level(p_dev_link, packet_type)); 
#endif

        hw_memcpy32_transparent((void*) HW_get_tx_acl_buf_addr(), 
            (void*)DL_Get_Tx_Qpdu(p_dev_link), packet_length);

        /*
         * B972 ensure that counters are correct
         */
        LSLCstat_Inc_Tx_Packet_Cntr(packet_type);
        LSLCstat_Dec_Tx_Packet_Cntr(NULLpkt);

        /*
         * Override length, type and tx_buf fields for packet
         */
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_LEN, 
				 mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_LEN) );
        if (packet_type == EV3)
        {
            mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE, HV3 /* ie 0x7 on BB */);
        }
        else
        {
            mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE, packet_type);
        }
        mHWreg_Toggle_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_BUF);

        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_L_CH, msg_type);
        mHWreg_Assign_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_P_FLOW, 1);

       /*
        * For all non CRC packets, must set the arqn = ACK
        */
        if ( DL_Get_Tx_LT_Previous_Arqn(p_dev_link) || 
             !is_ACL_CRC_Packet(packet_type) )
        {
             mHWreg_Assert_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_ARQN);
        }
#if 1 // GF 12 Mar 2011 - test mode changes
	  // Below seemed to fix problem with ARQN on loopback of CRC Errored
		// packets. Changed for issues with AGILENT test
		else
		{
             mHWreg_Clear_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_ARQN);
		}
#endif

        
#if (PRH_BS_DEV_ADOPT_ERRATUM_E2088==1)        
        /*
         * Ensure that packet ACK is ignored and Tx SEQN follows Rx SEQN.
         */
        mHWreg_Assign_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_SEQN,
            mHWreg_If_Cache_Bit_is_1(JAL_RX_STATUS_GP_REG, JAL_RX_SEQN) );
        DL_Set_Tx_LT_Tx_Ack_Pending(p_dev_link, FALSE);
#else
        mHWreg_Assign_Cache_Bit(JAL_TX_CTRL_REG, JAL_TX_SEQN,
            DL_Get_Tx_LT_Seqn(p_dev_link) ); 

#if 1 // GF 12 March - Removed for issues with Agilent Tester
     //   DL_Set_Tx_LT_Tx_Ack_Pending(p_dev_link, (u_int8)is_ACL_CRC_Packet(packet_type));
#endif

#endif

        mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_reg_shadow->tx_ctrl);
    }
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	_LSLCacc_Set_PTT(p_dev_link);
#endif
    LSLCacc_Enable_Transmitter();
}
#endif /*PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED*/

/************************************************************************
 * _LSLCacc_Extract_Payload
 * Extract the packet payload and deliver to queues.
 *
 * Precondition:        Packet is a valid packet
 * Postcondition:       Payload will be on queues if queue space
 *
 *
 * ID, NULL, POLL       No Payload
 * FHS                  Payload, build Rx Descriptor, no move
 * DM*, DH*, DV, AUX1   Payload, build Rx Descriptor and check for buffer avail.
 * HV*                  Payload if SCO via ACL registers
 *
 *************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
static t_TXRXstatus
   _LSLCacc_Extract_Payload(t_devicelink *p_dev_link, t_packet packetType)
{
    t_TXRXstatus status=RX_OK;


    if (packetType==FHSpkt)
    {
        /*
         * FHS length is fixed and FHS buffer is extracted on request.
         */
        HW_toggle_rx_buf();
        /*
         * Clock Absolute to Clock Offset completed in USLC procedure
         */
    }

    /*
     * If an ACL packet Then Extract immediately to Queue Endif
     */
    else if (mLSLCacc_Is_ACL_Packet(packetType))
    {
        _LSLCacc_Extract_ACL_Payload(p_dev_link, packetType);
    }  


#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1))
    /*
     * DVs ensure that SCOs are not mutually exclusive with ACL packets
     */

#if ( 0==PRH_BS_DBG_SOURCE_SINK_SYN_TEST_DATA)
    if (is_SYN_Packet(packetType))
    {
       _LSLCacc_Extract_SCO_Payload(p_dev_link, packetType);
    }
#else
    if ((is_SYN_Packet(packetType)) && (BTtst_Get_DUT_Mode() > DUT_ACTIVE ))
    {
       _LSLCacc_Extract_SCO_Payload(p_dev_link, packetType);
    }
#endif

#if (PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1)
   if((packetType == NULLpkt)|| (packetType == POLLpkt))
   {
		if((USLCsched_Get_Current_Frame_Activity()==SCO_SLAVE)
			|| (USLCsched_Get_Current_Frame_Activity()==SCO_MASTER))
		{
			_LSLCacc_Handle_Erroneous_SCO_Packet(p_dev_link);
		}
   }
#endif

#endif /*(PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) */

    return status;
}

/************************************************************************
 * LSLCacc_Initialise 
 *
 * Need to initialise any module static data
 ************************************************************************/
void LSLCacc_Initialise(void)
{
    u_int32 i;

	_was_previous_rx_slot_a_scan = 0;

    for (i = 0; i < sizeof(_ctrl_reg_shadow)/4; i++)
    {
        *((u_int32*)(&_ctrl_reg_shadow) + i ) = 0;
    }


#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    /* set all SCO fifos to inactive */
    lslc_acc_shadow_sco_cfg_ = 0;
#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
    /* Set sco_route via codec */
    lslc_acc_shadow_sco_route_ = 0; 
#endif
#endif

#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
    lslc_acc_recent_rssi_available_ = FALSE;
#endif

    LSLCacc_last_rx_LMP_msg = NO_OPERATION;

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)

#if (HW_RADIO==HWradio_ORCA_ORC2110) // edr sync errors prevalent with orca rf esp at 3 mhz 
	HW_set_edr_sync_err(5); // valid values 0..31: yet to investigated optimal value
#endif

	{
        /*
         * Ensure the edr shado reg is correctly initialised
         */

        mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
        mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
        mHWreg_Move_Cache_Register(JAL_EDR_CTRL_REG, _ctrl_reg_shadow.edr_ctrl);
#if (HW_RADIO==HWradio_ORCA_ORC2110)
        //the fastest point is at RX_EDR_DELAY  =  4’h8 which allows rci_edr2/3 signal through with 1us delay
    	//And the slowest point is at RX_EDR_DELAY  =  4’h7 where you’ll have a delay of ~ 15us.
    	edr_tx_edr_delay = 5;
        edr_rx_edr_delay = 8; // for ORCA ONLY - requires earlier assertion
#else
      	edr_tx_edr_delay = 5;
        edr_rx_edr_delay = 0;
#endif
    }
#endif
}
#endif//BLUETOOTH_MODE_LE_ONLY

/************************************************************************
 * LSLCacc_Attempt_RSSI_Read 
 *
 * Read the RSSI from the radio if supported
 ************************************************************************/
void LSLCacc_Attempt_RSSI_Read(boolean late_read_opportunity)
{
#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
    lslc_acc_recent_rssi_available_ = 
        HWradio_Update_Internal_RSSI_Cache(late_read_opportunity);
#endif
}

/************************************************************************
 * LSLCacc_Process_RSSI 
 *
 * Read and update the Device Link RSSI information if supported
 ************************************************************************/
void LSLCacc_Process_RSSI(t_devicelink *p_dev_link)
{
#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
    if (FALSE != lslc_acc_recent_rssi_available_)
    {
        if ( (SLAVE == DL_Get_Role_Peer(p_dev_link)) || 
             ( (MASTER == DL_Get_Role_Peer(p_dev_link)) && 
               (RX_HEC_ERROR != DL_Get_Rx_Status(p_dev_link)) &&
               (RX_AMADDR_ERROR != DL_Get_Rx_Status(p_dev_link))))
        {
            s_int8 rssi = HWradio_Read_RSSI();
            DL_Update_RSSI_Average(p_dev_link, rssi);

            /*
             * Record RSSI for this channel, if supported in channel assessment
             */
            LSLCass_Record_Rx_RSSI_Channel(p_dev_link, rssi);
        }
        lslc_acc_recent_rssi_available_ = FALSE;
    }
#endif
}

/************************************************************************
 * LSLCacc_Enable_Low_Power_Mode 
 *
 * Enable low power mode in cache register
 ************************************************************************/
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
void LSLCacc_Enable_Low_Power_Mode(void)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_USE_LF);
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);
}

/************************************************************************
 * LSLCacc_Disable_Low_Power_Mode 
 *
 * Disable low power mode in cache register
 ************************************************************************/
void LSLCacc_Disable_Low_Power_Mode(void)
{
    t_ctrl_reg_shadow *p_reg_shadow = &_ctrl_reg_shadow;

    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Assign_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_USE_LF);
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, p_reg_shadow->gp1);
}
#endif

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
void _LSLCacc_Non_EDR_Pkt_Tx_Rx(void)
{
    mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Assign_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ACL, 0);
    mHWreg_Assign_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO, 0);
    mHWreg_Move_Cache_Register(JAL_EDR_CTRL_REG, _ctrl_reg_shadow.edr_ctrl);
}

void _LSLCacc_Set_PTT(const t_devicelink *p_dev_link)
{
    mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Clear_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO);
    mHWreg_Clear_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ACL);

    /*
	 * CM: 
	 * Confirmed that it is OK to set ptt_acl=1 and ptt_esco=1 in the same slot.
     * Ie dont have to figure out whether the device is setting up to receive
     * and ACL or eSCO packet.
	 */
	if (DL_Get_PTT_ACL(p_dev_link))
    {
        mHWreg_Assert_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ACL);
    }
    if (DL_Get_PTT_ESCO(p_dev_link))
    {
        mHWreg_Assert_Cache_Bit(JAL_EDR_CTRL_REG, JAL_EDR_PTT_ESCO);
    }

	mHWreg_Assign_Cache_Field(JAL_EDR_CTRL_REG, JAL_EDR_TX_EDR_DELAY, edr_tx_edr_delay);
    mHWreg_Assign_Cache_Field(JAL_EDR_CTRL_REG, JAL_EDR_RX_EDR_DELAY, edr_rx_edr_delay);
//	mHWreg_Assign_Cache_Field(JAL_EDR_CTRL_REG, JAL_EDR_SYNC_ERROR, edr_sync_error);

    mHWreg_Move_Cache_Register(JAL_EDR_CTRL_REG, _ctrl_reg_shadow.edr_ctrl);
}
#endif
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
static void _LSLCacc_Handle_Erroneous_SCO_Packet(t_devicelink* p_dev_link)
{
	t_length pkt_contents_length;
	t_sco_fifo hw_active_sco_fifo = p_dev_link->active_sco_fifo;
	t_q_descr *p_qD = 0;
	t_sco_info *p_sco_link;
	boolean eSCO_Active = FALSE;
	boolean end_eSCO_window = TRUE;
	boolean enQueuePacket = TRUE;
	u_int8 i;

	// if this is a SCO packet we need to enqueue and commit a SCO packet with all Zero payload.

	if ((LSLCacc_Is_SCO_CFG_Via_ACL_Buffer(hw_active_sco_fifo))  &&
	    (SYSconfig_Get_Erroneous_Data_Reporting()))
	{
			u_int32 current_clk;

			
		p_sco_link = LMscoctr_Find_SCO_By_SCO_Index(hw_active_sco_fifo);
        current_clk = (LC_Get_Piconet_Clock_Ex((p_sco_link->device_index),TRUE)+2) & 0x0FFFFFFE;
		// Need to ensure the end window timer is updated

		// Determine if eSCO and pkt_contents_length

		if ((p_sco_link->link_type == ESCO_LINK) && (p_sco_link->state != SCO_IDLE))
		{
			eSCO_Active = TRUE;
			pkt_contents_length = p_sco_link->rx_packet_length;

			// GF BQB Added 30 March
			if(!LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(hw_active_sco_fifo) )
			{
				pkt_contents_length <<=1;
			}
		}
		else
		{
			eSCO_Active = FALSE;
			pkt_contents_length = LMscoctr_Determine_SCO_Pkt_Length(p_sco_link->lm_sco_packet);

			// GF BQB Added 30 March
			if(!LSLCacc_Is_SCO_CFG_Eight_Bit_Input_Mode(hw_active_sco_fifo) )
			{
				pkt_contents_length <<=1;
			}
		}

		if(eSCO_Active)
		{
			// May have to play with the current clock a little (i.e add to it) to get the correct value to 
			// find end of the eSCO ReTx window.

			end_eSCO_window = _is_end_eSCO_window(p_sco_link, p_dev_link,current_clk);
			if (end_eSCO_window)
				p_dev_link->esco_end_retransmission_window_timer = p_sco_link->esco_end_retransmission_window_timer;


			// The Rx eSCO is set 
			if(DL_Get_Rx_eSCO_Pending(p_dev_link))
				enQueuePacket = TRUE;
			else
				enQueuePacket = FALSE;

		    if(p_sco_link && (p_sco_link->state == SCO_MASTER_DEACTIVATION_PENDING))
				enQueuePacket = FALSE;
		}
		else
		{
			enQueuePacket = TRUE;
			end_eSCO_window = TRUE;
		}

		if ((p_dev_link->sco_commit_pending==FALSE) && (enQueuePacket) &&
			(DL_Get_Rx_SYNC_Erroneous_Data_Status(p_dev_link) > SYNC_DATA_LOST))
		{
			// First time we got a HEC error in the (e)SCO window
			if (p_dev_link->sco_commit_pending == FALSE)
			{
				u_int16 packet_length = pkt_contents_length;
#if 1 // TK 15 MAY 2012: HANDLE MULTIPLE ERRONEOUS BUFFERS PER PACKET
				while (packet_length > PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH)
				{
				    p_qD = BTq_Enqueue(SCO_IN_Q, hw_active_sco_fifo , PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH);
					if (p_qD)
					{ 
						if (!DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link)) // first buffer
							DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link,p_qD); // set first buffer

						for(i=0;i<PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;i++)
							p_qD->data[i] = 0x00;
						
						p_qD->message_type = SYNC_DATA_LOST;

						BTq_Commit(SCO_IN_Q, hw_active_sco_fifo);
						BTQueues[SCO_IN_Q+hw_active_sco_fifo].num_entries--; // protect
					}
					packet_length -= PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH;
				}
#endif
				p_qD = BTq_Enqueue(SCO_IN_Q, hw_active_sco_fifo, packet_length);
                
				if (p_qD)
					p_dev_link->sco_commit_pending = TRUE;
			}
			if (p_qD)
			{
				p_dev_link->sco_commit_pending = TRUE;
				if (DL_Get_Rx_eSCO_Pending(p_dev_link))
				{
					if (!DL_Get_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link))
						DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link,p_qD);
					DL_Set_Rx_SYNC_Erroneous_Data_Status(p_dev_link,SYNC_DATA_LOST);
				}

				for(i=0;i<pkt_contents_length;i++)
				{
					p_qD->data[i] = 0x00;
				}

				// For the SCO queues use the message_type field to store the indicator of the errors
				// on the packet.

				p_qD->message_type = SYNC_DATA_LOST;

				// If this is an SCO, or the end of the the eSCO window then we need to 
				// commit the Queue entry and ReSet the Descriptor and Status.


			} // if qd
		} // If enqueue

		if (DL_Get_Rx_SYNC_Erroneous_Data_Status(p_dev_link) != SYNC_DATA_GOOD)
		{
			if (p_dev_link->sco_commit_pending && end_eSCO_window)
			{
				BTq_Commit(SCO_IN_Q, hw_active_sco_fifo);
#if 1 // TK 15 MAY 2012: HANDLE MULTIPLE ERRONEOUS BUFFERS PER PACKET
				while (p_qD && p_qD->next)
				{
					BTQueues[SCO_IN_Q+hw_active_sco_fifo].num_entries++; // un-protect
					p_qD = p_qD->next;
				}
#endif
				DL_Set_Rx_SYNC_Erroneous_Data_Descriptor(p_dev_link,0);
				DL_Set_Rx_SYNC_Erroneous_Data_Status(p_dev_link,0xFF);
				p_dev_link->sco_commit_pending = FALSE;

			}
		}

	}
}

static boolean _is_end_eSCO_window(t_sco_info *p_sco_link, t_devicelink* pDL, 
                                           t_clock  current_clk)
{

    u_int32 temp_clk,end_win;
    temp_clk = current_clk + (LMeScoctr_Get_Tx_Packet_Type_in_Slots(p_sco_link) << 1) +
                         (LMeScoctr_Get_Rx_Packet_Type_in_Slots(p_sco_link) << 1) ;

    end_win = pDL->esco_end_retransmission_window_timer;

	if (pDL->role_of_peer == SLAVE)
	{
		if ( BTtimer_Is_Expired_For_Time (pDL->esco_end_retransmission_window_timer, 
			current_clk + (LMeScoctr_Get_Tx_Packet_Type_in_Slots(p_sco_link) << 1) +
			(LMeScoctr_Get_Rx_Packet_Type_in_Slots(p_sco_link) << 1) ))
		{

			return TRUE;
		}
	}
	else
	{
		if ( BTtimer_Is_Expired_For_Time (p_sco_link->esco_end_retransmission_window_timer, 
			current_clk + (LMeScoctr_Get_Tx_Packet_Type_in_Slots(p_sco_link) << 1) +
			(LMeScoctr_Get_Rx_Packet_Type_in_Slots(p_sco_link) << 1) ))
		{

			return TRUE;
		}
	}
    return FALSE;
}

#endif
