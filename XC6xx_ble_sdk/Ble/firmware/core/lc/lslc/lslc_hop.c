/*************************************************************************
 * MODULE NAME:    lslc_hop.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LSLC Hop frequency generator
 * MAINTAINER:     John Nelson
 * CREATION DATE:  5 April 1999
 *
 * SOURCE CONTROL: $Id: lslc_hop.c,v 1.67 2013/06/24 12:22:48 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 1999-2004 Ceva Inc.
 *     All rights reserved.
 *
 * ISSUES:
 *    1.   Need to verify Inquiry Response
 *    2.   mod rfMode  The longest calc is for F i.e. a modulo rfMode 79/23
 *
 * NOTES TO USERS:
 *       The C optimal version may directly access the LLC interface.
 *       The X inputs are separately retained for page, page_scan and
 *       inquiry_scan which allows a single instance of each of these procedures
 *       to take place in parallel e.g. during the inquiry_scan random wait
 *       a page or page_scan can be executed.
 *
 *       Hardware HSE is supported using identical interface.
 ************************************************************************/

#include "sys_config.h"

#include "lc_types.h"
#include "lslc_hop.h"


/*
 *  Permutation structures for rfModes 79 and 23
 */
const static u_int8  permutationRF79[14] = {
      BIT0|BIT1, BIT2|BIT3, BIT1|BIT2, BIT3|BIT4, BIT0|BIT4,
      BIT1|BIT3, BIT0|BIT2, BIT3|BIT4, BIT1|BIT4, BIT0|BIT3,
      BIT2|BIT4, BIT1|BIT3, BIT0|BIT3, BIT1|BIT2  };

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
const static u_int8  permutationRF23[14] = {
      BIT0|BIT1, BIT2|BIT3, BIT0|BIT3, BIT1|BIT2, BIT0|BIT2,
      BIT1|BIT3, BIT0|BIT1, BIT2|BIT3, BIT0|BIT2, BIT1|BIT3,
      BIT0|BIT3, BIT1|BIT2, BIT0|BIT1, BIT2|BIT3  };
#endif
/*
 * ARM-based optimisation of SW HOP select engine block #3 to reduce MIPS.
 * Proved in THUMB mode on INTEGRATOR AP. Not suitable for non-ARM processors.
 */
#define LSLCHOP_USE_FAST_OPTIMISED_PERMUTATOR    1

/*
 * Private data members
 *
 * Notes on Engines and Kernels
 *
 * 1. The Kernel data structure is used to represent the A-F based on uap_lap
 * The Engine data structure represents all internal parameters for 
 * the hop selection calculation.
 *
 * 2. For V1.1 compatibility (the kernel is associated with the V1.1 engine).
 * Only one engine exists for V1.1, with one per AFH piconet for V1.2.
 */
static t_hs_kernel LSLChop_hs_kernel;
static t_hs_engine LSLChop_hs_engine;

/*
 * Helper Functions
 */
static t_freq _LSLChop_Execute_HSE_Non_Connection(
    t_hs_kernel *p_hs_kernel, t_clock clk, t_state state, u_int X);
static t_freq _LSLChop_Execute_HSE_Connection_79(
    t_hs_kernel *p_hs_kernel, t_clock clk);
static t_freq _LSLChop_Execute_HSE_Single_Connection(
   t_hs_kernel *p_hs_kernel, t_clock clk);
static t_freq _LSLChop_Execute_HSE_Reduced(
    t_hs_kernel *p_hs_kernel, t_clock clk);

#if (PRH_BS_CFG_SYS_HW_HOP_SELECTION_ENGINE_SUPPORTED==1)
static t_freq _LSLChop_Execute_HSE_Connection_Hardware(
    t_hs_kernel *p_hs_kernel, t_clock clk);
#else
/*
 * Hardware HSE is not included in build, ensure its ineffective!
 */
#define _LSLChop_Execute_HSE_Connection_Hardware _LSLChop_Execute_HSE_Reduced
#endif

/************************************************************************
 * LSLChop_Initialise
 * Establish the Frequency Selection Scheme for this device
 * Input Address[27:0] used is UAP[3:0]LAP[23:0]
 * Fixed Frequencies for Tx/Rx [required by Bluetooth Test Mode] stored
 *    but only used if SINGLE_FREQ
 *
 * Note: No frequency tables are built in the Initialise function, since
 *       it can be invoked in interrupt space in testmode due to 
 *       test scenario change
 *
 ************************************************************************/
void LSLChop_Initialise(t_rfSelection selection,
               t_freq txMasterFixedFreq, t_freq rxMasterFixedFreq )
{
    t_hs_engine *p_hs_kernel = &LSLChop_hs_engine;

    p_hs_kernel->rfSelection = selection;
    p_hs_kernel->txMasterHopFixedFreq = txMasterFixedFreq;
    p_hs_kernel->rxMasterHopFixedFreq = rxMasterFixedFreq;
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    if (selection == EUROPE_USA_FREQ)
#endif
    {
        p_hs_kernel->rfMode = RF79;
        p_hs_kernel->train  = Atrain;
    }
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    else
    {
        p_hs_kernel->rfMode = RF23;
        p_hs_kernel->train  = Btrain;
    }
#endif
    /*
     * Select the Hardware/Software Hop Selection Engine
     * (which also assigns the correct handler function).
     */
    LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
}



/*
 * Build a frequency table for fast access during Page/Inquiry
 * Note Page (also can support Inquiry) table organised as follows
 *  _Page_Inquiry_Frequency_Table[0]     X= 0  Tx
 *  _Page_Inquiry_Frequency_Table[1]     X= 1  Tx
 *  _Page_Inquiry_Frequency_Table[2]     X= 2  Tx
 *  ...
 *  _Page_Inquiry_Frequency_Table[32]    X= 0  Rx
 *  _Page_Inquiry_Frequency_Table[33]    X= 1  Rx
 *  ....
 *  _Page_Scan and _Inquiry_Scan tables have Rx first, Tx next
 */
u_int8  _Page_Inquiry_Frequency_Table[64];
u_int8  _Page_Scan_Frequency_Table[64];

#if (PRH_BS_CFG_SYS_HOP_MODE_EUROPE_USA_ONLY_SUPPORTED==1)
/*
 * Inquiry/Inquiry Scan always uses the GIAC for frequency hopping for all IACs
 * Table is therefore fixed and the same for all Inquiry/InquiryScan IAC
 */
const u_int8 _LSLChop_Inquiry_Frequency_Table_79hop[64] =
/*
 * The Inquiry table can be fixed (ROM) e.g. for 79 hop ONLY, as follows
 *    Tx channels for X=[0:31],  Rx channels for X=[0:31]
 */
 {
  43, 59, 27, 77, 45, 61, 29,  0, 47, 63, 31,  2, 49, 65, 33,  4,
  51, 67, 35,  6, 53, 69, 37,  8, 55, 71, 39, 10, 57, 73, 41, 75,

  16, 44, 12, 56, 24, 52, 20, 50, 18, 46, 14, 58, 26, 54, 22, 64,
  32, 60, 28, 72, 40, 68, 36, 66, 34, 62, 30, 74, 42, 70, 38, 48
};

#else
/*
 * Inquiry Scan Table in RAM
 * Keep a separate table so that inquiry/page scans can be built at initialisation
 */
u_int8  _Inquiry_Scan_Frequency_Table[64];
/*
 * 23 HOP Engine Required
 */
static t_freq _LSLChop_Execute_HSE_Connection_23(
    t_hs_kernel *p_hs_kernel, t_clock clk);
#endif

