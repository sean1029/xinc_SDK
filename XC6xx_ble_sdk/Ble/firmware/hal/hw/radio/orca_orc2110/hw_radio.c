 /*************************************************************************
 *
 * MODULE NAME:    hw_radio.c
 * PROJECT CODE:   XpertBlue
 * DESCRIPTION:    ORC2110-12 radio driver for Tabasco.
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  04.07.12
 *
 * SOURCE CONTROL: $Id: hw_radio.c,v 1.15 2013/08/15 15:32:31 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2012 Ceva Inc.
 *     All rights reserved.
 *
 * RECENT REVISION HISTORY:
 *
 *  04.07.12 TK    initial reference version, untested.
 *  11.10.12 TK    first working version - verified for BR/EDR rates.
 *  20.11.12 TK    added support for BT LE (untested).
 *  22.11.12 TK    enabled support for HW window widening.
 *
 ********************************************************************************/
#include "sys_config.h"
#include "sys_types.h"
#include "hw_radio.h"
#include "hw_radio_defs.h"
#include "hw_register.h"
#include "hw_hab_defs.h"
#include "hw_habanero.h" 
#include "hw_habanero_impl.h"
#include "hw_lc.h" 
#include "lslc_access.h"
#include "uslc_chan_ctrl.h"
#include "hw_radio_defs.h"
#include "sys_mmi.h"
#include "hw_leds.h"
#include "hw_delay.h"

#include "hw_pta.h"
#include "hw_macro_defs.h"
#include "rfc.h"
#include "sys_debug_config.h"
#include <math.h>
#include "patch_function_id.h"
#include "bsp_register_macro.h"
/*****************NOTED!!******************
If not use ORCA RF, keep GIO8 OverrideLow.  
****************NOTED!!*******************/
#define OUR_RADIO

#define USE_TIMED_TX_POWER_WRITE 		(1 && (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1))
#define RECOVER_TO_STANDBY  			1
#define ENABLE_SYMBOL_RECOVERY_BLOCK	0   /* #2921 */
#define TX_REPREPARE_ON_PKD_SUPPORTED 	0
#define WAIT_ON_READ_DATA 				1

#define valid_frequency(FRAME_POS,MASK) ((MASK & RADIO_FREQ_MASK(FRAME_POS)) != 0)

/********************************************************************************
 *      Local Variables
 ********************************************************************************/
extern t_RadioMode CurrentRadioMode ;
 
#if (TX_REPREPARE_ON_PKD_SUPPORTED==1)
extern u_int8 tx_prepared_on_tim2 ;
extern u_int8 tx_reprepare_on_pkd ;
#endif

extern u_int32 jal_le_eser_time_config_word ;
extern u_int8 LE_Active_Mode ;

extern uint16_t reg_i_idac_rf ;
extern uint16_t reg_q_qdac_rf ;

extern u_int8 le_mode;
extern mHWreg_Create_Cache_Register(JAL_SER_ESER_CTRL_REG);
/********************************************************************************
 *
 * T1 Hardware SER_MASK Configuration Limitations:
 *
 * To avoid glitches in ESER block, the SER_MASK is double-registered to the
 * ct_1m_clk in ESER block. So when write a new SER_MASK, must wait at least 3us
 * to read back, and the new SER_MASK also takes effect 3us later after writing
 * the new SER_MASK. As all other ESER CTRL signals share the same register map
 * location, writing to any of these signals, must wait for 3us to write new
 * SER_MASK, otherwise the new SER_MASK is not written in.
 *
 * To avoid potential scenarios of SER_MASK not written in, update register to
 * cache, and write to hardware once per radio service call only.
 *
 * As cached method is also MIPS improvement, adopt for all Hardware revisions.
 *
 ********************************************************************************/

//static mHWreg_Create_Cache_Register(JAL_SER_ESER_CTRL_REG) = 0;