/************************************************************************
 * LSLChop_Build_Frequency_Table
 *
 * Builds a table of X to frequency mappings for selected ULAP/state
 *
 * state        Page|Inquiry|Page_Scan|Inquiry_Scan
 * uap_lap      The UAP and LAP for the frequencies
 ************************************************************************/
void LSLChop_Build_Frequency_Table(t_state state, t_uap_lap uap_lap)
{
    t_hs_kernel table_hse_kernel;
    boolean   num_freqs = 16 << (LSLChop_hs_engine.rfMode==RF79);
    u_int8    *next_freq_entry;
    u_int     X;
    t_state   alt_state;

    if (state == Page_Scan)
    {
        next_freq_entry = _Page_Scan_Frequency_Table;
        alt_state = SlavePageResponse;
    }
    else if (state == Inquiry_Scan)
    {
#if (PRH_BS_CFG_SYS_HOP_MODE_EUROPE_USA_ONLY_SUPPORTED==1)
        return;  /* Nothing to do, table in ROM! */
#else
        next_freq_entry = _Inquiry_Scan_Frequency_Table;
        alt_state = InquiryResponse;
#endif
    }
    else
    {
        next_freq_entry = _Page_Inquiry_Frequency_Table;
        alt_state = state;
    }

    /*
     * Setup new Kernel for table required
     * This ensures that if Software HSE is used following Page/Pagescan
     * that the table is correctly setup.
     */
    LSLChop_Build_Hop_Kernel(&table_hse_kernel, uap_lap);

    for( X=0; X < num_freqs; X++)
    {
        /*
         * Page/Inquiry:            Transmit frequency for X
         * Page_Scan/Inquiry_Scan:  Receive  frequency for X
         */
         *next_freq_entry = _LSLChop_Execute_HSE_Non_Connection(
             &table_hse_kernel, 0/*clk*/, state, X);

        /*
         * Page/Inquiry:            Receive  frequency for each X
         * Page_Scan/Inquiry_Scan:  Transmit frequency for X
         */
         *(next_freq_entry+32) = _LSLChop_Execute_HSE_Non_Connection(
             &table_hse_kernel, BIT1/*clk*/, alt_state, X);
         next_freq_entry++;
     }
}

/************************************************************************
 * LSLChop_Get_Frequency_Channel_Connection()
 *
 * Determine frequency for a Connection.  
 *
 * Refer to called functions for detailed parameter descriptions.
 *
 * p_hs_kernel              Defines initial kernel params: A, B, C, D, E, F
 *                          based on uap_lap (address part) 
 * clk                      clock in ticks for frequency
 *
 * RETURNS: the calculated frequency for the clock value
 ************************************************************************/
t_freq LSLChop_Get_Frequency_Channel_Connection(
    t_hs_kernel *p_hs_kernel, t_clock clk)
{
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
    return LSLChop_hs_engine.hs_connection_fn(p_hs_kernel, clk);

#elif (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==0)
    /*  
     * Always use default
     */
    return LSLChop_hs_engine.hs_connection_fn(&LSLChop_hs_kernel, clk);
#endif
}

/************************************************************************
 * LSLChop_Build_Hop_Kernel
 * Initialise Fixed parts of A-F based on Address, and initialise rfMode
 * Clock dependent parts are handled by the hop kernel.
 *
 * State          Address
 *
 * =====          =======
 *
 * Connection     MASTER
 *
 * Page           Paged Unit
 *
 * Inquiry        The GIAC lap + DCI for uap
 *
 ************************************************************************/
void LSLChop_Build_Hop_Kernel(t_hs_kernel *p_hs_kernel, t_uap_lap uap_lap)
{
    /*
     * Record the kernel's uap_lap to avoid unnecessary setups
     */
    p_hs_kernel->uap_lap = uap_lap;

    /* Initial values as f(Addr) for A, B, C, D, E and F.  E,F combined to EF.
     *         A = Addr[27:23]           => UAP[3:0]lap[23]
     *         B = Addr[22:19]           => LAP[22:19]
     *         C = Addr[8,6,4,2,0]       => LAP[8,6,4,2,0]
     *         D = Addr[18:10]           => LAP[18:10]
     *         E = Addr[13,11,9,7,5,3,1] => LAP[*]
     *         F = 0
     */
    p_hs_kernel->Ai = (u_int8) ( ((uap_lap>>23)&0x1E) | ((uap_lap&BIT23)>>23) );
    p_hs_kernel->Bi = (u_int8) ( (uap_lap>>19) & 0x000F );
    p_hs_kernel->Ci = (u_int8) ( ((uap_lap&BIT8)>>4) | ((uap_lap&BIT6)>>3) |
                             ((uap_lap&BIT4)>>2) | ((uap_lap&BIT2)>>1) |
                              (uap_lap&BIT0) );
    p_hs_kernel->Di = (u_int16)( (uap_lap>>10) & 0x01FF );
    p_hs_kernel->EFi= (u_int8) ( ((uap_lap&BIT13)>>7) | ((uap_lap&BIT11)>>6) |
                            ((uap_lap&BIT9) >>5) | ((uap_lap&BIT7)>>4)  |
                            ((uap_lap&BIT5)>>3)  | ((uap_lap&BIT3)>>2)  |
                            ((uap_lap&BIT1)>>1) );
}

/************************************************************************
 * LSLChop_Setup_Hop_Kernel
 *
 * Initialise Fixed parts of A-F based on Address, and initialise rfMode
 * Clock dependent parts are handled by the hop kernel.
 *
 * State          Address
 * =====          =======
 * Connection     MASTER
 * Page           Paged Unit
 * Inquiry        The GIAC lap + DCI for uap
 *
 * This function is V1.1 backwards compatible.
 * There was only a single hs kernel pre V1.2.
 ************************************************************************/
void LSLChop_Setup_Hop_Kernel(t_uap_lap uap_lap)
{
    /* 
     * Build the current selected Kernel
     */
    LSLChop_Build_Hop_Kernel(&LSLChop_hs_kernel, uap_lap);
}

/************************************************************************
 * _LSLChop_Get_X_Page_Inquiry
 *
 *  Return the X input for the corresponding clk and rfMode
 *
 * Page:
 *  RF79 X=(CLKN[16:12]+koffset+(CLKN[4:2,0]-CLKN[16-12])mod16)mod32
 *  RF23 X=(CLKN[15:12]+8+CLKN[4:2,0])mod16
 * Inquiry:
 *  RF79 X=(CLKE[16:12]+koffset+(CLKE[4:2,0]-CLKE[16:12])mod16)mod32
 *  RF23 X=(CLKE[15:12]+8+CLKE[4:2,0])mod16
 *  train => koffset (trainA/B= 24/8) for RF79,
 *  always 8 'Btrain' for RF23
 ************************************************************************/
__INLINE__ u_int _LSLChop_Get_X_Page_Inquiry(t_clock clk)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine;
    u_int X;

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    if (p_hs_engine->rfMode==RF79)
#endif
    {
        X =          ( ( (clk>>12) /*& 0x001F*/) +
                     ( ( (((clk>> 1) & 0x000E) | (clk & BIT0))
                                - (clk>>12)  ) & 0x0F)
                      + p_hs_engine->train ) & 0x1F;
    }
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    else
    {
        X =          ( ((clk>>12) /*& 0x000F*/) +
                     ( ((clk>> 1) & 0x000E) | (clk & BIT0) )
                     + 8 ) & 0xF;
    }
#endif
    return X;
}

/************************************************************************
 * LSLChop_Get_Next_Frequency
 * Generates the next frequency in the frequency hopping sequence.
 *
 * NOTE:  The clk must be defined as follows:
 *
 * State              Role       Clock     HOP Address Used
 *
 * =====              ====       =====     ================
 *
 * Page               M          SLAVE(E)  BD_ADDR of Paged Unit
 *
 * MasterPageResponse M          SLAVE(E)  BD_ADDR of Paged Unit
 *
 * PageScan           S          OWN       Own BD_ADDR
 *
 * SlavePageResponse  S          MASTER     Own BD_ADDR
 *
 * Inquiry            M          INQUIRER  Always the GIAC
 *
 * InquiryScan        S          OWN       Always the GIAC
 *
 * InquiryResponse    S          OWN       Always the GIAC
 *
 * Connection         M/S        MASTER    BD_ADDR of MASTER
 *
 * Source V1.1
 *
 * For states with both Tx/Rx, the clk BIT1 determines whether Tx[0], Rx[1].
 *
 * SlavePageResponse Anomaly
 * The clock used must track the Master Tx/Rx slots.  Hence the clock must
 * be a piconet aligned clock.  CLKE is in effect that since CLKE[1:0]==CLK[1:0]
 * but is unknown to the slave (Table 11.3 has anomaly here between Y1/Y2).
 * Until V1.1 final, the tables indicated that SPR was determined by CLKN1 but
 * this could never work since CLKN is free running (non-aligned).  In V1.1
 * for Y2 it was indicated that CLKE should be used without detail (this 
 * should also apply to Y1).  Hence, it is adequate that clk is BT_CLK.
 *
 * RETURNS: the next frequency in the frequency hopping sequence.
 * NOTES:
 *    The train selection A/B is used for interlaced scanning.  By
 *    selecting the alternative train LSLChop_Set_Train(Alternative_Scan)
 *    the second frequency will be calculated and returned.
 *
 *      Xsecond = (Xfirst + 16) mod 32
 *
 *      But Xfirst  = CLKN[16-12] == (CLKN >> 12) mod 32
 *      =>  Xsecond = ((CLKN>>12)+16) mod 32
 *
 *    u_int  used where 16/32 bit appropriate for speed.
 *************************************************************************/
t_freq LSLChop_Get_Next_Frequency(t_state state, t_clock clk /* clk value, see note */ )
{
    /* 
     * Optimised access to local hs kernel structure
     */
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine;
    t_hs_kernel *p_hs_kernel = &LSLChop_hs_kernel; 
    u_int  maskBits;
    u_int  X;
    volatile t_freq frequency = 0x00;

    /*
     * Make sure that the kernel is setup for Connection state
     */
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    if (p_hs_engine->rfMode==RF79)
    {
        maskBits = 0x1F;
    }
    else /* RF23 */
    {
        maskBits = 0xF;
    }
#else
    maskBits = 0x1F;
#endif

/*
 * Single Frequency     Always assume from Master's viewpoint i.e. M tx | M rx
 * By context can define by:
 * State              Role       Tx Hop    Rx Hop (BIT1 of clock)
 * =====              ====       ======    ======
 * Page               M          M tx 0    M rx 1
 * MasterPageResponse M          M tx 0    M rx 1
 * PageScan           S          M tx -    M tx -       Receive always on M tx
 * SlavePageResponse  S          M rx 0    M tx 1
 * Inquiry            M          M tx 0    M rx 1
 * InquiryScan        S          M tx -    M tx -       Receive always on M tx
 * InquiryResponse    S          M rx 0    M tx 1       
 * Connection         M          M tx 0    M rx 1       Normal master
 * Connection         S          M rx 0    M tx 1       Slave, obey master!
 *
 * If (In M Receive Slot  And (State == [Page|MPR|Inquiry|Connection|SPR|IR] )
 *    Hop Frequency is M rx
 * Else
 *    Hop Frequency is M tx
 * Endif
 *
 */

    if (p_hs_engine->rfSelection == SINGLE_FREQ)
    {
        if ( (clk&BIT1) &&              /* Master Receive  Slot         */
               ( (state==Page) || (state==MasterPageResponse) ||
                 (state==Inquiry) || (state==Connection) ||
                 (state==SlavePageResponse) || (state==InquiryResponse) ) ) 
        {
            frequency = p_hs_engine->rxMasterHopFixedFreq;
        }
        else                            /* Master TxSlave transmission    */
        {
            frequency = p_hs_engine->txMasterHopFixedFreq;
        }
        return frequency;
    }

    else if (p_hs_engine->rfSelection == REDUCED_FREQ)
    {
        /*
         * Reduced Hopping Sequence [optional]
         * Channel 0, 23, 46, 69, 93 are used [see V1.0 p 797]
         */
        frequency = ( (clk>>1)%5 )* 23;

        if (frequency==92)
        {
            frequency++;
        }
        return frequency;
    }

    /***********************************************************************/

    /*
     * 1. Determine kernel X input based on state
     */
    switch(state)
    {
    case Connection:
        /*
         * Master and Slave State: Connection (always the priority!)
         */
        X =  ((clk>>2) & maskBits);     /* RF79\23  CLK[6:2]\CLK[5:2] */

        /*
         * Execute the previously selected engine.
         */
        frequency = p_hs_engine->hs_connection_fn(p_hs_kernel, clk);
        break;

    /*
     **** Master States:  Page, Inquiry, and MasterResponse
     */
    case Page:
        /*
         *  RF79 X=(CLKN[16:12]+koffset+(CLKN[4:2,0]-CLKN[16-12])mod16)mod32
         *  RF23 X=(CLKN[15:12]+8+CLKN[4:2,0])mod16
         */
    case Inquiry:
        /*
         *  RF79 X=(CLKE[16:12]+koffset+(CLKE[4:2,0]-CLKE[16:12])mod16)mod32
         *  RF23 X=(CLKE[15:12]+8+CLKE[4:2,0])mod16
         *  train => koffset (trainA/B= 24/8) for RF79,
         *  always 8 'Btrain' for RF23
         */

        X = _LSLChop_Get_X_Page_Inquiry(clk);

#if (PRH_BS_CFG_SYS_HOP_MODE_EUROPE_USA_ONLY_SUPPORTED==1)
        if (state == Inquiry)
        {
             frequency = _LSLChop_Inquiry_Frequency_Table_79hop[X + ((clk&BIT1)<<4)];
        }
        else
#endif
        {
            frequency = _Page_Inquiry_Frequency_Table[X + ((clk&BIT1)<<4)];
        }
        break;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
    case MasterExtendedInquiryResponse:
        X = _LSLChop_Get_X_Page_Inquiry(clk-4);
        frequency = _LSLChop_Inquiry_Frequency_Table_79hop[X + ((clk&BIT1)<<4)];
        break;
#endif
        
    case MasterPageResponse:
        /*
         *  RF79
         *    X=(CLKE*[16:12]+koffset+(CLKE*[4:2,0]-CLKE*[16-12]mod16)+N)mod32
         *
         *  RF23 X=(CLKE*[15:12] + 8 + CLKE*[4:2,0]+N)mod16
         *
         *  train => koffset (trainA/B= 24/8), must be frozen from Page
         *  N incremented on each CLKE[1] going to zero, and before first
         *  FHS sent (hence +3 below).
         */
        X = (p_hs_engine->frozen_X_page +
            ((clk - p_hs_engine->frozen_clk_page_inq + 3)>>2)  ) & maskBits;
        frequency = _Page_Inquiry_Frequency_Table[X + ((clk&BIT1)<<4)];
        break;

    /*
     **** Slave States: PageScan, SlavePageResponse
     *                  InquiryScan, InquiryResponse
     */
    case Page_Scan:
        /*
         *  Listen at frequency, change at rate CLKN bit 12
         *  See function header for train usage in interlaced scan.
         *  For backwards compatibility, compare to != Alternative_Scan
         */
        if (p_hs_engine->train!=Alternative_Scan)
        {
            X  = clk>>12;
        }
        else
        {
            /*
             * Interlaced scan (must base on first scan not clock)
             */
            X = p_hs_engine->frozen_X_page_scan + 16;
        }
        X &= maskBits;

        /*
         * Associated clock will be frozen when corresponding Response
         * is received since it is only at that stage that the clocks are aligned.
         */
        p_hs_engine->frozen_X_page_scan = X;  /* Remember X for SlaveResponse   */

        frequency = _Page_Scan_Frequency_Table[X];
        break;

    case Inquiry_Scan:
        /*
         *  Listen at frequency, change at rate CLKN bit 12
         *  See function header for train usage in interlaced scan.
         *  For backwards compatibility, compare to != Alternative_Scan
         */
        if (p_hs_engine->train!=Alternative_Scan)
        {
            X  = (clk>>12) + p_hs_engine->phase_N_inquiry_scan;
        }
        else
        {
            /*
             * Interlaced scan (must base on first scan not clock)
             */
            X = p_hs_engine->frozen_X_inquiry_scan + 16;
        }
        X &= maskBits;

        /*
         * Associated clock will be frozen when corresponding Response
         * is received since it is only at that stage that the clocks are aligned.
         */
        p_hs_engine->frozen_X_inquiry_scan = X;

#if (PRH_BS_CFG_SYS_HOP_MODE_EUROPE_USA_ONLY_SUPPORTED==1)
        frequency = _LSLChop_Inquiry_Frequency_Table_79hop[X];
#else
        frequency = _Inquiry_Scan_Frequency_Table[X];
#endif
        break;

    case InquiryResponse:
        /*
         *  RF79\23 X=CLKN*[16\15-12]+N) mod 32\16
         *  N explicitly incremented on each FHS packet sent
         *
         *  Explicitly use the increment function LSLChop_Increment_N()
         */
		X = p_hs_engine->frozen_X_inquiry_scan;

        /*
         * Frequency here is may be a transmit/receive frequency.
         * The clk parameter BIT1 determines Tx[0]/Rx[1] but its absolute
         * value is meaningless!
         */
#if (PRH_BS_CFG_SYS_HOP_MODE_EUROPE_USA_ONLY_SUPPORTED==1)
        frequency = _LSLChop_Inquiry_Frequency_Table_79hop[X + 32];
#else
        frequency = _Inquiry_Scan_Frequency_Table[X + 32];
#endif
        break;

    case SlavePageResponse:
        /*
         *  RF79\23 X=CLKN*[16\15-12]+N) mod 32\16
         *  N incremented on each CLKN[1], starting at 0
         *  Anomaly in V1.0b Specification:  CLKN is not aligned with CLK.
         */
        /*
         *  PageScan routine responsible for incrementing X
         */
        X = p_hs_engine->frozen_X_page_scan & maskBits;
        frequency = _Page_Scan_Frequency_Table[X + ((clk&BIT1)<<4)];
        break;

    default:
         X = 0;
    }

    /*
     * Now remember Actual X input for use in Data Whitening
     * Use only first half X for Inquiry until hardware complete.
     */
    if ( (state!=Inquiry) || (clk & BIT0)==0)
    {
        p_hs_engine->X = X;
    }

    return frequency;
}