#define mHWreg_Logical_OR_With_Cache_Register(Register, Value) \
       ( Cache_##Register |= Value )

#define mHWreg_Logical_AND_With_Cache_Register(Register, Value) \
       ( Cache_##Register &= Value )


/*************************************************************************************
 *  Prototypes and externs
 *************************************************************************************/
void _HWhab_Config_PHY(void);
void _HWhab_Config_Intr_Ctrl(void);
void _HWhab_Config_SYM(void);
void _HWhab_Config_COR(void);
void _HWhab_WinExt_Adjust_En(void);
void _HWhab_WinExt_Adjust_Dis(void);
void _HWhab_Radio_Setup_Tx_SPI_Times(void);
void _HWhab_Radio_Setup_Rx_SPI_Times(void);
void _HWhab_Init_RF(void);
void HWradio_Initialise(void);
void HWradio_Reset(void);
void HWradio_Set_Syncword(t_syncword const syncword);
static void _HWradio_SetGios_ResetCombines(void);
static void _HWradio_SetGios_FullRx(void);
static void _HWradio_SetGios_Standby(void);
void HWradio_DisableAllSpiWrites(void);
void _HWradio_ProgNow_xinc(u_int32 in_val);
u_int32 _HWradio_ReadNow_xinc(u_int32 Reg_To_Read);
void config_radio_tx(unsigned	int	io_Channel);

/********************************************************************************
 *
 * JAL_SER_ESER_DATA:
 * POSITION1 | 0:RESERVED(NOW)  4:tx mid freq   8:rx start freq   12:rx mid freq
 * POSITION2 | 1:tx start freq  5:              9:                13:idle
 * POSITION3 | 2:tx power       6:              10:               14:
 * POSITION4 | 3:               7:              11:               15:
 *
 ********************************************************************************/

#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)

#define Set_LE_Timed_RX_START_Receive_Command(channel_nb) \
{\
}

#define Set_LE_Timed_TX_START_Transmit_Command(channel_nb) \
{\
}

#define Send_LE_Transmit_Command_Now(channel_nb) \
{\
}

#define Send_LE_Receive_Command_Now(channel_nb) \
{\
}
#endif


/***************************************************************************
 *
 * FUNCTION: HWradio_Assign_TX_START_Channel
 *
 * Early assign (or late overwrite) useful on MIPS compromised scenarios.
 *
 ***************************************************************************/
void SER_WRITE(unsigned int regAddr,unsigned int regValue)
{
#if (__DEBUG_IO_ENABLE_XINC == 1)
    __DEBUG_IO_0_HIGH_XINC();

	_HWradio_ProgNow_xinc(0x80000000|regAddr<<16|regValue);
	__DEBUG_IO_0_LOW_XINC();
#else
    _HWradio_ProgNow_xinc(0x80000000|regAddr<<16|regValue);

#endif
	
}
unsigned int SER_READ(unsigned int regAddr,unsigned int *regValue)
{
#if (__DEBUG_IO_ENABLE_XINC == 1)
    __DEBUG_IO_0_HIGH_XINC();

	*regValue=_HWradio_ReadNow_xinc(regAddr);
	__DEBUG_IO_0_LOW_XINC();	
#else
    *regValue=_HWradio_ReadNow_xinc(regAddr);
#endif
    return 0;
}

#if		(__DEBUG_TX_ALIGN == 1)
unsigned	int	sample_i[8] = 0;
unsigned	int	sample_q[8] = 0;
#endif


typedef struct
{
	uint32_t DCO_EST_Q:10;
	uint32_t RESERVED0:2;
	uint32_t DCO_EST_I:10;	
	uint32_t RESERVED1:2;
	uint32_t DCO_VALID:1;
  uint32_t RESERVED3:7;
}BTPHY_DC_STAT_T;

BTPHY_DC_STAT_T* dc_stat_t = (BTPHY_DC_STAT_T*)0x4002C31C; //?????????

#define setbit(x,y) ((x) |= (1<<(y)))
#define clrbit(x,y) ((x) &= ~(1<<(y)))

#define  RX_HIGH()   \
 do                    \
 {                      \
  *((u_int32 volatile*)0x4002c270) |= (u_int32)(0x1<<24);  \
  *((u_int32 volatile*)0x4002c274) = (u_int32)(0x62);   \
  *((u_int32 volatile*)0x40028030) = (u_int32)(0x80000000); 	\
*((u_int32 volatile*)0x40028044) = (u_int32)(0x8000);    \
 }while(0)


#define  RX_LOW() \
do                \
{                  \
 	*((u_int32 volatile*)0x40028030) = (u_int32)(0x0000); \
  	*((u_int32 volatile*)0x40028044) = (u_int32)(0x00000000); \
 	*((u_int32 volatile*)0x40028048) = (u_int32)(0x80000000); \
  	*((u_int32 volatile*)0x4002805c) = (u_int32)(0x8000);   \
}while(0)

uint16_t TRUE_FORM_10BIT(uint16_t val)
{	
	val ^= 0x1<<9 ;
	
	return val;
}

//dc calib time 10u 
void Calib_Delay(void)
{
	for(int i=0;i<10000;i++)
	{
		for(int j=0;j<40;j++);
	}
}

void  Dcoc_Simple_Calib(void)
{
    //soft rxen gio1/gio10
	//high
	*((u_int32 volatile*)0x4002c270) |= (u_int32)(0x1<<24);
	*((u_int32 volatile*)0x4002c274) = (u_int32)(0x62);
 	*((u_int32 volatile*)0x40028030) = (u_int32)(0x80000000);      
    *((u_int32 volatile*)0x40028044) = (u_int32)(0x8000);
    
	if(dc_stat_t->DCO_VALID == 1)
	{   
	    uint32_t i_val,q_val;
		i_val = dc_stat_t->DCO_EST_I;
		q_val = dc_stat_t->DCO_EST_Q;
		{  // 只执行一次
			static int i = 0;
			i++; 
			if(i == 1)
			{
				Dcoc_Calib(i_val,q_val);
			}
		}
	}

	//low
 	*((u_int32 volatile*)0x40028030) = (u_int32)(0x0000);      
  	*((u_int32 volatile*)0x40028044) = (u_int32)(0x00000000);
 	*((u_int32 volatile*)0x40028048) = (u_int32)(0x80000000);      
  	*((u_int32 volatile*)0x4002805c) = (u_int32)(0x8000);
	//*((u_int32 volatile*)0x4002c274) = (u_int32)(0x42); 


    
	*((u_int32 volatile*)0x4002c270) |= (u_int32)(0x1<<24);
	*((u_int32 volatile*)0x4002c274) = (u_int32)(0x62);
 	*((u_int32 volatile*)0x40028030) = (u_int32)(0x80000000);      
    *((u_int32 volatile*)0x40028044) = (u_int32)(0x8000);

}

void Dcoc_Calib_Dcoc_I(void) //Calibrate I first
{
	uint16_t reg_i_idac = 0;

	uint16_t i_amg_reg = 0;
	uint16_t reg_i_idac_ref = 0;
	
	uint16_t i_amg_reg2 = 0;
	uint16_t reg_i_idac_ref2 = 0;

	uint16_t n = 0;	
	uint16_t m = 6;
	
	RX_HIGH();
	Calib_Delay();
	
	i_amg_reg = dc_stat_t->DCO_EST_I;
	if(TRUE_FORM_10BIT(i_amg_reg) > 512)
	{
		reg_i_idac_ref = 64;
        
	}
	else
	{
		reg_i_idac_ref = 0;
	}
	
	reg_i_idac = reg_i_idac_ref;
	fpga_spi_write(0x0098,0x80 | reg_i_idac);

	RX_LOW();
	for(int i=5;i>=0;i--)
	{
		RX_HIGH();
		Calib_Delay();		     
		if(TRUE_FORM_10BIT(dc_stat_t->DCO_EST_I) < 512)
		{
			n = 1;		
			i_amg_reg = dc_stat_t->DCO_EST_I;	 //read
			reg_i_idac_ref = reg_i_idac;
			if(i == 0)
			{
				break;
			}			
			reg_i_idac  = reg_i_idac_ref + pow(-1,n)*pow(2,(i));
			fpga_spi_write(0x0098,0x80 | reg_i_idac);
		}
		else
		{
			n = 0;		
			i_amg_reg2 = dc_stat_t->DCO_EST_I;	 //read
			reg_i_idac_ref2 = reg_i_idac;
			if(i == 0)
			{
				break;
			}
			reg_i_idac  = reg_i_idac_ref2 + pow(-1,n)*pow(2,(i));
			fpga_spi_write(0x0098,0x80 | reg_i_idac);	
		}

		RX_LOW();
	}	
	
	RX_HIGH();
	Calib_Delay();
	if(abs(TRUE_FORM_10BIT(i_amg_reg)-512) > abs(TRUE_FORM_10BIT(i_amg_reg2)-512))
	{
		reg_i_idac_rf = reg_i_idac_ref;
	}
	else
	{
		reg_i_idac_rf = reg_i_idac_ref2;
	}	
	RX_LOW();
}	

void Dcoc_Calib_Dcoc_Q(void)  //I val must write it
{
	uint16_t reg_q_qdac = 0;

	uint16_t q_amg_reg = 0;
	uint16_t reg_q_qdac_ref = 0;
	
	uint16_t q_amg_reg2 = 0;
	uint16_t reg_q_qdac_ref2 = 0;

	uint16_t n = 0;	
	uint16_t m = 6;
	
	RX_HIGH();
	Calib_Delay();
	q_amg_reg = dc_stat_t->DCO_EST_Q;
	if(TRUE_FORM_10BIT(dc_stat_t->DCO_EST_Q) > 512)
	{
		reg_q_qdac_ref = 64;
	}
	else
	{
		reg_q_qdac_ref = 0;
	}
	
	reg_q_qdac = reg_q_qdac_ref;
	fpga_spi_write(0x0098,reg_q_qdac<<8 | (0x80 | reg_i_idac_rf));
	RX_LOW();
	
	for(int i=5;i>=0;i--)
	{
		RX_HIGH();
		Calib_Delay();		
		if(TRUE_FORM_10BIT(dc_stat_t->DCO_EST_Q) < 512)
		{
			n = 1;		
			q_amg_reg = dc_stat_t->DCO_EST_Q;	 
			reg_q_qdac_ref = reg_q_qdac;			
			if(i == 0)
			{
				break;
			}
			reg_q_qdac  = reg_q_qdac_ref + pow(-1,n)*pow(2,(i));
			fpga_spi_write(0x0098,reg_q_qdac<<8 | (0x80 | reg_i_idac_rf));
		}
		else
		{
			n = 0;		
			q_amg_reg2 = dc_stat_t->DCO_EST_Q;	
			reg_q_qdac_ref2 = reg_q_qdac;
			if(i == 0)
			{
				break;
			}
			reg_q_qdac  = reg_q_qdac_ref2 + pow(-1,n)*pow(2,(i));
			fpga_spi_write(0x0098, reg_q_qdac<<8 | (0x80 | reg_i_idac_rf));	
		}
		RX_LOW();
	}	
	
	RX_HIGH();
	Calib_Delay();
	if(abs(TRUE_FORM_10BIT(q_amg_reg)-512) > abs(TRUE_FORM_10BIT(q_amg_reg2)-512))
	{
		reg_q_qdac_rf = reg_q_qdac_ref;
	}
	else
	{
		reg_q_qdac_rf = reg_q_qdac_ref2;
	}		
	//RX_LOW();
    //RX_HIGH();
}		

void _HWradio_Agc_Debug(void)
{

    uint32_t ii;
    extern  void        config_radio_rx(unsigned    int io_Channel);
    config_radio_rx(0);

    //tx low
    *((u_int32 volatile*)0x4002c274) |= (1<<4);
    *((u_int32 volatile*)0x40028038) = (u_int32)(0x0000);      //4,7 high override
    *((u_int32 volatile*)0x4002803c) = (u_int32)(0x00000000);
    *((u_int32 volatile*)0x40028050) = (u_int32)(0x8000);      //4,7 low override
    *((u_int32 volatile*)0x40028054) = (u_int32)(0x80000000);
    //-------------------
    //agc initial  begin
    //-------------------
    *((u_int32 volatile*)0x4002c294) = 0x0; //16 agc_mode_sw 7:0 sw_cfg_rf_lna_gain
    *((u_int32 volatile*)0x4002c298) =
                                    (0<<0 | //0 enable
                                    1<<1 | //1 hw_agc_sel
                                    0<<16| //16 agc_adc_en_sw       
                                    0<<17| //17 hw_agc_ctrl_enable  
                                    0<<18| //18 cfg_cci_gain_mode   
                                    7<<20| //23:20cfg_LNA_init      
                                    9<<24); //27:24cfg_ABB_init     
    *((u_int32 volatile*)0x4002c298) |= 0x1<<17; //17 hw_agc_ctrl_enable
    //*((u_int32 volatile*)0x4002c298) |= 0x1;     //0 enable              

    *((u_int32 volatile*)0x4002c29c) = 0x05 ;//20:16,14:0 cfg_LNAPowDetTHLinear
    *((u_int32 volatile*)0x4002c2a0) = 0x1f4;//24:16,14:0 cfg_ABBPowDetTHLinear
    *((u_int32 volatile*)0x4002c2a4) = 20;//24:16,14:0 cfg_turner_gain
    *((u_int32 volatile*)0x4002c2d8) = 24<<24| 18<<16| 12<<8| 0;// hw_cfg_rf_lna_gain0
    *((u_int32 volatile*)0x4002c2dc) = 48<<24| 42<<16| 36<<8| 30;// hw_cfg_rf_lna_gain4
    *((u_int32 volatile*)0x4002c2e0) = 0x4 <<24| 0x2 <<16| 0x0 <<8| 0x0 ;// sw_cfg_rf_abb_gain
    *((u_int32 volatile*)0x4002c2e4) = 0xC <<24| 0xA <<16| 0x8 <<8| 0x6 ;// hw_cfg_rf_abb_gain3
    *((u_int32 volatile*)0x4002c2e8) = 0x14<<24| 0x12<<16| 0x10<<8| 0xE ;// hw_cfg_rf_abb_gain7
    *((u_int32 volatile*)0x4002c2ec) = 0x1C<<24| 0x1A<<16| 0x18<<8| 0x16;// hw_cfg_rf_abb_gain11
    *((u_int32 volatile*)0x4002c2f0) = 0x1E;// hw_cfg_rf_abb_gain15    
    //-------------------
    //agc initial  end
    //-------------------    

    while(1) {
        //*((u_int32 volatile*)0x4002c298) |= 0x1;     //0 enable 
        //for(ii=0;ii<10000;ii++);
        //*((u_int32 volatile*)0x4002c298) &= 0xFFFFFFFE;     //0 enable 
        
        //*((u_int32 volatile*)0x4002c298) |= 0x1;     //0 enable 
        //soft rxen gio1/gio10
        //high
        *((u_int32 volatile*)0x4002c270) |= (u_int32)(0x1<<24);
        *((u_int32 volatile*)0x4002c274) = (u_int32)(0x7A);        
        *((u_int32 volatile*)0x40028048) = (u_int32)(0x00000000);  //low overide 
        *((u_int32 volatile*)0x4002805c) = (u_int32)(0x0000);
        *((u_int32 volatile*)0x40028030) = (u_int32)(0x80000000);  //high overide    
        *((u_int32 volatile*)0x40028044) = (u_int32)(0x8000);
        
        for(ii=0;ii<3000;ii++);
        
        //*((u_int32 volatile*)0x4002c298) &= 0xFFFFFFFE;     //0 enable 
        //low
        *((u_int32 volatile*)0x4002c274) = (u_int32)(0x72);
        *((u_int32 volatile*)0x40028030) = (u_int32)(0x0000);      
        *((u_int32 volatile*)0x40028044) = (u_int32)(0x00000000);
        *((u_int32 volatile*)0x40028048) = (u_int32)(0x80000000);      
        *((u_int32 volatile*)0x4002805c) = (u_int32)(0x8000);        
        //*((u_int32 volatile*)0x4002c274) = (u_int32)(0x72);
        
    }
}

/***************************************************************************
 *
 * FUNCTION: _HWradio_Go_To_Idle_State    
 *
 ***************************************************************************/
void _HWradio_Go_To_Idle_State(void)
{
 	*((u_int32 volatile*)0x4002c240) = (BTPHY_CTL0_NO_SHDN)|(0x80000000);
	
	#if	(__DEBUG_XINC_FPGA == 1)
	{
		*((u_int32 volatile*)0x4002c244) = (u_int32)(0x1e600000);//0.56
		*((u_int32 volatile*)0x4002c244) |= 0x08;	   // rx down sample enable 
		//*((u_int32 volatile*)0x4002c244) |= 0x04;	   // tx up sample enable 
		//*((u_int32 volatile*)0x4002c244) |= 0x10;
		*((u_int32 volatile*)0x4002c244) |= 0x40;
		*((u_int32 volatile*)0x4002c244) |= 0x100;	//- DAC unsigned.
		*((u_int32 volatile*)0x4002c244) |= 0x200;//adc signed
		
		*((u_int32 volatile*)0x4002c274) = (u_int32)(0x0000042); //down sample 1/2  +  iq clk generate by RF 
		*((u_int32 volatile*)0x4002c264) = (u_int32)(0x730255);  //threshold 2=>4   7=>1
	
		// *((u_int32 volatile*)0x4002c288) = 0x0017;  //0x0010
	    // *((u_int32 volatile*)0x4002c28C) = 0x0011;
	    // *((u_int32 volatile*)0x4002c290) = 0x0011;	
		
		*((u_int32 volatile*)0x4002c288) = 0x0000;  //0x0010
	    *((u_int32 volatile*)0x4002c28C) = 0x0000;
	    *((u_int32 volatile*)0x4002c290) = 0x0000;		
	}
	#else
    	*((u_int32 volatile*)0x4002c244) = (u_int32)(0x20410200);//0.52  	

		*((u_int32 volatile*)0x4002c274) = (u_int32)(0x42);
		*((u_int32 volatile*)0x4002c264) = (u_int32)(0x730255);
	#endif
	//*((u_int32 volatile*)0x4002c264) = (u_int32)(0x1037f43);
	#if	(__DEBUG_XINC_FPGA == 1)
	*((u_int32 volatile*)0x4002c268) = (u_int32)(0x820ff04);
	#else
	*((u_int32 volatile*)0x4002c268) = (u_int32)(0x820ff04);
	#endif
	*((u_int32 volatile*)0x4002c26c) = (u_int32)(0x820ff04);
	#if	(__DEBUG_XINC_FPGA == 1)
	*((u_int32 volatile*)0x4002c258) = (u_int32)(0x780);//sync threshold
	#else
	*((u_int32 volatile*)0x4002c258) = (u_int32)(0x705);
	#endif
	*((u_int32 volatile*)0x4002c25c) = (u_int32)(0x50);
	//*((u_int32 volatile*)0x4002c294) = (u_int32)(0x20000000);	

   //       *((u_int32 volatile*)0x4002c294) = (u_int32)(0x9801c18);	// hard pin controld -70    lna       filter
   #if	(__DEBUG_XINC_FPGA == 1)
	*((u_int32 volatile*)0x4002c294) = (u_int32)(0x20001c0b);	//TX_BB_GAIN DEBUG !!!!!!!!
	//*((u_int32 volatile*)0x40002430) = (u_int32)(0x3f3f020);	//ctune  DEBUG !!!!!!!!!
	_Ctune_Set();
	#else
	*((u_int32 volatile*)0x4002c294) = (u_int32)(0x9801c33);	//softgain
	#endif
	//*((u_int32 volatile*)0x4002c294) = (u_int32)(0x9800fb1);	//hard pin controld  -10   
	//*((u_int32 volatile*)0x4002c294) = (u_int32)(0x9810fb1);	//-10   OK  160705
       //*((u_int32 volatile*)0x4002c294) = (u_int32)(0x9810fb7);	//-30   dbg  160713
 	//*((u_int32 volatile*)0x4002c294) = (u_int32)(0xD511c08);	//-90  OK  160705   1.4 times Modem Tx
   	#if	(__DEBUG_XINC_FPGA == 1)
	*((u_int32 volatile*)0x4002c270) = (u_int32)(0x30000000);	//0403 18:11\
	//*((u_int32 volatile*)0x4002c270) |=  (1<<25);
	*((u_int32 volatile*)0x4002c278) = (u_int32)(0xa8c784);	//rxen ahead
	#else
	*((u_int32 volatile*)0x4002c270) = (u_int32)(0x34000000);	//0403 18:11
	*((u_int32 volatile*)0x4002c278) = (u_int32)(0xa8c582);	//rxen ahead
	#endif
	
	
 	*((u_int32 volatile*)0x4002c298) = (u_int32)(0x1c351c35);	
	*((u_int32 volatile*)0x4002c29c) = (u_int32)(0x1c021c00);
	*((u_int32 volatile*)0x4002c2a0) = (u_int32)(0x1c061c04);
	*((u_int32 volatile*)0x4002c2a4) = (u_int32)(0x1c0a1c08);
	*((u_int32 volatile*)0x4002c2a8) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2ac) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2b0) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2b4) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2b8) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2bc) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2c0) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2c4) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2c8) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2cc) = (u_int32)(0x1c0a1c0a);	
	*((u_int32 volatile*)0x4002c2d0) = (u_int32)(0x1c0a1c0a);
	*((u_int32 volatile*)0x4002c2d4) = (u_int32)(0x1c0a1c0a);

	*((u_int32 volatile*)0x4002c2d8) = (u_int32)(0x08040201);	
	*((u_int32 volatile*)0x4002c2dc) = (u_int32)(0x80402010);
	*((u_int32 volatile*)0x4002c2e0) = (u_int32)(0xffffffff);
	*((u_int32 volatile*)0x4002c2e4) = (u_int32)(0xffffffff);
	*((u_int32 volatile*)0x4002c2e8) = (u_int32)(0xffffffff);
	*((u_int32 volatile*)0x4002c2ec) = (u_int32)(0xffffffff);
	*((u_int32 volatile*)0x4002c2f0) = (u_int32)(0xffffffff);
	*((u_int32 volatile*)0x4002c2f4) = (u_int32)(0xffffffff);