/************************************************************************
 * LSLChop_Get_Table_Frequency_Page_FHS_Response_ID
 * 
 * Get the corresponding table frequency for the transmit ID to a
 *  received Page FHS 
 ************************************************************************/
t_freq LSLChop_Get_Table_Frequency_Page_FHS_Response_ID(void)
{
    /*
     * X is already incremented, use direct access to table for rapid setup
     */
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine;
    t_freq frequency;

    if (p_hs_engine->rfSelection==SINGLE_FREQ)
    {
        frequency = p_hs_engine->rxMasterHopFixedFreq;
    }
    else
    {
        frequency = _Page_Scan_Frequency_Table[LSLChop_Get_X()+32];
    }
    return frequency;
}

/************************************************************************
 * LSLChop_Get_TxRxFreqs_SlavePageResponse
 * 
 * Sets the corresponding table frequencies for the transmit ID and the
 * page FHS due to be received.
 ************************************************************************/
void LSLChop_Get_TxRxFreqs_SlavePageResponse( t_freq *p_tx_chan, t_freq *p_rx_chan)
{    
    if (LSLChop_hs_engine.rfSelection==SINGLE_FREQ)
    {
        *p_tx_chan = LSLChop_hs_engine.rxMasterHopFixedFreq;
        *p_rx_chan = LSLChop_hs_engine.txMasterHopFixedFreq;
    }
    else
    {
        *p_tx_chan = _Page_Scan_Frequency_Table[LSLChop_Get_X()+32];
        *p_rx_chan = _Page_Scan_Frequency_Table[(LSLChop_Get_X()+1)&0x1F];
    }
}

/************************************************************************
 * _LSLChop_Execute_HSE_Non_Connection
 *
 * Hop Selection Kernel in Software for non Connection state
 *
 * p_hs_kernel  Defines initial kernel params: A, B, C, D, E, F
 *              based on uap_lap (address part) 
 * clk          Clock values
 * state        Page|Inquiry|Page_Scan|Inquiry_Scan
 * X            The kernel X input
 *
 * Notes
 * The use of each local block below optimises register performance.
 *
 ************************************************************************/
static t_freq _LSLChop_Execute_HSE_Non_Connection(
    t_hs_kernel *p_hs_kernel, t_clock clk, t_state state, u_int X)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine;
    u_int  rfMode = p_hs_engine->rfMode;

    register u_int  Result;
    u_int           Y1;
    u_int  numBits;
    u_int  maskBits;

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    if (rfMode==RF79)
    {
        numBits = 5;
        maskBits = 0x1F;
    }
    else /* RF23 */
    {
        numBits = 4;
        maskBits = 0xF;
    }
#else
    numBits = 5;
    maskBits = 0x1F;
#endif

    /*
     * Y1  SlaveToMaster (based on CLK1), 0 if scan
     * Y2  is derived (Y1 << numBits)
     */
    if ((state==Inquiry_Scan) || (state==Page_Scan) )
    {
        Y1 = 0;                         /* Always Assume Master to Slave    */
    }
    else
    {
        Y1 = (clk & BIT1) >> 1;
    }

    /*
     * Stage 1*** ADD
     */
    {
        u_int  A=p_hs_kernel->Ai;
        Result = (X + A) & maskBits;
    }

    /*
     * Stage 2*** XOR   B not modified by clock in Connection
     */
    {
        Result ^= p_hs_kernel->Bi;
    }

    /*
     * Stage 3*** Permutate
     *   for each Permutation Operation do
     *     if (Permutation Necessary) then
     *        if (Bits to Permutate are different) then
     *           Swap Bits
     *        endif
     *     endif
     *   endfor
     */
    {
        u_int permControl;

        {
            u_int  C=p_hs_kernel->Ci;
            u_int  D=p_hs_kernel->Di;

            permControl = D | ((Y1) ? (C ^ 0x1F) : C) << 9;
        }
        {
            const u_int8    *permEntry;
            register u_int  mask;

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
            permEntry = ((rfMode == RF79) ? permutationRF79+13 : permutationRF23+13);
#else
            permEntry = permutationRF79+13;
#endif
            mask=BIT13;
            while (mask!=0)
            {
                register u_int permute = *permEntry--;
                if (permControl & mask)
                {
                    if( ! ( (permute&Result) == 0 || (permute&Result)==permute))
                        Result ^= permute;
                }
                mask>>=1;
            }
        }
    }

    /*
     * Stage 4*** Add and Modulo rfMode  79\23
     * Use subtraction due to limited summation i.e. max if RF79=317, RF23=295
     * Result = Result + E + F + Y2 where Y2 is (Y1 << numBits)
     */
    {
        u_int EF=p_hs_kernel->EFi;

        Result += ( (u_int16) EF + (Y1<<numBits) );
        while( Result >= rfMode)            /* Fast:  Result %= rfMode;     */
        {
            Result -= rfMode;
        }
    }

    /*
     * Stage 5*** Frequency selection [Algorithm based]
     *            Lookup Table may be needed for adaptive frequency elimination.
     * if (Result in Table TopHalf ( <=rfMode/2) then
     *   Result = Result*2                  * Table TopHalf    0,2,4,6,  ,rfMode-1 *
     * else
     *   Result = (Result-rfMode/2-1)*2 + 1 * Table BottomHalf 1,3,5,7,  ,rfMode-2 *
     * endif
     */
    Result <<= 1;
    if (Result > rfMode)
    {
        Result -= rfMode;
    }

    return Result;
} /* End of Kernel Processing */

/************************************************************************
 * LSLChop_Set_Train
 *
 * Select the train for paging and inquiry state machines, or the
 * equivalent for V1.2 interlaced scanning, if supported.
 *
 * 1. Paging and Inquiry State Machines control setting of Trains
 *    to Atrain or Btrain.
 * 2. For interlaced page or inquiry scans, the train function can be 
 *    used to select the Standard_Scan or Alternative Scan.  
 *
 ************************************************************************/
void LSLChop_Set_Train(t_train train_type)
{
    LSLChop_hs_engine.train = train_type;
}

/************************************************************************
 * LSLChop_Get_Train
 * Will not get train automatically, leave to Paging Logic.
 *
 * RETURNS a t_train type.
 *
 ************************************************************************/
t_train LSLChop_Get_Train(void)
{
    return LSLChop_hs_engine.train;
}

/************************************************************************
 * LSLChop_Toggle_Train
 * Will not toggle train automatically, leave to Paging Logic
 ************************************************************************/
void LSLChop_Toggle_Train(void)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine;

    if (p_hs_engine->train==Atrain)
    {
        p_hs_engine->train = Btrain;
    }
    else
    {
        p_hs_engine->train = Atrain;
    }
}

/************************************************************************
 *  LSLChop_Get_X
 *  Accessor used for setting the Whitening word during FHS i.e.
 *  Slave Page Response
 *
 *  RETURNS: the current X value
 ************************************************************************/
u_int LSLChop_Get_X(void)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine;

    if (p_hs_engine->rfSelection == SINGLE_FREQ)
    {
        return 0;
    }
    else
    {
        return p_hs_engine->X;
    }
}

/************************************************************************
 * LSLChop_Freeze_X_for_Page
 * Modifier for freezing the X input
 * -- Called in Master when Slave Page Response ID received from Slave
 *    The clock used must be the BT clock corresponding to the rx slot.
 *
 * Recalculates and freezes the X value.
 ************************************************************************/
void LSLChop_Freeze_X_for_Page(t_clock clk)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine; /*Optimised access to structure*/

    p_hs_engine->frozen_clk_page_inq = clk;
    p_hs_engine->frozen_X_page = _LSLChop_Get_X_Page_Inquiry(clk);
}

/************************************************************************
 * LSLChop_Set_X_for_Inquiry
 * Modifier for setting explicitly the X input
 * -- Called in when lookahead used for frequency calculations
 *
 * Recalculates and sets the X value.
 ************************************************************************/
void LSLChop_Set_X_for_Inquiry(t_clock clk)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine; /*Optimised access to structure*/

    p_hs_engine->frozen_clk_page_inq = clk;
    p_hs_engine->X = _LSLChop_Get_X_Page_Inquiry(clk);
}

/************************************************************************
 * LSLChop_Increment_X_Page_Scan
 * Modifier for incrementing the current X for each expected response.
 ************************************************************************/
void LSLChop_Increment_X_Page_Scan(void)
{
    ++(LSLChop_hs_engine.frozen_X_page_scan);
}

/************************************************************************
 * LSLChop_Increment_N
 * Modifier for incrementing the current N input each time an FHS sent
 ************************************************************************/
void LSLChop_Increment_N_Inquiry_Scan(void)
{
    ++(LSLChop_hs_engine.phase_N_inquiry_scan);
}

/************************************************************************
 * LSLChop_Get_rfSelection
 * Accessor for current rfSelection
 ************************************************************************/
t_rfSelection LSLChop_Get_rfSelection(void)
{
    return LSLChop_hs_engine.rfSelection;
}

/************************************************************************
 * _LSLChop_Execute_HSE_Reduced
 *
 * Hop Selection Kernel for fixed frequencies
 *
 * clk                      clock in ticks for frequency
 * p_hs_kernel              Ignored 
 ************************************************************************/
static t_freq _LSLChop_Execute_HSE_Reduced(t_hs_kernel *p_hs_kernel, t_clock clk)
{
    t_freq frequency;

    /*
     * Reduced Hopping Sequence [optional]
     * Channel 0, 23, 46, 69, 93 are used [see V1.0 p 797]
     */
    frequency = ( (clk>>1)%5 ) * 23;

    if (frequency==92)
    {
        frequency++;
    }
    return frequency;
}

/************************************************************************
 * _LSLChop_Execute_HSE_Single_Connection
 *
 * Hop Selection Kernel for fixed frequencies for Connection Only
 *
 * clk                      clock in ticks for frequency
 * p_hs_kernel              Contains fixed Rx/Tx frequencies  
 ************************************************************************/
static t_freq _LSLChop_Execute_HSE_Single_Connection(
    t_hs_kernel *p_hs_kernel, t_clock clk)
{
    return (clk&BIT1) ? LSLChop_hs_engine.rxMasterHopFixedFreq
                      : LSLChop_hs_engine.txMasterHopFixedFreq;
}

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
/************************************************************************
 * _LSLChop_Execute_HSE_Connection_23
 *
 * Hop Selection Kernel for 23 mode
 * The use of each local block below optimises register performance.
 * 
 * The Kernel calculates the frequency based on clock and kernel parameters.
 *
 * p_hs_kernel              Defines initial kernel params: A, B, C, D, E, F
 *                          based on uap_lap (address part) 
 * clk                      clock in ticks for frequency
 ************************************************************************/