#if 0
  *((u_int32 volatile*)0x4002c298) =
                                    (0<<0 | //0 enable
                                    1<<1 | //1 hw_agc_sel
                                    0<<16| //16 agc_adc_en_sw       
                                    0<<17| //17 hw_agc_ctrl_enable  
                                    0<<18| //18 cfg_cci_gain_mode   
                                    7<<20| //23:20cfg_LNA_init      
                                    9<<24); //27:24cfg_ABB_init     
    *((u_int32 volatile*)0x4002c298) |= 0x1<<17; //17 hw_agc_ctrl_enable  
    *((u_int32 volatile*)0x4002c294) = 0x20010000; //16 agc_mode_sw 7:0 sw_cfg_rf_lna_gain
#endif
#if 1
//-------------------
    //agc initial  begin
    //-------------------
    //*((u_int32 volatile*)0x4002c294) = 0x20010000; //16 agc_mode_sw 7:0 sw_cfg_rf_lna_gain
    *((u_int32 volatile*)0x4002c298) =
                                    (0<<0 | //0 enable
                                    1<<1 | //1 hw_agc_sel
                                    0<<16| //16 agc_adc_en_sw       
                                    0<<17| //17 hw_agc_ctrl_enable  
                                    0<<18| //18 cfg_cci_gain_mode   
                                    7<<20| //23:20cfg_LNA_init      
                                    9<<24); //27:24cfg_ABB_init     
    *((u_int32 volatile*)0x4002c298) |= 0x1<<17; //17 hw_agc_ctrl_enable
    //*((u_int32 volatile*)0x4002c298) |= 0x1;     //0 enable              
    
    *((u_int32 volatile*)0x4002c29c) = 0x05 ;//20:16,14:0 cfg_LNAPowDetTHLinear
    *((u_int32 volatile*)0x4002c2a0) = 0x1f4;//24:16,14:0 cfg_ABBPowDetTHLinear
    *((u_int32 volatile*)0x4002c2a4) = 26;//24:16,14:0 cfg_turner_gain
    *((u_int32 volatile*)0x4002c2d8) = 24<<24| 18<<16| 12<<8| 0;// hw_cfg_rf_lna_gain0
    *((u_int32 volatile*)0x4002c2dc) = 48<<24| 42<<16| 36<<8| 30;// hw_cfg_rf_lna_gain4
    *((u_int32 volatile*)0x4002c2e0) = 0x4 <<24| 0x2 <<16| 0x0 <<8| 0x0 ;// sw_cfg_rf_abb_gain
    *((u_int32 volatile*)0x4002c2e4) = 0xC <<24| 0xA <<16| 0x8 <<8| 0x6 ;// hw_cfg_rf_abb_gain3
    *((u_int32 volatile*)0x4002c2e8) = 0x14<<24| 0x12<<16| 0x10<<8| 0xE ;// hw_cfg_rf_abb_gain7
    *((u_int32 volatile*)0x4002c2ec) = 0x1C<<24| 0x1A<<16| 0x18<<8| 0x16;// hw_cfg_rf_abb_gain11
    *((u_int32 volatile*)0x4002c2f0) = 0x1E;// hw_cfg_rf_abb_gain15    
    //-------------------
    //agc initial  end
    //-------------------    
#endif
    
	*((u_int32 volatile*)0x40000130) |= 0x01;	
	*((u_int32 volatile*)0x40002450) |= 0x01;	//cpr_ao_corerfldo_en  open rf digital
//	*((u_int32 volatile*)0x4002F840) |= (0x01<<6);	//RG_RCCAL_SELSPI
	rf_init();
	
	#if(__DEBUG_RF_LOWPOWER_ENABLE == 1)
	SER_WRITE(0x10, 0x3f);		// - ldo off
	#endif

    #if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)     
    __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_OFF);
    #endif
	#if __DEBUG_XINCHIP_RF_LOWPOWER_ENABLE	
    if ( 1 == g_debug_xinchip_rf_lowpower_enable)
    {
    	ldo_off_debug();
    //    __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_OFF);
    }
	#endif
    
    //#define __DEBUG_HW_AGC 0
    #if (__DEBUG_HW_AGC==1)
    _HWradio_Agc_Debug();
	#endif
    
	#if		(__DEBUG_CW_WAVE == 1)
	
	config_radio_tx(0);
	{
        uint32_t    v_test[3];        
        SER_READ(REG_CTL_0x3c, &v_test[0]);
        SER_READ(REG_CTL_0x15c, &v_test[1]);	
        SER_READ(REG_CTL_0xbfc, &v_test[2]);
	
    }        
 	*((u_int32 volatile*)0x4002c274) |= (1<<4);
 	*((u_int32 volatile*)0x40028038) = (u_int32)(0x8000);      
  	*((u_int32 volatile*)0x4002803c) = (u_int32)(0x80000000);
	while(1);
 	#endif

	#if		(__DEBUG_TX_ALIGN == 1)
	
	*((u_int32 volatile*)0x4002c274) |= (1<<4);
 	*((u_int32 volatile*)0x40028038) = (u_int32)(0x8000);      
  	*((u_int32 volatile*)0x4002803c) = (u_int32)(0x80000000);
	
	*((u_int32 volatile*)0x4002c274) |= (1<<2);

	//+	TX-delay
	SER_WRITE(0x23 , 0x6600);	
	SER_WRITE(0x24 , 0x6666);
	SER_WRITE(0x25 , 0x6666);


	//+ Enable buffer.
	SER_READ(0x20, &sample_i[0]);
	sample_i[0] &= ~(1<<6);
	SER_WRITE(0x20 , sample_i[0]);
	
	SER_READ(0x20, &sample_i[0]);
	sample_i[0] |= (1<<6);

	//sample_i[0] |= (1<<4);
	SER_WRITE(0x20 , sample_i[0]);
	
	for(unsigned	int	j=0; j<0xffff; j++);

	for(unsigned	int	i=0; i<8; i++)
	{		
		SER_READ((0x405+i), &sample_i[i]);
		SER_READ((0x40d+i), &sample_q[i]);		
	}

	while(1);
	#endif
//	_HWradio_Go_To_Idle_State_Patch();//ía2??￡?é?¨ò?
}

/***************************************************************************
 *
 * FUNCTION:    _HWhab_Config_PHY
 * PURPOSE: 
 *
 ***************************************************************************/
void _HWhab_Config_PHY(void)
{
  //u_int32 hab_phy_cfg = HWhab_PHY_CFG;
    /*clk divider settings should not be re-configured by radio driver*/ 
  //HWhab_Set_PHY_Cfg((hab_phy_cfg&0xFFFFFFE0)|(HWhab_Get_PHY_Cfg()&0x1F));
  HWhab_Set_PHY_Cfg(HWhab_PHY_CFG);
}

/***************************************************************************
 *
 * FUNCTION:    _HWhab_Config_Intr_Ctrl
 * PURPOSE:     Configure the Interrupts
 *
 ***************************************************************************/
void _HWhab_Config_Intr_Ctrl(void)
{
    /* mask all interrupts */
    HWhab_Set_RIF_Intr_Mask0(0x3);
    HWhab_Set_RIF_Intr_Mask1(0x3);
    HWhab_Set_RIF_Intr_Mask2(0x3);
    HWhab_Set_RIF_Intr_Mask3(0x3);
    HWhab_Set_LTR_Intr_Mask(1);

    /* clear all interrupts */
    HWhab_Set_RIF_Intr_Clr(0xF);
    HWhab_Set_LTR_Intr_Clr(1);
}

/***************************************************************************
 *
 * FUNCTION:    _HWhab_Config_SYM
 * PURPOSE:     Configure the Symbol/Clock Recovery Block
 *
 ***************************************************************************/
void _HWhab_Config_SYM(void)
{
#if (ENABLE_SYMBOL_RECOVERY_BLOCK==1)
/* Recommended the settings below to improve
 * data transfer rate against some other interoperable devices
 */
	HWhab_Set_Sym_Gn(0x2);
	HWhab_Set_Sym_Enb(1);
#else
/* SYM_GN = 7 and SYM_ENB = 0 are default settings
 * but this does not consider phase error for data from radio to baseband
 */ 
	HWhab_Set_Sym_Gn(0x7);
	HWhab_Set_Sym_Enb(0);
#endif
  HWhab_Set_Sym_Mask(0x1);
}