static t_freq _LSLChop_Execute_HSE_Connection_23(
    t_hs_kernel *p_hs_kernel, t_clock clk)
{
    register u_int  Result;

    /*
     * Stage 1*** ADD
     */
    {
        u_int X = (u_int) (clk>>2);             /* RF23  CLK[5:2] */
        u_int A = p_hs_kernel->Ai ^ (clk>>21);  /* A[27:23] XOR CLK[25:21]    */
        Result = (X + A) & 0x000F;      /* Ensure only 4 bits           */
    }

    /*
     * Stage 2*** XOR   B not modified by clock in Connection
     */
    {
        Result ^= p_hs_kernel->Bi;
    }

    /*
     * Stage 3*** Permutate
     *   for each Permutation Operation do
     *     if (Permutation Necessary) then
     *        if (Bits to Permutate are different) then
     *           Swap Bits
     *        endif
     *     endif
     *   endfor
     */
    {
        register u_int permControl;
        u_int C;

        /*
         * permControl (P) = 
         *  P0-8 corresponds to D0-8, and, 
         *  Pi+9 corresponds to Ci XOR Y1 for i =0,4 where Y1 = clk & BIT1
         *  
         * Start with the D input = A[18:10] XOR CLK[15-7]
         */
        permControl = p_hs_kernel->Di^((clk>> 7) & 0x01FF);  
        /*
         * Now include the C input = A[8,6,4,2,0] XOR CLK[20-16]
         */
        C = p_hs_kernel->Ci ^ ((clk>>16) & 0x001F); 
        if (clk & BIT1)
        {
            C ^= 0x1F;
        }
        permControl |= (C << 9);

#if LSLCHOP_USE_FAST_OPTIMISED_PERMUTATOR 
        {
            register u_int16 b0, b1, b2, b3, f;

            /*
             * b0=BIT0;b1=BIT1;b2=BIT2;b3=BIT3;b4=BIT4;
             */
            b0 = Result&BIT0;
            b1 = Result&BIT1;
            b2 = Result&BIT2;
            b3 = Result&BIT3;

            if (permControl & BIT13) {f=b2;b2=b3;b3=f;} /* 2|3 */
            if (permControl & BIT12) {f=b0;b0=b1;b1=f;} /* 0|1 */
            if (permControl & BIT11) {f=b1;b1=b2;b2=f;} /* 1|2 */
            if (permControl & BIT10) {f=b0;b0=b3;b3=f;} /* 0|3 */

            if (permControl & BIT9)  {f=b1;b1=b3;b3=f;} /* 1|3 */
            if (permControl & BIT8)  {f=b0;b0=b2;b2=f;} /* 0|2 */
            if (permControl & BIT7)  {f=b2;b2=b3;b3=f;} /* 2|3 */
            if (permControl & BIT6)  {f=b0;b0=b1;b1=f;} /* 0|1 */
            if (permControl & BIT5)  {f=b1;b1=b3;b3=f;} /* 1|3 */

            if (permControl & BIT4)  {f=b0;b0=b2;b2=f;} /* 0|2 */
            if (permControl & BIT3)  {f=b1;b1=b2;b2=f;} /* 1|2 */
            if (permControl & BIT2)  {f=b0;b0=b3;b3=f;} /* 0|3 */
            if (permControl & BIT1)  {f=b2;b2=b3;b3=f;} /* 2|3 */
            if (permControl & BIT0)  {f=b0;b0=b1;b1=f;} /* 0|1 */

            Result = (b0!=0); /*Bit0 if b0 <> 0*/
            if (b1) {Result|=BIT1;}
            if (b2) {Result|=BIT2;}
            if (b3) {Result|=BIT3;}
        }

#elif !LSLCHOP_USE_FAST_OPTIMISED_PERMUTATOR
       {
            const u_int8    *permEntry = permutationRF23+13;
            register u_int  mask;

            mask=BIT13;
            while (mask!=0)
            {
                register u_int permute = *permEntry--;
                if (permControl & mask)
                {
                    if( ! ( (permute&Result) == 0 || (permute&Result)==permute))
                        Result ^= permute;
                }
                mask>>=1;
            }
       }
#endif
    }


    /*
     * Stage 4*** Add and Modulo rfMode  23
     * Kernel Addition     E + Y2 + F + PERM4out
     * where E is EFi                                           (max 128)
     *       Y2 is 16 clk 1                                     (max 16)
     *       F is 16 x CLK27-6 mod N
     *       PERM4out is output of the permutator               (max 16)
     *
     * Note postponing mod N does not cause u_int32 overflow since
     * 16 x CLK27-6 leaves is max ~2*11!
     *
     * Result = Result + E + F + Y2 where Y2 is (Y1 << numBits)
     */

    /*
     * Add to result E + F' pre mod
     */
    Result += p_hs_kernel->EFi + ((clk >> 6) << 3);

    /*
     * Add on Y2 = CLK1 * 16 (extract clk bit1 and shift 3 left!)
     */
    Result += ((clk&BIT1)<<3);
    Result %= 23;

    /*
     * Stage 5*** Frequency selection [Algorithm based]
     * if (Result in Table TopHalf ( <=rfMode/2) then
     *   Result = Result*2                  * Table TopHalf    0,2,4,6,  ,22 *
     * else
     *   Result = (Result-rfMode/2-1)*2 + 1 * Table BottomHalf 1,3,5,7,  ,21 *
     * endif
     */
    Result <<= 1;
    if (Result > 23)
    {
        Result -= 23;
    }

    return Result;
} /* End of Kernel Processing */

#endif
/************************************************************************
 * _LSLChop_Execute_HSE_Connection_79
 *
 * Hop Selection Kernel for 79 mode
 * 
 * The Kernel calculates the frequency based on clock and kernel parameters.
 *
 * p_hs_kernel              Defines initial kernel params: A, B, C, D, E, F
 *                          based on uap_lap (address part) 
 * clk                      clock in ticks for frequency
 ************************************************************************/
static t_freq _LSLChop_Execute_HSE_Connection_79(
    t_hs_kernel *p_hs_kernel, t_clock clk)
{
    register u_int Result;

    Result = LSLChop_Execute_79_Hop_Kernel_Connection_Summation(
        clk, p_hs_kernel);

    /*
     * Store the summation for used in AFH if necessary.
     */
    p_hs_kernel->sum_before_mod = Result;

    /*
     * Reduce to valid 79 hop frequency
     */
    Result %= 79;

    /*
     * Stage 5*** Frequency selection [Algorithm based]
     * if (Result in Table TopHalf ( <=rfMode/2) then
     *   Result = Result*2                  * Table TopHalf    0,2,4,6,  ,78 *
     * else
     *   Result = (Result-rfMode/2-1)*2 + 1 * Table BottomHalf 1,3,5,7,  ,77 *
     * endif
     */
    Result <<= 1;
    if (Result > 79)
    {
        Result -= 79;
    }
    return Result;
}

/************************************************************************
 * LSLChop_Execute_79_Hop_Kernel_Connection_Summation
 *
 * Hop Selection Kernel in Software to support AFH or otherwise
 * The use of each local block below optimises register performance.
 * 
 * The Kernel calculates the summation.
 *
 * For Connection: A, C and D, F control inputs are f(clk, bd_addr)
 ************************************************************************/
u_int32 LSLChop_Execute_79_Hop_Kernel_Connection_Summation(
    t_clock clk, t_hs_kernel *p_hs_kernel)
{
    register u_int32  Result;

    /*
     * Stage 1*** ADD   (note single mask applied)
     */
    {
        u_int X = (u_int) (clk>>2);             /* RF79  CLK[6:2] */
        u_int A = p_hs_kernel->Ai ^ (clk>>21);  /* A[27:23] XOR CLK[25:21]    */

        Result = (X + A) & 0x001F;
    }

    /*
     * Stage 2*** XOR   B not modified by clock in Connection
     */
    {
        Result ^= p_hs_kernel->Bi;
    }

    /*
     * Stage 3*** Permutate
     *   for each Permutation Operation do
     *     if (Permutation Necessary) then
     *        if (Bits to Permutate are different) then
     *           Swap Bits
     *        endif
     *     endif
     *   endfor
     */
    {
        register u_int permControl;
        u_int C;

        /*
         * permControl (P) = 
         *  P0-8 corresponds to D0-8, and, 
         *  Pi+9 corresponds to Ci XOR Y1 for i =0,4 where Y1 = clk & BIT1
         *  
         * Start with the D input = A[18:10] XOR CLK[15-7]
         */
        permControl = p_hs_kernel->Di^((clk>> 7) & 0x01FF);  
        /*
         * Now include the C input = A[8,6,4,2,0] XOR CLK[20-16]
         */
        C = p_hs_kernel->Ci ^ ((clk>>16) & 0x001F); 
        if (clk & BIT1)
        {
            C ^= 0x1F;
        }
        permControl |= (C << 9);

#if LSLCHOP_USE_FAST_OPTIMISED_PERMUTATOR 
        {
            register u_int16 b0, b1, b2, b3, b4, f;

            /*
             * b0=BIT0;b1=BIT1;b2=BIT2;b3=BIT3;b4=BIT4;
             */
            b0 = Result&BIT0;
            b1 = Result&BIT1;
            b2 = Result&BIT2;
            b3 = Result&BIT3;
            b4 = Result&BIT4;

            if (permControl & BIT13) {f=b1;b1=b2;b2=f;} /* 1|2 */
            if (permControl & BIT12) {f=b0;b0=b3;b3=f;} /* 0|3 */
            if (permControl & BIT11) {f=b1;b1=b3;b3=f;} /* 1|3 */
            if (permControl & BIT10) {f=b2;b2=b4;b4=f;} /* 2|4 */
            if (permControl & BIT9)  {f=b0;b0=b3;b3=f;} /* 0|3 */
            if (permControl & BIT8)  {f=b1;b1=b4;b4=f;} /* 1|4 */
            if (permControl & BIT7)  {f=b3;b3=b4;b4=f;} /* 3|4 */
            if (permControl & BIT6)  {f=b0;b0=b2;b2=f;} /* 0|2 */
            if (permControl & BIT5)  {f=b1;b1=b3;b3=f;} /* 1|3 */
            if (permControl & BIT4)  {f=b0;b0=b4;b4=f;} /* 0|4 */
            if (permControl & BIT3)  {f=b3;b3=b4;b4=f;} /* 3|4 */
            if (permControl & BIT2)  {f=b1;b1=b2;b2=f;} /* 1|2 */
            if (permControl & BIT1)  {f=b2;b2=b3;b3=f;} /* 2|3 */
            if (permControl & BIT0)  {f=b0;b0=b1;b1=f;} /* 0|1 */

            Result = (b0!=0); /*Bit0 if b0 <> 0*/
            if (b1) {Result|=BIT1;}
            if (b2) {Result|=BIT2;}
            if (b3) {Result|=BIT3;}
            if (b4) {Result|=BIT4;}
        }

#elif !LSLCHOP_USE_FAST_OPTIMISED_PERMUTATOR 
        {
            const u_int8    *permEntry = permutationRF79+13;
            register u_int  mask;

            mask=BIT13;
            while (mask!=0)
            {
                register u_int permute = *permEntry--;
                if (permControl & mask)
                {
                    if( ! ( (permute&Result) == 0 || (permute&Result)==permute))
                        Result ^= permute;
                }
                mask>>=1;
            }
        }
#endif
    }

    /*
     * Stage 4*** Add and Modulo rfMode  79
     * Kernel Addition     E + Y2 + F + PERM5out
     * where E is EFi                                           (max 128)
     *       Y2 is 32 clk 1                                     (max 32)
     *       F is 16 x CLK27-7 mod N
     *       PERM5out is output of the permutator               (max 32)
     *
     * Note postponing mod N does not cause u_int32 overflow since
     * 16 x CLK27-7 leaves is max ~2*11!
     *
     * Result = Result + E + F + Y2 where Y2 is (Y1 << numBits)
     */

    /*
     * Add to result E + F' pre mod
     */
    Result += p_hs_kernel->EFi + ((clk >> 7) << 4);

    /*
     * Add on Y2 = CLK1 * 32 (extract clk bit1 and shift 4 left!)
     */
    Result += ((clk&BIT1)<<4);

    return Result;
} /* End of Kernel Processing */

/************************************************************************
 * LSLChop_Set_HSE_Engine
 *
 * Sets the Hop Selection Engine to hardware/software if supported
 * and select the Connection Hop Kernel function based on rf selection
 *
 * hse_type     LSLChop_HSE_HARDWARE or LSLChop_HSE_SOFTWARE
 ************************************************************************/
void LSLChop_Set_HSE_Engine(t_hse_type hse_type)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine;
    t_rfSelection rf_hop_selection;

#if (BUILD_TYPE==UNIT_TEST_BUILD)
    /*
     * Specifically, to allows HW HSE to be turned off during system test
     */
    if (!SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_HW_HOP_SELECTION_ENGINE_FEATURE))
    {
        hse_type = LSLChop_HSE_SOFTWARE;
    }
#endif

    p_hs_engine->hs_engine_type = hse_type;

    /*
     * _LSLChop_Select_Hop_Kernel_Connection
     *
     * Select the Connection Hop Kernel function based on 
     * 1. Radio Frequency Selection    SINGLE, EUROPE_USA, FRANCE, REDUCED
     * 2. Hardware or Software based hop selection engine
     */
    rf_hop_selection = LSLChop_hs_engine.rfSelection;

    if (rf_hop_selection==SINGLE_FREQ)
    {
        p_hs_engine->hs_connection_fn = _LSLChop_Execute_HSE_Single_Connection;
    }
    else if (rf_hop_selection==REDUCED_FREQ)
    {
        p_hs_engine->hs_connection_fn = _LSLChop_Execute_HSE_Reduced;
    }
    else if (hse_type==LSLChop_HSE_HARDWARE)
    {
        /*
         * Function below supports both 79/23 hop
         */
        p_hs_engine->hs_connection_fn = _LSLChop_Execute_HSE_Connection_Hardware;
    }
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    else if (rf_hop_selection==FRANCE_FREQ)
    {
         p_hs_engine->hs_connection_fn = _LSLChop_Execute_HSE_Connection_23;
    }
#endif
    else /* default if (rf_hop_selection==EUROPE_USA_FREQ) */
    {
         p_hs_engine->hs_connection_fn = _LSLChop_Execute_HSE_Connection_79;
    }
}

/************************************************************************
 * LSLChop_Get_HSE_Engine
 * Sets the Hop Selection Engine to hardware/software if supported
 * 
 ************************************************************************/
t_hse_type LSLChop_Get_HSE_Engine(void)
{
    return LSLChop_hs_engine.hs_engine_type;
}

#if (PRH_BS_CFG_SYS_HW_HOP_SELECTION_ENGINE_SUPPORTED==1)
#include "hw_register.h"
#include "hw_lc.h"

#if ((PRH_BS_CFG_TABASCO_VERSION < PRH_BS_HW_TABASCO_VERSION_T1_0_0) || (BUILD_TYPE==UNIT_TEST_BUILD))

/*
 * V1.1 Jalapeno Hop Selection Engine Registers
 *
 *  Addr       b7     b6     b5     b4     b3     b2     b1     b0
 *          +------+------+------+------+------+------+------+------+
 *  CONTROL |   Clk_Delta[2:0]   | INDEX| PAGE | H_   | H_   | SYS  |
 *  D0      |                    | MAP  |OFFSET| SLAVE| PAGE |      |
 *          +------+------+------+------+------+------+------+------+
 *  D1      |SUP_BT| SEL[1:0]    |   N_COUNT[4:0]                   |
 *          |_CLK[0|             |                                  |
 *          +------+------+------+------+------+------+------+------+
 *  D2      |     SUP_BT_CLOCK[8:1]                                 |
 *          +------+------+------+------+------+------+------+------+
 *  D3      |     SUP_BT_CLOCK[16:9]                                |
 *          +------+------+------+------+------+------+------+------+
 *  D4 HOP  |######|         HOP[6:0]                                |
 *          +------+------+------+------+------+------+------+------+
 */
typedef enum e_hop_sel { SEL_PAGE_SCAN_INQUIRY_SCAN = 0,
                                  SEL_PAGE_INQUIRY = 1,
                                  SEL_INQUIRY_PAGE_RESPONSE_MASTER_SLAVE = 2,
                                  SEL_CONNECTION = 3 } t_hop_hw_sel;

/************************************************************************
 * _LSLChop_Execute_HSE_Connection_Hardware
 *
 * Hop Selection Kernel using hardware hop selection engine
 *
 * p_hs_kernel              Ignored 
 * clk                      clock in ticks for frequency
 ************************************************************************/