/***************************************************************************
 *
 * FUNCTION:    _HWhab_Config_COR
 * PURPOSE:     Configure the Correlator
 *
 ***************************************************************************/
void _HWhab_Config_COR(void)
{
#if(__DEBUG_ACCESS_CODE_FOR_BQT == 0)
    HWhab_Set_Sync_Error(0x7);
#else
	HWhab_Set_Sync_Error(0x0);

#endif
}					

/***********************************************************************
 *
 * FUNCTION:    _HWhab_Init_RF
 * PURPOSE:     Configure the SPI and GIO blocks. 
 *
 *
 *
 ************************************************************************/
void _HWhab_Init_RF(void)
{
    volatile u_int32 config_word = 0; 

	/* configure the RF Interface to BlueRF - also clock divider */
#if (SYS_HAL_USE_RADIO_CLOCK_AS_TAB_CLOCK==1)
	mHWreg_Assign_Register(HAB_RF_MUX_CLK_DIV_REG, 0x00004200); 
#else	
	mHWreg_Assign_Register(HAB_RF_MUX_CLK_DIV_REG, 0x00080200); 
#endif


#if (PRH_BS_DEV_WIRED_LINK==1)  //- LC add (#if #endif) for Page timeout problem.
    // TK 21/3/2014 - override TX/RX delays configured in radio driver. - 
    HW_set_tx_delay(1); //mHWradio_TX_TAB_DELAY 
    HW_set_rx_delay(5); //mHWradio_RX_TAB_DELAY
#else
	/* Set Tx and Rx delays*/
	//HW_set_tx_delay(mHWradio_TX_DELAY);  
	HW_set_tx_delay(TX_DELAY_DEFINE); //mHWradio_TX_TAB_DELAY 
	//HW_set_rx_delay(mHWradio_RX_DELAY);


	/*-----------------------------

		100:for TIM/SLA/BV-04C. RX_DELAY_DEFINE(64):for nomal WORK.

	----------------------------*/

	HW_set_rx_delay(RX_DELAY_DEFINE);
	
#endif
    
	//config_word = 9; 		   /* There are nine address bits in DBUS data. */ //del by gxf 2014-12-22 15:43:00
	config_word = 16; 		   /* There are nine address bits in DBUS data. */	//del by gxf 2014-12-22 15:43:00
    //config_word |= (25 << 5);  /* 9 address plus 16 data gives pkt len of 25 */
    config_word |= (31 << 5);  /* 9 address plus 16 data gives pkt len of 25 *///del by gxf  2014-12-23 10:10:21
    //config_word |= (1<< 10);  /* clk pol - 1. data clocked out on rising edge.*/
    config_word |= (0<< 10);  /* clk pol - 1. data clocked out on rising edge.*/
    config_word |= (0 << 11);  /* data pol - 0  */
    config_word |= (1 << 12);  /* serial enable - 1 */
    config_word |= (1<< 16);  /* clk low  - 1  Number of ref clk cycles for which SPI clk is Low.*/
    config_word |= (1 << 20);  /* clk high - 1  Number of ref clk cycles for which SPI clk is High.*/
    config_word |= (0 << 23);  /* clk byp  - 0  */
    config_word |= (1 << 24);  /* sel pol - 1   Active Low select Enable.*/
    ////del by gxf 2014-12-22 15:43:00
    //config_word |= (1 << 27);  /* sel pol - 1   DBUS Enable. See special DBUS requirements in Blue RF Radio Spec.*/
    config_word |= (0 << 27);  /* sel pol - 1   DBUS Enable. See special DBUS requirements in Blue RF Radio Spec.*/
    config_word |= (1 << 29);  //add by qiliang ,mxd spi en
	config_word |= (0 << 30);  /* i/f - SPI = 0 , JTAG = 1 */
    mHWreg_Assign_Register(JAL_SER_CFG_REG, config_word); /* Set up JAL_SER_CFG register.*/

	
    config_word =  (1 << JAL_SER_ESER_CTRL_REG_JAL_SER_MAP_SEL_BIT_OFFSET);   /* enable the enhanced serial block */
    config_word |= (0 << JAL_SER_ESER_CTRL_REG_JAL_SER_MASK_BIT_OFFSET);  /* mask - 0 */
    config_word |= (1 << JAL_SER_ESER_CTRL_REG_JAL_SER_NOW_BIT_OFFSET);  /* now - 1 */
    config_word |= (0 << JAL_SER_ESER_CTRL_REG_JAL_SER_SEQ_BIT_OFFSET);  /* seq - 0 */
    mHWreg_Assign_Cache_Register(JAL_SER_ESER_CTRL_REG, config_word); /* Set up JAL serial time register.*/
    mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);
#if     0
    /* configure Timing Register */
  	RadioSetupTime = HW_RADIO_SETUP_TIME/16;
	if (HW_RADIO_SETUP_TIME%16)
		RadioSetupTime++;

	/* setup the time for the SPI writes */
    config_word = RadioSetupTime++;
	config_word |= (RadioSetupTime++ << JAL_SER_ESER_TIME_1_BIT_OFFSET);
	config_word |= (RadioSetupTime++ << JAL_SER_ESER_TIME_2_BIT_OFFSET);
	config_word |= (RadioSetupTime   << JAL_SER_ESER_TIME_3_BIT_OFFSET);

#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
	config_word |= (0xf << JAL_SER_TIME_EARLY_SHFT); // default - all SPIs write early due to win_ext
#endif

	jal_bt_eser_time_config_word = config_word;
#endif
	//jal_le_eser_time_config_word = 0x0000789; // note - value taken from rohm rf driver

	jal_le_eser_time_config_word = 0x8ace;//0x000258b; // -0x58be;//

    mHWreg_Assign_Register(JAL_SER_ESER_TIME_REG, jal_le_eser_time_config_word);

#if 0
	/* setup the data to read */
//	mHWreg_Assign_Register(JAL_SER_ESER_DATA_0,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_1,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_2,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_3,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_4,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_5,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_6,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_7,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,  READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_10, READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_11, READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_12, READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_13, READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_14, READ_REG(0x1F));
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_15, READ_REG(0x1F));
#endif
    /* GIO Combines - Un combine everything. */
    mHWreg_Assign_Register(HAB_GIO_CTRL_COMB_3_2_1_0_REG, 0x00000000);
    mHWreg_Assign_Register(HAB_GIO_CTRL_COMB_7_6_5_4_REG, 0x00000000);
    mHWreg_Assign_Register(HAB_GIO_CTRL_COMB_B_A_9_8_REG, 0x00000000);
	mSetHWEntry(HAB_GIO_COMBINE_14, 0);
	mSetHWEntry(HAB_GIO_COMBINE_15, 0);
	
    /* GIO Highs */
    mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_DATA_DIR               , 0x00000000);
    mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_PDN	                , 0x00000000); 
    mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_RESETN		            , 0x00000000); 
    mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_TX_SYNC_2	            , 0x00000000); 
    mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_BPKTCTL_AND_TX_SYNC_1	, 0x00000000); 
    mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_BPKTCTL_2              , 0x00000000); 


	mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_TX_EN					, 0x00000000); 
	mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_RX_EN					, 0x00000000);
    /* GIO Lows */
    mHWreg_Assign_Register(HW_RADIO_GIO_LOW_DATA_DIR		        , 0x80008000); 
    mHWreg_Assign_Register(HW_RADIO_GIO_LOW_TX_SYNC_2		        , 0x80000000); 
    mHWreg_Assign_Register(HW_RADIO_GIO_LOW_BPKTCTL_AND_TX_SYNC_1	, 0x80008000); 
    mHWreg_Assign_Register(HW_RADIO_GIO_LOW_BPKTCTL_2	            , 0x00008000); 

	mHWreg_Assign_Register(HW_RADIO_GIO_LOW_TX_EN					, 0x80008000); 
	mHWreg_Assign_Register(HW_RADIO_GIO_LOW_RX_EN					, 0x80008000);

    mHWreg_Clear_Cache_Bit(JAL_SER_ESER_CTRL_REG, JAL_SER_NOW);
    mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);

#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
    HWhab_Set_GIOs_Hi_Early(0);
    HWhab_Set_GIOs_Hi_Late(0);
    HWhab_Set_GIOs_Lo_Early(0);
    HWhab_Set_GIOs_Lo_Late(0);

    mSetHWEntry(JAL_SER_TIME_EARLY, 0);
    mSetHWEntry(JAL_SER_TIME_LATE, 0);
#endif
}

/***************************************************************************
 *
 * FUNCTION:    _HWhab_WinExt_Adjust_En
 * PURPOSE:     Configuration of the WinExt adjustment - Enable
 *
 ***************************************************************************/
void _HWhab_WinExt_Adjust_En(void)
{
#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
	mSetHWEntry(JAL_SER_TIME_EARLY, 0xF); // enable - all SPIs write early due to win_ext

	HWhab_Set_GIOs_Hi_Early(0xFFF); // enable - all GIOs assert early due to win_ext
    HWhab_Set_GIOs_Lo_Early(BIT8|BIT6); // enable - SYNC_PULSE low early due to win_ext
#endif
}

/***************************************************************************
 *
 * FUNCTION:    _HWhab_WinExt_Adjust_Dis
 * PURPOSE:     Configuration of the WinExt adjustment - Disable
 *
 ***************************************************************************/
void _HWhab_WinExt_Adjust_Dis(void)
{
#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
	mSetHWEntry(JAL_SER_TIME_EARLY, 0); // disable - all SPIs write early due to win_ext

	HWhab_Set_GIOs_Hi_Early(0); // disable - all GIOs assert early due to win_ext
    HWhab_Set_GIOs_Lo_Early(0); // disable - SYNC_PULSE low early due to win_ext
#endif
}


/***************************************************************************
 * Radio HAL
 ***************************************************************************/

/***************************************************************************
 *
 * FUNCTION:   HWradio_Initialise
 * PURPOSE:    Initialises Habanero 
 *
 ***************************************************************************/
void HWradio_Initialise(void)
{
    _HWhab_Config_PHY();
    _HWhab_Config_SYM();
    _HWhab_Config_COR();
    _HWhab_Config_Intr_Ctrl();

    HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);

    _HWhab_Init_RF();						/* Set default register settings. */
    
    /* add (#if #endif)*/
#if ((__DEBUG_RADIO_MAC_TO_MAC__==1)||(__DEBUG_RADIO_BYPASS__==1))
    
#else
    _HWradio_Go_To_Idle_State();			/* Power Up radio and then Radio needs to start in IDLE state.*/
#endif    
    _HWradio_SetGios_Standby();

    CurrentRadioMode =RADIO_MODE_STANDBY;

#if (TX_REPREPARE_ON_PKD_SUPPORTED==1)
    tx_prepared_on_tim2 = FALSE;
    tx_reprepare_on_pkd = FALSE;
#endif
}

/***************************************************************************
 *
 * FUNCTION:   HWradio_Reset
 * PURPOSE:    Resets the Habanero core AND register map by writing special
 *             sequence to reset register
 *			   Also resets radio registers.
 *	
 *	NOTE:      Reset for Habanero implemented in core (hw_lc.c)
 ***************************************************************************/ 
void HWradio_Reset(void)
{
}

/***************************************************************************
 *
 * FUNCTION:    HWradio_Set_Syncword
 * PURPOSE:
 *
 ***************************************************************************/
void HWradio_Set_Syncword(t_syncword const syncword)
{
    HWhab_Set_Syncword_Ref((u_int32 const*)&syncword);
}

/*************************************************************************************
 *
 * FUNCTION NAME: _HWradio_ResetGioCombines
 * DESCRIPTION:   This function resets the GIO line combines that may have 
 *                been programmed.
 *
 *
 * PARAMETER:     None.
 *
 *
 * RETURNS:       None.
 *
 *
 * SPECIAL CONSIDERATIONS: None.
 *
 *
 *************************************************************************************/
static void _HWradio_SetGios_ResetCombines(void)
{
	/* GIO Combines - Un combine everything. */
	mHWreg_Assign_Register(HAB_GIO_CTRL_COMB_3_2_1_0_REG, 0x00000000);
	mHWreg_Assign_Register(HAB_GIO_CTRL_COMB_7_6_5_4_REG, 0x00000000);
	mHWreg_Assign_Register(HAB_GIO_CTRL_COMB_B_A_9_8_REG, 0x00000000);
}



#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)
/*************************************************************************************
 *
 * FUNCTION NAME: _HWradio_SetGios_LE_Testmode_Tx
 *
 * Sets up the high and low times for the GIOs in order to produce correct signals
 * for the Test Tx at the start of Frame driver mode.
 *
 *************************************************************************************/
static void _HWradio_SetGios_LE_Testmode_Tx(void)
{
	    // Write Register 1 in the RF. With the correct RXCOUNTER and SYNTCOUNTER
	    //   in BT 2.1 RXCOUNTER value is 150 and SYNTCOUNTER is 50
	    // for LE we Try RXCOUNTER = 100.

	    /*  Not needed here as DATA_DIR is over-riden low on PKA.
         *
		 * mHWreg_Assign_Register( HW_RADIO_GIO_LOW_DATA_DIR,
	     *                      hw_radio_gio_override_low_data_dir +
	     *                      HW_RADIO_SET_BDATA1_TO_RX_NORM );
	     */

	    /*************************
	     * BPKTCTRL
	     *************************/
	    /* Setup the BPKTCTRL high time for Tx and the tx BDATA1 sync pluse high time */
	    mHWreg_Assign_Register( HW_RADIO_GIO_HIGH_BPKTCTL_AND_TX_SYNC_1,
	                            HW_RADIO_SET_BPKTCTL_HIGH_PRE_TX_NORM +
	                            HW_RADIO_PRE_TX_BDATA1_PULSE_HIGH_NORM);

	    // NOTE :- The BPKTCTL goes low after the TIFS has started for the next RX.
	    // So we drive the BPKTCTL low HW_RADIO_BLUE_RF_TIME_T_PD (6us) after the end of
	    // the Packet Assembly, also need to factor in the mHWradio_TX_DELAY.
	    //
	    /* Setup the low times for BPKTCTL after TX and the low time for the Tx BDATA1 sync pulse */
#if		0	
	    mHWreg_Assign_Register( HW_RADIO_GIO_LOW_BPKTCTL_AND_TX_SYNC_1,
	    						HW_RADIO_SET_BPKTCTL_LOW_END_TX_NORM +
	                            HW_RADIO_PRE_TX_BDATA1_PULSE_LOW_NORM);

#else
		mHWreg_Assign_Register( HW_RADIO_GIO_LOW_BPKTCTL_AND_TX_SYNC_1,
						(HW_RADIO_SET_BPKTCTL_LOW_END_TX_NORM +
						HW_RADIO_PRE_TX_BDATA1_PULSE_LOW_NORM)|0x8000);

#endif
}

/*************************************************************************************
 *
 * FUNCTION NAME: _HWradio_SetGios_LE_TIFS_TxRx
 *
 * Sets up the high and low times for the GIOs in order to produce correct signals
 * for the LE Tx Rx driver mode.
 *
 *************************************************************************************/
static void _HWradio_SetGios_LE_TIFS_TxRx(void)
{
    /* setup the GIOs for the BPKTCTRL Tx and BDATA1 sync before the data tx. */

    /*************************
     * BPKTCTL
     *************************/
    /* Setup the BPKTCTL high time for Tx and the tx BDATA1 sync pulse high time */
    mHWreg_Assign_Register( HW_RADIO_GIO_HIGH_BPKTCTL_AND_TX_SYNC_1,
							HW_RADIO_LE_SET_BPKTCTL_HIGH_PRE_TX_TIFS +
							HW_RADIO_LE_PRE_TX_BDATA1_PULSE_HIGH_TIFS);

    // NOTE :- The BPKTCTL goes low after the TIFS has started for the next RX.
    // So we drive the BPKTCTL low HW_RADIO_TXEN_HOLD_TIME (1us) after the end of
    // the Packet Assembly, also need to factor in the mHWradio_TX_DELAY.

    /* Setup the low times for BPKTCTL after TX and the low time for the Tx BDATA1 sync pulse */
	#if		0	
    mHWreg_Assign_Register( HW_RADIO_GIO_LOW_BPKTCTL_AND_TX_SYNC_1,
    		                HW_RADIO_LE_SET_BPKTCTL_LOW_END_TX_TIFS +
    		                HW_RADIO_LE_PRE_TX_BDATA1_PULSE_LOW_TIFS);
	#else
    mHWreg_Assign_Register( HW_RADIO_GIO_LOW_BPKTCTL_AND_TX_SYNC_1,
    		                (HW_RADIO_LE_SET_BPKTCTL_LOW_END_TX_TIFS +
    		                HW_RADIO_LE_PRE_TX_BDATA1_PULSE_LOW_TIFS)|0x8000);	
	#endif
}

#endif

/*************************************************************************************
 *
 * FUNCTION NAME: _HWradio_SetGios_FullRx
 * DESCRIPTION:   This functions sets up the high and low times for the GIOs in
 *                order to produce correct signals for the  FullRx driver mode.
 *
 * PARAMETER:      None.
 *
 *
 * RETURNS :       None.
 *
 *
 * SPECIAL CONSIDERATIONS:  None.
 *
 *
 *************************************************************************************/
static void _HWradio_SetGios_FullRx(void)
{
		/* cancell all combines */
		_HWradio_SetGios_ResetCombines();

		/* Ensure BPKTCTL is low prior to entering this mode. */
//		_HWradio_SetGios_BpktCtlLow();

} 

/*************************************************************************************
 *
 * FUNCTION NAME: _HWradio_SetGios_Standby
 * DESCRIPTION:   This functions sets up the high and low times for the GIOs in
 *                order to produce correct signals for the Standby driver mode.
 *
 *
 * PARAMETER:      None.
 *
 *
 * RETURNS :       None.
 *
 *
 * SPECIAL CONSIDERATIONS:  None.
 *
 *
 *************************************************************************************/
static void _HWradio_SetGios_Standby(void)
{
		/* cancell all combines */
//		_HWradio_SetGios_ResetCombines();

		/* Ensure BPKTCTL is low prior to entering this mode. */
//		_HWradio_SetGios_BpktCtlLow();

		HWradio_SetOverrideLow(TX_EN);
		HWradio_SetOverrideLow(RX_EN);
}

/*************************************************************************************
 *
 * FUNCTION NAME: HWradio_DisableAllSpiWrites
 *
 * Disables all SPI writes (TX/RX writes and RSSI reads).
 *
 *************************************************************************************/
void HWradio_DisableAllSpiWrites(void)
{
    mHWreg_Logical_AND_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0xffff0000);
    mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);
}



void _HWradio_ProgNow_xinc(u_int32 in_val)
{
	/* must wait if SPI bus is busy */
	HWdelay_Wait_For_Serial_Interface_Idle();
		

	/* specify data to write */
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_0, in_val);
	
	/* specify NOW mode */
	mHWreg_Assert_Cache_Bit(JAL_SER_ESER_CTRL_REG, JAL_SER_NOW);
	mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);	
	
	/* must wait until SPI transition to busy or timeout */
	HWdelay_Wait_For_Serial_Interface_Busy_us(5);
	/*while (!(mHWreg_Get_Register(JAL_SER_CFG_REG) & BIT31))
		{
		};*/
	
	
	/* must wait if SPI bus is busy */
	HWdelay_Wait_For_Serial_Interface_Idle();

	/* disable NOW mode */
	mHWreg_Clear_Cache_Bit(JAL_SER_ESER_CTRL_REG, JAL_SER_NOW);
	mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);
}


//**********************************************************************
u_int32 _HWradio_ReadNow_xinc(u_int32 Reg_To_Read)
{
    u_int32 val;
    /* must wait if SPI bus is busy */
    HWdelay_Wait_For_Serial_Interface_Idle();             		
	    	
    /* specify data to read */
	mHWreg_Assign_Register(JAL_SER_ESER_DATA_0, Reg_To_Read<<16);

	/* specify NOW mode */
    mHWreg_Assert_Cache_Bit(JAL_SER_ESER_CTRL_REG, JAL_SER_NOW);
    mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);
	
    /* wait until SPI transition to busy or timeout */
   // HWdelay_Wait_For_Serial_Interface_Busy_us(5);
	while (!(mHWreg_Get_Register(JAL_SER_CFG_REG) & BIT31))
		{
		};
	
    /* must wait if SPI bus is busy */
    HWdelay_Wait_For_Serial_Interface_Idle();

	/* read data specified */
	val = mHWreg_Get_Register(JAL_SER_READ_DATA);

	/* disable NOW mode */
    mHWreg_Clear_Cache_Bit(JAL_SER_ESER_CTRL_REG, JAL_SER_NOW);
    mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);
	return(val);
}

/*************************************************************************************
 *
 * FUNCTION NAME: _HWradio_Send_Idle_Now
 * DESCRIPTION:   This function send an Idle command to the Radio. It is sent 
 *                immediately. The function wil not return until it has been sent
 *
 *
 * PARAMETER:      None.
 *
 *
 * RETURNS :       None.
 *
 *
 * SPECIAL CONSIDERATIONS:  None.
 *
 *
 *************************************************************************************/
#define _HWradio_Send_Idle_Now() \
{\
}


/*************************************************************************************
 *
 * FUNCTION NAME: HWradio_LE_RxComplete
 *
 *************************************************************************************/
void HWradio_LE_RxComplete(void)
{
	HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);
#ifndef OUR_RADIO

#else
	/*HWradio_CancelOverride(TX_EN);*/
	HWradio_SetOverrideLow(RX_EN);
	HWradio_SetOverrideHigh(TX_EN);
	
#endif
    HWradio_SetOverrideHigh(DATA_DIR);
}

#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)
/*************************************************************************************
 *
 * FUNCTION NAME: HWradio_LE_Service
 * DESCRIPTION:   This function is the main Radio service routine. It handles the radio
 *                driver state machine and the change from one mode to another. It is also
 *                called to set the frequencies for the next frame or frame fragment
 *                depending on the current mode.
 *
 * PARAMETER:      in_RadioMode - This is the requested mode for the driver.
 *                 in_Context - the is the device context which defines the current activity
 *                 io_Channels - This is a pointer to the channel settings. When a channel setting
 *                               is used, it is reset to be invalid.
 *
 * RETURNS :       None.
 *
 *
 * SPECIAL CONSIDERATIONS:  None.
 *
 *
 *************************************************************************************/