static t_freq _LSLChop_Execute_HSE_Connection_Hardware(
    t_hs_kernel *p_hs_kernel, t_clock clk)
{
    t_freq hw_frequency;

    mHWreg_Create_Cache_Register(JAL_HSE_HOP_REG);
    mHWreg_Create_Cache_Register(JAL_HSE_CONTROL_REG);

    /*
     * Required for Connection:
     *     SYS           0/1             079 HOP    1   23 HOP
     *     H_SLAVE       0               Not Used
     *     H_PAGE_OFFSET 0               Not Used
     *     INDEX_MAP     1               Linear Map Table
     *     CLK_DELTA     N               Offset Slots, to slot required.
     *     N_COUNT       0               Not Used
     *     SEL           3               Connection
     *     SUP_BT_CLK    0               Not Used
     */

    /*
     * Always Use LINEAR frequency index mapping, SEL=3, sup_bt_clk ignored
     *     mHWreg_Assert_Cache_Bit(JAL_HSE_CONTROL_REG, JAL_INDEX_MAP);
     *     mHWreg_Clear_Cache_Register(JAL_HSE_CONTROL_REG);
     *     mHWreg_Assign_Cache_Field(JAL_HSE_CONTROL_REG, JAL_SEL,
     *         SEL_CONNECTION);
     *    mHWreg_Assign_Cache_Field(JAL_HSE_CONTROL_REG, JAL_SUP_BT_CLK, clk);
     */
    /*
     * Efficient, amalgamating may not be!
     */
     mHWreg_Assign_Cache_Register(JAL_HSE_CONTROL_REG, 0x00006000);
     mHWreg_Assert_Cache_Bit(JAL_HSE_CONTROL_REG, JAL_INDEX_MAP);
    
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    /*
     * If 23 HOP Set JAL_SYS to 1 [Default is 79 HOP]
     */
    if (LSLChop_hs_engine.rfMode == RF23)
    {
        mHWreg_Assert_Cache_Bit(JAL_HSE_CONTROL_REG, JAL_SYS);
    }
#endif
    /*
     * Always set CLK_DELTA ahead for next freq required
     * The reference is always the bt_clk
     *
     * During MSS, if becoming a Master then HW HSE fails? Why??
     */
    mHWreg_Assign_Cache_Field(JAL_HSE_CONTROL_REG, JAL_CLK_DELTA,
            (clk - HW_get_bt_clk()));

    /*
     * Hop Selection Kernel now set in Hardware, Activate and Wait!!!!
     */
    mHWreg_Store_Cache_Register(JAL_HSE_CONTROL_REG);

#if (BUILD_TYPE==UNIT_TEST_BUILD)
    /*
     * Support emulated generation during regression testing
     * No hardware then just force frequency into JAL_HSE_HOP_REG register
     */
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    if (LSLChop_hs_engine.rfMode == RF23)
    {
        mHWreg_Assign_Cache_Field(JAL_HSE_HOP_REG, JAL_HOP,
            _LSLChop_Execute_HSE_Connection_23(p_hs_kernel, clk) );
    }
    else
    {
        mHWreg_Assign_Cache_Field(JAL_HSE_HOP_REG, JAL_HOP,
            _LSLChop_Execute_HSE_Connection_79(p_hs_kernel, clk) );
    }
#else
    mHWreg_Assign_Cache_Field(JAL_HSE_HOP_REG, JAL_HOP,
        _LSLChop_Execute_HSE_Connection_79(p_hs_kernel, clk) );
#endif
    mHWreg_Store_Cache_Register(JAL_HSE_HOP_REG);
#endif

    mHWreg_Load_Cache_Register(JAL_HSE_HOP_REG);

    hw_frequency = mHWreg_Get_Cache_Field(JAL_HSE_HOP_REG, JAL_HOP);
    return hw_frequency;
}

#else /*PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0*/

/*
 * V1.2 Tabasco Hop Selection Engine Registers
 *
 *  Addr       b7     b6     b5     b4     b3     b2     b1     b0
 *  CONTROL +------+------+------+------+------+------+------+------+
 *  D0      |    HSE_BT_CLK[27:0]                                   |
 *          +------+------+------+------+------+------+------+------+
 *  D4      |    HSE_UAP_LAP[23:0]                                  |
 *          +------+------+------+------+------+------+------+------+
 *  D8      |    HSE_SUM[23:0]                                      |
 *          +------+------+------+------+------+------+------+------+
 *  DC      |    HSE_RF_CHAN_INDEX[6:0]                             |
 *          +------+------+------+------+------+------+------+------+
 */

/************************************************************************
 * _LSLChop_Execute_HSE_Connection_Hardware
 *
 * Hop Selection Kernel using V1.2 based hardware hop selection engine 
 *
 * p_hs_kernel              Ignored 
 * clk                      clock in ticks for frequency
 ************************************************************************/
static t_freq _LSLChop_Execute_HSE_Connection_Hardware(
    t_hs_kernel *p_hs_kernel, t_clock clk)
{
    t_freq hw_frequency;

    mHWreg_Assign_Register(JAL_HSE_UAP_LAP_REG, p_hs_kernel->uap_lap);
    mHWreg_Assign_Register(JAL_HSE_BT_CLK_REG, clk);

#if (BUILD_TYPE==UNIT_TEST_BUILD)
    /*
     * Support emulated generation during regression testing
     * No hardware then just force frequency into JAL_HSE_HOP_REG register
     */
    mHWreg_Assign_Register(JAL_HSE_RF_CHAN_INDEX_REG, 
            _LSLChop_Execute_HSE_Connection_79(p_hs_kernel, clk) );
    mHWreg_Assign_Register(JAL_HSE_SUM_REG, 
            p_hs_kernel->sum_before_mod );
#endif

    hw_frequency = (t_freq) mHWreg_Get_Register(JAL_HSE_RF_CHAN_INDEX_REG);

    /*
     * Store the summation for used in AFH if necessary.
     */
    p_hs_kernel->sum_before_mod = mHWreg_Get_Register(JAL_HSE_SUM_REG);;


    return hw_frequency;
}
#endif

#endif

#if (PRH_BS_DEV_EARLY_PREPARE_PAGE_ID_FHS_TURNAROUND==1)
t_freq LSLChop_Get_Frequency_FirstFHS_MasterPageResponse(t_clock base_clk)
{
    t_hs_engine *p_hs_engine = &LSLChop_hs_engine; 
    t_clock last_freq_clk;
    t_freq frequency;
    u_int  X;

    p_hs_engine->frozen_clk_page_inq = base_clk;
    p_hs_engine->frozen_X_page = _LSLChop_Get_X_Page_Inquiry(base_clk);

    last_freq_clk = ((base_clk+4) & 0xFFFFFFC);

    switch(p_hs_engine->rfSelection)
    {
    case EUROPE_USA_FREQ:
        X = (p_hs_engine->frozen_X_page +
        ((last_freq_clk - p_hs_engine->frozen_clk_page_inq + 3)>>2)  ) & 0x1F;
        frequency = _Page_Inquiry_Frequency_Table[X];
        p_hs_engine->X = X;
        break;
    case FRANCE_FREQ:
        X = (p_hs_engine->frozen_X_page +
        ((last_freq_clk - p_hs_engine->frozen_clk_page_inq + 3)>>2)  ) & 0x0F;
        frequency = _Page_Inquiry_Frequency_Table[X];
        p_hs_engine->X = X;
        break;
    case SINGLE_FREQ:
        frequency = p_hs_engine->txMasterHopFixedFreq;
        break;
    case REDUCED_FREQ:
        frequency = ( (last_freq_clk>>1)%5 )*23;
        if (frequency==92) frequency++;
        break;
    }

    return frequency;
}
#endif