boolean HWradio_LE_Service(t_RadioMode in_RadioMode, u_int8 io_Channel, t_radio_freq_mask freq_mask)
{
	boolean lv_result =  TRUE;
	uint32_t rfreq = 0, tfreq = 0;
	/* a double on the requested mode and the current mode */
	switch(in_RadioMode)
	{
		case RADIO_MODE_LE_FULL_RX :

			switch(CurrentRadioMode)
			{
				case RADIO_MODE_STANDBY :
				case RADIO_MODE_LE_TIFS_TX_RX :
				case RADIO_MODE_LE_FULL_RX:
#if	0
					/* Take the channel for  RX_START and start scan */
				    _HWradio_LE_DisableSpiWritesExceptTXRXandNOW();
#endif
					/* Set the Hab receive mode to full window */
					HWhab_Set_Rx_Mode(HWradio_RXm_FULL_WIN);

					/* set up the gios for full rx */
					_HWradio_SetGios_FullRx();
					
					/* set up the gios for first Tx on exit out of Full Rx
					 * This first Tx will be performed based on the TIFs count. */
					_HWradio_SetGios_LE_TIFS_TxRx();

					_HWradio_Send_Idle_Now();
					//HWdelay_Wait_For_us(12);
					//Send_LE_Receive_Command_Now(io_Channel);
					//Send_LE_Receive_Command_Now(78);
					#if		(__DEBUG_RADIO_TYPE == RADIO_ZSW)
					_HWradio_ProgNow(WRITE_REG_ZSW(bt_idx_reg, (io_Channel)));
					#elif	(__DEBUG_RADIO_TYPE == RADIO_2421)	
					
					io_Channel = io_Channel>>1;
					SER_WRITE(0x00, rf_rx_freq[io_Channel].freq_dec);
					SER_WRITE(0x01, rf_rx_freq[io_Channel].freq_int);
					SER_WRITE(0x01, (0x8000|rf_rx_freq[io_Channel].freq_int) );	
					SER_WRITE(0x04,  0x48b);
						   
					#endif
					
					HWradio_SetOverrideLow(TX_EN);
					HWradio_SetOverrideHigh(RX_EN);
					/* Setup LE Tx/Rx SPI */
					//at passive scan , if this flag not set active mode, RF frequence will be not configed		
					if (LE_Active_Mode) //temp for debug   
					{
					#if		(__DEBUG_RADIO_TYPE == RADIO_ZSW)
						Set_LE_Timed_TX_START_Transmit_Command(io_Channel);
						Set_LE_Timed_RX_START_Receive_Command(io_Channel);
					#elif	(__DEBUG_RADIO_TYPE == RADIO_2421)
					//- for after connect_req.
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x00, rf_tx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(0x01,rf_tx_freq[io_Channel].freq_int));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(0x01,(0x8000|rf_tx_freq[io_Channel].freq_int)));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(0x04,0x1c70));
						
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x00, rf_rx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(0x01,rf_rx_freq[io_Channel].freq_int));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(0x01,(0x8000|rf_rx_freq[io_Channel].freq_int)));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(0x04,0x48b));
						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0xff00);					
					#elif(__DEBUG_RADIO_TYPE ==RADIO_0851)
                    #if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)     
                    __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
                    #endif
                    
                    #if(__DEBUG_RF_MPW1_SHDN_ENABLE == 1)	
                    __write_hw_reg32(BTPHY_CTL0,  BTPHY_CTL0_NO_SHDN);
                    #endif
                    
					#if(__DEBUG_RF_LOWPOWER_ENABLE == 1)
						SER_WRITE(0x10, 0x00); //- ldo on
					#endif	
						io_Channel = io_Channel>>1;

					//	mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x00, rf_tx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x01,(TX_CONST_VALUE|rf_tx_freq[io_Channel].freq_int)));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(0x0e,0x20));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(0x0e,0x28));
						
					//	mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x00, rf_rx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x01,(RX_CONST_VALUE|rf_rx_freq[io_Channel].freq_int)));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(0x0e,0x20));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(0x0e,0x28));
						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0x1100);
						
						SER_WRITE(0x00, rf_rx_freq[io_Channel].freq_dec);
						SER_WRITE(0x01, (RX_CONST_VALUE|rf_rx_freq[io_Channel].freq_int) );	
						//SER_WRITE(0x0e, 0x20 );	
						//SER_WRITE(0x0e, 0x28 );
					#elif(__DEBUG_RADIO_TYPE == RADIO_XINC)
						io_Channel = io_Channel>>1;					
						//SER_WRITE(REG_CTL_0xbfc, REG_CTL_0xbfc_RX);			
						#if __DEBUG_XINCHIP_RF_LOWPOWER_ENABLE
                        if ( 1 == g_debug_xinchip_rf_lowpower_enable)
                        {                        
    						ldo_on_debug();
    						__write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
                        }
						#endif						
						tfreq = (rf_tx_freq[io_Channel].freq_int << 4) | (rf_tx_freq[io_Channel].freq_dec  );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(REG_CTL_0x15c,tfreq));
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v0));
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v1));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(REG_CTL_0x3c,REG_CTL_0x3c_TX));
						
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(REG_CTL_0xbfc,REG_CTL_0xbfc_TX));

						rfreq = (rf_rx_freq[io_Channel].freq_int <<4) | (rf_rx_freq[io_Channel].freq_dec  );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(REG_CTL_0x15c,rfreq));					
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v0));
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v1));						
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(REG_CTL_0x3c,REG_CTL_0x3c_RX));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(REG_CTL_0xbfc,REG_CTL_0xbfc_RX));
						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0xFF00);
						mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);

						SER_WRITE(REG_CTL_0x15c, rfreq);
//						SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v0);
//						SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v1);
						SER_WRITE(REG_CTL_0x3c, REG_CTL_0x3c_RX);
								
					#endif
					}


					CurrentRadioMode = RADIO_MODE_LE_FULL_RX;
					break;

				default :
					lv_result = FALSE; // not allowed
					break;
			}
			break;

		case RADIO_MODE_LE_INITIAL_RX :

			switch(CurrentRadioMode)
			{
				case RADIO_MODE_STANDBY :
#if	0
					_HWradio_LE_DisableSpiWritesExceptTXRXandNOW();
#endif
					if(valid_frequency(RX_START,freq_mask))
					{
						/* Set the Hab receive mode to full window */
						HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);

						/* set up the gios for full rx */
						_HWradio_SetGios_FullRx();
						_HWradio_SetGios_LE_TIFS_TxRx();

						_HWradio_Send_Idle_Now();
						
						//Send_LE_Receive_Command_Now(io_Channel);

						/*	HWdelay_Wait_For_us(12);*/
						#if		(__DEBUG_RADIO_TYPE == RADIO_ZSW)
						_HWradio_ProgNow(WRITE_REG_ZSW(bt_idx_reg, (io_Channel)));
						HWradio_SetOverrideLow(TX_EN);
						HWradio_SetOverrideHigh(RX_EN);
						// Setup LE Tx SPI
						Set_LE_Timed_TX_START_Transmit_Command(io_Channel);
						Set_LE_Timed_RX_START_Receive_Command(io_Channel);
						#elif	(__DEBUG_RADIO_TYPE == RADIO_2421)	
						//- for after connect_req.
						io_Channel = io_Channel>>1;
						
						SER_WRITE(0x00, rf_rx_freq[io_Channel].freq_dec);
						SER_WRITE(0x01, rf_rx_freq[io_Channel].freq_int);
						SER_WRITE(0x01, (0x8000|rf_rx_freq[io_Channel].freq_int) );	
						SER_WRITE(0x04,  0x48b);

						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x00, rf_tx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(0x01,rf_tx_freq[io_Channel].freq_int));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(0x01,(0x8000|rf_tx_freq[io_Channel].freq_int)));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(0x04,0x1c70));
						
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x00, rf_rx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(0x01,rf_rx_freq[io_Channel].freq_int));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(0x01,(0x8000|rf_rx_freq[io_Channel].freq_int)));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(0x04,0x48b));
						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0xff00);	
						#elif	(__DEBUG_RADIO_TYPE == RADIO_0851)
                        #if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)     
                        __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
                        #endif
                        
                        #if(__DEBUG_RF_MPW1_SHDN_ENABLE == 1)	
                        __write_hw_reg32(BTPHY_CTL0,  BTPHY_CTL0_NO_SHDN);
                        #endif                        
                        
						#if(__DEBUG_RF_LOWPOWER_ENABLE == 1)
						SER_WRITE(0x10, 0x00); //- ldo on
						#endif
						io_Channel = io_Channel>>1;


						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x00, rf_tx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x01,(TX_CONST_VALUE|rf_tx_freq[io_Channel].freq_int)));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(0x0E,0x20));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(0x0e,0x28));
						
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x00, rf_rx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x01,(RX_CONST_VALUE|rf_rx_freq[io_Channel].freq_int)));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(0x0e,0x20));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(0x0e,0x28));
						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0x1100);	
						
						SER_WRITE(0x00, rf_rx_freq[io_Channel].freq_dec);
						SER_WRITE(0x01, (RX_CONST_VALUE|rf_rx_freq[io_Channel].freq_int) );	
						//SER_WRITE(0x0e, 0x20 );	
						//SER_WRITE(0x0e, 0x28 );	
						#elif(__DEBUG_RADIO_TYPE == RADIO_XINC)
						#if __DEBUG_XINCHIP_RF_LOWPOWER_ENABLE
                        if ( 1 == g_debug_xinchip_rf_lowpower_enable)
                        {                        
    						ldo_on_debug();
    						__write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
                        }
						#endif
						io_Channel = io_Channel>>1;						
						//SER_WRITE(REG_CTL_0xbfc, REG_CTL_0xbfc_RX);							

						tfreq = (rf_tx_freq[io_Channel].freq_int << 4) | (rf_tx_freq[io_Channel].freq_dec );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(REG_CTL_0x15c,tfreq));						
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v0)); 
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v1));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(REG_CTL_0x3c,REG_CTL_0x3c_TX));
						
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(REG_CTL_0xbfc,REG_CTL_0xbfc_TX));
						rfreq = (rf_rx_freq[io_Channel].freq_int <<4) | (rf_rx_freq[io_Channel].freq_dec  );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(REG_CTL_0x15c,rfreq));						
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v0));
//						mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v1));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(REG_CTL_0x3c,REG_CTL_0x3c_RX));
						
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(REG_CTL_0xbfc,REG_CTL_0xbfc_RX));
						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0xFF00);

						SER_WRITE(REG_CTL_0x15c, rfreq);						
//                        SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v0);
//                        SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v1);
						SER_WRITE(REG_CTL_0x3c, REG_CTL_0x3c_RX);
						
						#endif
						mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);
						CurrentRadioMode = RADIO_MODE_LE_INITIAL_RX;
					}
					break;

				default:
					lv_result = FALSE; // not allowed
					break;
			}
			break;

		case RADIO_MODE_LE_INITIAL_TX:

			switch(CurrentRadioMode)
			{
				case RADIO_MODE_STANDBY :
				case RADIO_MODE_LE_TIFS_TX_RX :

					/* Setup Hab for the normal receive mode */

					HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);

				case RADIO_MODE_LE_INITIAL_TX :

					/* Set the program words for tx */
					if(valid_frequency(TX_START,freq_mask))
					{

						//_HWradio_LE_DisableSpiWritesExceptTXRXandNOW();

						HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);

						HWradio_SetOverrideHigh(DATA_DIR);
#ifndef OUR_RADIO
						_HWradio_SetGios_LE_Initial_TxRx();				
#else
					/*	_HWradio_LE_SetTxEn((HW_RADIO_SET_BPKTCTL_HIGH_PRE_TX_NORM>>16),
											(HW_RADIO_LE_SET_BPKTCTL_LOW_END_TX_TIFS>>16));*/


						HWradio_SetOverrideLow(RX_EN);
						HWradio_SetOverrideHigh(TX_EN);

						
						//RX_EN window is config in txcomplete
						//_HWradio_LE_SetRxEn(0,150);
#endif

						_HWradio_Send_Idle_Now();
//						HWdelay_Wait_For_us(12);
#ifndef OUR_RADIO			
						
#endif
#if		(__DEBUG_RADIO_TYPE == RADIO_ZSW)				
						Send_LE_Transmit_Command_Now(io_Channel);
						Set_LE_Timed_RX_START_Receive_Command(io_Channel);
						Set_LE_Timed_TX_START_Transmit_Command(io_Channel);
#elif	(__DEBUG_RADIO_TYPE == RADIO_2421)	

						io_Channel = io_Channel>>1;

					       SER_WRITE(0x00, rf_tx_freq[io_Channel].freq_dec);
					       SER_WRITE(0x01, rf_tx_freq[io_Channel].freq_int);
					       SER_WRITE(0x01, (0x8000|rf_tx_freq[io_Channel].freq_int) );	
					       SER_WRITE(0x04,  0x1c70);
						   
				  #if		1
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x00, rf_tx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(0x01,rf_tx_freq[io_Channel].freq_int));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(0x01,(0x8000|rf_tx_freq[io_Channel].freq_int)));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(0x04,0x1c70));
						
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x00, rf_rx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(0x01,rf_rx_freq[io_Channel].freq_int));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(0x01,(0x8000|rf_rx_freq[io_Channel].freq_int)));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(0x04,0x48b));

						//mHWreg_Clear_Cache_Bit(JAL_SER_ESER_CTRL_REG, JAL_SER_NOW);
						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0xff00);
				#endif		
#elif(__DEBUG_RADIO_TYPE ==RADIO_0851)
                        #if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)     
                        __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
                        #endif
                        
                        #if(__DEBUG_RF_MPW1_SHDN_ENABLE == 1)   
                        __write_hw_reg32(BTPHY_CTL0,  BTPHY_CTL0_NO_SHDN);
                        #endif

                        #if(__DEBUG_RF_LOWPOWER_ENABLE == 1)    
				        SER_WRITE(0x10, 0x00); //- ldo on
				        #endif
						io_Channel = io_Channel>>1;

					    SER_WRITE(0x00, rf_tx_freq[io_Channel].freq_dec);
					    SER_WRITE(0x01, (TX_CONST_VALUE|rf_tx_freq[io_Channel].freq_int) );	
						//SER_WRITE(0x0e, 0x20 );	
						//SER_WRITE(0x0e, 0x28 );	
						   
					//	mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x00, rf_tx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(0x01,(TX_CONST_VALUE|rf_tx_freq[io_Channel].freq_int)));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(0x0e,0x20));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(0x0e,0x28));
						
					//	mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x00, rf_rx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(0x01,(RX_CONST_VALUE|rf_rx_freq[io_Channel].freq_int)));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(0x0e,0x20));
						//mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(0x0e,0x28));						

						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0x1100);

#elif(__DEBUG_RADIO_TYPE == RADIO_XINC)
					io_Channel = io_Channel>>1;
					tfreq = (rf_tx_freq[io_Channel].freq_int << 4) | (rf_tx_freq[io_Channel].freq_dec  );
					#if __DEBUG_XINCHIP_RF_LOWPOWER_ENABLE
                    if ( 1 == g_debug_xinchip_rf_lowpower_enable)
                    {                    
    					ldo_on_debug();
    					__write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
                    }
					#endif
					SER_WRITE(REG_CTL_0x15c, tfreq);
//                    SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v0);
//                    SER_WRITE(REG_CTL_0x6c, REG_CTL_0x6c_v1);
					SER_WRITE(REG_CTL_0x3c, REG_CTL_0x3c_TX);
					
					//SER_WRITE(REG_CTL_0xbfc, REG_CTL_0xbfc_TX); 						

					mHWreg_Assign_Register(JAL_SER_ESER_DATA_12,WRITE_REG_RF(REG_CTL_0x15c,tfreq));
//					mHWreg_Assign_Register(JAL_SER_ESER_DATA_13,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v0));
//					mHWreg_Assign_Register(JAL_SER_ESER_DATA_14,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v1));
					mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(REG_CTL_0x3c,REG_CTL_0x3c_TX));

					//mHWreg_Assign_Register(JAL_SER_ESER_DATA_15,WRITE_REG_RF(REG_CTL_0xbfc,REG_CTL_0xbfc_TX));

					rfreq = (rf_rx_freq[io_Channel].freq_int << 4) | (rf_rx_freq[io_Channel].freq_dec );
					mHWreg_Assign_Register(JAL_SER_ESER_DATA_8,WRITE_REG_RF(REG_CTL_0x15c,rfreq));					
//					mHWreg_Assign_Register(JAL_SER_ESER_DATA_9,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v0));
//					mHWreg_Assign_Register(JAL_SER_ESER_DATA_10,WRITE_REG_RF(REG_CTL_0x6c,REG_CTL_0x6c_v1));
					mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(REG_CTL_0x3c,REG_CTL_0x3c_RX));
					
					//mHWreg_Assign_Register(JAL_SER_ESER_DATA_11,WRITE_REG_RF(REG_CTL_0xbfc,REG_CTL_0xbfc_RX));
					mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0xFF00);	
#endif					

						// Removed as no Override should exist on BPKTCTL for LE
#ifndef OUR_RADIO
#if	0		
						HWradio_CancelOverride(BPKTCTL_AND_TX_SYNC_1);
#endif
#else
						//HWradio_CancelOverride(TX_EN);
#endif
					}
					else if(valid_frequency(RX_START,freq_mask))
					{
						HWradio_SetOverrideLow(DATA_DIR);

					}

					mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);

					CurrentRadioMode = RADIO_MODE_LE_INITIAL_TX;

					break;

				default:
					lv_result = FALSE; // not allowed
					break;
			}
			break;

		case RADIO_MODE_LE_TESTMODE_TX:
				switch(CurrentRadioMode)
				{

				case RADIO_MODE_STANDBY :
				case RADIO_MODE_LE_TIFS_TX_RX :

	                /* Setup Hab for the normal receive mode */

				    HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);

				case RADIO_MODE_LE_TESTMODE_TX:
				case RADIO_MODE_LE_INITIAL_TX :

				{
					//HWradio_SetOverrideHigh(TX_EN);
					//mHWreg_Assign_Register(HW_RADIO_GIO_HIGH_TX_EN	, 0x4dd0000);
					//mHWreg_Assign_Register(HW_RADIO_GIO_LOW_TX_EN	, 0x1380000);
					/* Set the program words for tx */
					if(valid_frequency(TX_START,freq_mask))
					{
					//	_HWradio_LE_DisableSpiWritesExceptTXRXandNOW();

					    HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);

						HWradio_SetOverrideHigh(DATA_DIR);
						// Added GF - 5 oct 201

						_HWradio_SetGios_LE_Testmode_Tx();

						_HWradio_Send_Idle_Now();
						//HWdelay_Wait_For_us(12);
						//_HWradio_ProgNow(WRITE_REG1_BT_TX(io_Channel));
						//_HWradio_ProgNow(WRITE_REG1_LE_TX(io_Channel));
						
						//_HWradio_ProgNow(WRITE_REG_RF(0x00,gTxFreqMhzBt[2*io_Channel+1]));    
    					//	_HWradio_ProgNow(WRITE_REG_RF(0x01,gTxFreqMhzBt[2*io_Channel]));   
						//_HWradio_ProgNow(WRITE_REG_RF(0x01,gTxFreqMhzBt[2*io_Channel]|0x8000));
					//	_HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));	//1242 TX


					//- ++ SER TIME ++
	 					/*mHWreg_Assign_Register(JAL_SER_ESER_DATA_0,WRITE_REG_RF(0x00, 0xfc0) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_1,WRITE_REG_RF(0x01,0x193c));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_2,WRITE_REG_RF(0x01,0x993c));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_3,WRITE_REG_RF(0x04,0x1c70));*/
						io_Channel = io_Channel>>1;
						/*mHWreg_Assign_Register(JAL_SER_ESER_DATA_0,WRITE_REG_RF(0x00, rf_tx_freq[io_Channel].freq_dec) );
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_1,WRITE_REG_RF(0x01,rf_tx_freq[io_Channel].freq_int));
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_2,WRITE_REG_RF(0x01,(0x8000|rf_tx_freq[io_Channel].freq_int)));*/
						#if		0
						mHWreg_Assign_Register(JAL_SER_ESER_DATA_3,WRITE_REG_RF(0x04,0x1c70));

     						mHWreg_Logical_OR_With_Cache_Register(JAL_SER_ESER_CTRL_REG, 0x08);
						#endif	
     					//	_HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));	//1242 TX
					//- ++
		
						//Send_LE_Transmit_Command_Now(io_Channel);
												
	                    // Removed as no Override should exist on BPKTCTL for LE
	                    #if	0	
						HWradio_CancelOverride(BPKTCTL_AND_TX_SYNC_1);
						#endif
					}

					mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);

					CurrentRadioMode = RADIO_MODE_LE_TESTMODE_TX;
				}
				break;
				default :
					break;

				}
			break;
		case RADIO_MODE_LE_TESTMODE_RX :

			switch(CurrentRadioMode)
			{
				case RADIO_MODE_STANDBY :
				case RADIO_MODE_LE_TIFS_TX_RX :
				case RADIO_MODE_LE_TESTMODE_RX:

					/* Take the channel for  RX_START and start scan */
	#if	0					
				    _HWradio_LE_DisableSpiWritesExceptTXRXandNOW();
	#endif
					/* Set the Hab receive mode to full window */
					HWhab_Set_Rx_Mode(HWradio_RXm_FULL_WIN);

					/* set up the gios for full rx */
					_HWradio_SetGios_FullRx();
					
					/* set up the gios for first Tx on exit out of Full Rx
					 * This first Tx will be performed based on the TIFs count. */
					_HWradio_SetGios_LE_TIFS_TxRx();

					_HWradio_Send_Idle_Now();
					//HWdelay_Wait_For_us(12);
					//Send_LE_Receive_Command_Now(io_Channel);
					//Send_LE_Receive_Command_Now(78);
					#if		0
					_HWradio_ProgNow(WRITE_REG_ZSW(bt_idx_reg, (io_Channel)));
					#else

					/*_HWradio_ProgNow(WRITE_REG_RF(0x00,gTxFreqMhzBt[2*io_Channel+1]));    
    					_HWradio_ProgNow(WRITE_REG_RF(0x01,gTxFreqMhzBt[2*io_Channel]));   
					_HWradio_ProgNow(WRITE_REG_RF(0x01,gTxFreqMhzBt[2*io_Channel]|0x8000));
					*/	
			//	    SER_WRITE(0x0C,0x7879);
				 
			//	    SER_WRITE(0x0D,0x0C0F);    
			#if		1
				   /* SER_WRITE(0x00,0x0FC0);
				    SER_WRITE(0x01,0x193C);
				    SER_WRITE(0x01,0x993C);*/
				   // io_Channel = io_Channel>>1;
				  //  SER_WRITE(0x00, rf_rx_freq[io_Channel].freq_dec);
				  //  SER_WRITE(0x01, rf_rx_freq[io_Channel].freq_int);
				  // SER_WRITE(0x01, (0x8000|rf_rx_freq[io_Channel].freq_int) );	
			#else
                               SER_WRITE(0x00,0x7A17);//2439   
                               SER_WRITE(0x01,0x1961); 
                               SER_WRITE(0x01,0x9961);
			#endif		
					/*_HWradio_ProgNow_xinc(WRITE_REG_RF(0x04,0x48b));*/
					
					#endif

					
					HWradio_SetOverrideLow(TX_EN);
					HWradio_SetOverrideHigh(RX_EN);
					/* Setup LE Tx/Rx SPI
			
					if (LE_Active_Mode)
					{
						Set_LE_Timed_TX_START_Transmit_Command(io_Channel);
						Set_LE_Timed_RX_START_Receive_Command(io_Channel);
					}

					mHWreg_Store_Cache_Register(JAL_SER_ESER_CTRL_REG);
					 */
					 
					CurrentRadioMode = RADIO_MODE_LE_TESTMODE_RX;
					break;

				default :
					lv_result = FALSE; // not allowed
					break;
			}
			break;

		case RADIO_MODE_LE_TIFS_TX_RX :
			HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);

			/* Set the program words for tx */
			if(valid_frequency(TX_START,freq_mask))
			{
				HWradio_SetOverrideHigh(DATA_DIR);
				/* setup the GIOs for the BPKTCTRL Tx and BDATA1 sync before the data tx. */
				HWradio_SetOverrideLow(RX_EN);
				HWradio_SetOverrideHigh(TX_EN);

			
				//Send_LE_Transmit_Command_Now(io_Channel);
//				_HWradio_LE_SetTxEn((HW_RADIO_LE_SET_BPKTCTL_HIGH_PRE_TX_TIFS>>16),
//									(HW_RADIO_LE_SET_BPKTCTL_LOW_END_TX_TIFS>>16));	

#if	0	
				HWradio_CancelOverride(BPKTCTL_AND_TX_SYNC_1);
#endif		
				_HWradio_SetGios_LE_TIFS_TxRx();

			}
			else if(valid_frequency(RX_START,freq_mask))
			{
				HWradio_SetOverrideLow(DATA_DIR);
				/* setup the GIOs for the BPKTCTRL Tx and BDATA1 sync before the data tx. */
				_HWradio_SetGios_LE_TIFS_TxRx();

				HWradio_SetOverrideLow(TX_EN);
				HWradio_SetOverrideHigh(RX_EN);
				//Send_LE_Receive_Command_Now(io_Channel);

			}
			CurrentRadioMode = RADIO_MODE_LE_TIFS_TX_RX;
			break;
	

		/************************/
		/* STANDBY MODE REQUEST */
		/************************/

		case RADIO_MODE_STANDBY :

			switch(CurrentRadioMode)
			{

				case RADIO_MODE_STANDBY : 
					/*
					 *	Do nothing if we are already in Standby mode.
					 */
					break;

		    	case RADIO_MODE_LE_TIFS_TX_RX :
					/* Note, the following should only occur from a PKA */

		    	case RADIO_MODE_LE_INITIAL_TX :
				case RADIO_MODE_LE_FULL_RX:


					_HWradio_Send_Idle_Now();
                    ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();				    
					//HWradio_DisableAllSpiWrites();

					/* Setup the GIO lines for Standby */
					_HWradio_SetGios_Standby();
					

				    HWradio_SetOverrideLow(DATA_DIR);

					/* Set the Hab Rx mode to normal */
					HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);
				// ++ 
				// - Put radio to shutdown.
				#if	(__DEBUG_HARDWARE_PIN_ENABLE == 0)
					{
				/*	unsigned	int	val;
					SER_READ(0x1, &val);
					val &= 0xfff;
					SER_WRITE(0x01 , (0x4000 | val));
                */
					}
				#endif
				
                #if(__DEBUG_RF_MPW1_SHDN_ENABLE == 1)   
                __write_hw_reg32(BTPHY_CTL0,  BTPHY_CTL0_SHDN);
                #endif

				#if(__DEBUG_RF_LOWPOWER_ENABLE == 1)
					SER_WRITE(0x10, 0x3f);		// - ldo off
				#endif
				// ++
				#if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)		
                    __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_OFF);
				#endif
				#if __DEBUG_XINCHIP_RF_LOWPOWER_ENABLE		
                if ( 1 == g_debug_xinchip_rf_lowpower_enable)
                {                
    				ldo_off_debug();
    //				__write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_OFF);
                }
				#endif																				
					break;


				default:
					break;
			}
			CurrentRadioMode = RADIO_MODE_STANDBY;

			break;

		default:
			lv_result = FALSE; // not allowed
			break;
	}

	if (lv_result == TRUE )
		CurrentRadioMode = in_RadioMode;
#if (RECOVER_TO_STANDBY==1)
    else /* unexpected state: recover to radio mode standby */
    {
        //HWradio_DisableAllSpiWrites();
		((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
		 
		_HWradio_Send_Idle_Now();
		_HWradio_SetGios_Standby();
		HWhab_Set_Rx_Mode(HWradio_RXm_NORMAL);
        CurrentRadioMode = RADIO_MODE_STANDBY;
    }
#endif
	return(lv_result);
}


extern	void		config_radio_rx(unsigned	int	io_Channel)
{
		unsigned	int		i;

#if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)     
        __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
#endif
        
#if(__DEBUG_RF_MPW1_SHDN_ENABLE == 1)   
        __write_hw_reg32(BTPHY_CTL0,  BTPHY_CTL0_NO_SHDN);
#endif
        
#if(__DEBUG_RF_LOWPOWER_ENABLE == 1)
       SER_WRITE(0x10, 0x00); //- ldo on
#endif  

#if	0        
		/*for(i=0x500;i>0;i--);
		for(i=0x500;i>0;i--);*/
		SER_WRITE(0x0, 0x0);
		/*for(i=0x500;i>0;i--);
		for(i=0x500;i>0;i--);*/		
		SER_WRITE(0x1, (RX_CONST_VALUE | rf_rx_freq[io_Channel].freq_int));	

		/*for(i=0x500;i>0;i--);
		
		for(i=0x500;i>0;i--);
		SER_WRITE(0xe, 0x20);

		for(i=0x500;i>0;i--);
		SER_WRITE(0xe, 0x28);*/
#else
		uint32_t rfreq;

		rfreq = (rf_rx_freq[io_Channel].freq_int << 4) | (rf_rx_freq[io_Channel].freq_dec  );
		SER_WRITE(REG_CTL_0x3c, REG_CTL_0x3c_RX);
		SER_WRITE(REG_CTL_0x15c, rfreq);	
 		//SER_WRITE(REG_CTL_0xbfc, REG_CTL_0xbfc_RX); 
#endif

}

extern	void		config_radio_tx(unsigned	int	io_Channel)
{
		unsigned	int		i;

#if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)     
        __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
#endif
        
#if(__DEBUG_RF_MPW1_SHDN_ENABLE == 1)   
        __write_hw_reg32(BTPHY_CTL0,  BTPHY_CTL0_NO_SHDN);
#endif
        
#if(__DEBUG_RF_LOWPOWER_ENABLE == 1)
       SER_WRITE(0x10, 0x00); //- ldo on
#endif 
#if	0
		/*for(i=0x500;i>0;i--);
		for(i=0x500;i>0;i--);*/
		SER_WRITE(0x0, 0x0);
		/*for(i=0x500;i>0;i--);
		for(i=0x500;i>0;i--);*/	
		SER_WRITE(0x1, (TX_CONST_VALUE | rf_tx_freq[io_Channel].freq_int));	
		/*for(i=0x500;i>0;i--);
		
		for(i=0x500;i>0;i--);
		SER_WRITE(0xe, 0x20);

		for(i=0x500;i>0;i--);
		SER_WRITE(0xe, 0x28);*/
#else
		uint32_t	tfreq;

		tfreq = (rf_tx_freq[io_Channel].freq_int <<4) | (rf_tx_freq[io_Channel].freq_dec  );
		SER_WRITE(REG_CTL_0x3c, REG_CTL_0x3c_TX);
		SER_WRITE(REG_CTL_0x15c, tfreq);	
 		
#endif


}
/*************************************************************************************
 *
 * FUNCTION NAME: HWradio_LE_TxComplete
 *
 *************************************************************************************/
void HWradio_LE_TxComplete(void)
{
#ifndef OUR_RADIO
// GF 25 Nov
 	HWdelay_Wait_For_us(15+mHWradio_TX_DELAY);
	//HWdelay_Wait_For_us(5+mHWradio_TX_DELAY);

    HWradio_SetOverrideLow(BPKTCTL_AND_TX_SYNC_1);
#else
    HWradio_SetOverrideLow(TX_EN);
	HWradio_SetOverrideHigh(RX_EN);
#endif
    HWradio_SetOverrideLow(DATA_DIR);

  /* _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c77));  //turn off  TXFILTER & PA*/
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c77));  //turn off  TXFILTER & PA
  // _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c77));  //turn off  TXFILTER & PA


//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c73));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c73));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c73));  //turn off  TXFILTER & PA
//
//
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c73));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c73));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c73));  //turn off  TXFILTER & PA
//
//
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA
//   _HWradio_ProgNow(WRITE_REG_RF(0x04,0x1c70));  //turn off  TXFILTER & PA

   
}

/*************************************************************************************
 *
 * FUNCTION NAME: HWradio_LE_Setup_Radio_For_Next_TXRX
 *
 *************************************************************************************/
void HWradio_LE_Setup_Radio_For_Next_TXRX(t_frame_pos const next_slot_posn)
{
	mHWreg_Assign_Register(JAL_SER_ESER_TIME_REG, jal_le_eser_time_config_word);
}

#endif

/*************************************************************************************
 *
 * FUNCTION NAME: HWradio_Convert_Tx_Power_Level_Units_to_Tx_Power
 *
 *************************************************************************************/
s_int8 HWradio_Convert_Tx_Power_Level_Units_to_Tx_Power(u_int8 power_level)
{
	if (power_level == MAX_POWER_LEVEL_UNITS)
	{
		return 0;
	}
	else if (power_level == 1)
	{
		return -5;
	}
	else //	if (power_level == 0)
	{
		return -9;
	}
}

#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)
/*************************************************************************************
 *
 * FUNCTION NAME: HWradio_LE_Read_RSSI
 *
 *************************************************************************************/
s_int8 HWradio_LE_Read_RSSI(void)
{
//	return (s_int8)((s_int8)((_HWradio_ReadNow(READ_REG(4))>>8)&0xff));
	return 0;
}
void HWradio_LE_Set_Active_Mode(u_int8 mode)
{
	LE_Active_Mode = mode;
}
#endif

t_error HWradio_Dec_Tx_Power_Level(void)
{
    return 0;
}

t_error HWradio_Inc_Tx_Power_Level(void)
{
    return 0;
}

void HWradio_Setup_For_TIFS_Event(u_int8 io_Channel)
{
    return;
}

void _HWradioCleanUpRx(void)
{
    return;
}
